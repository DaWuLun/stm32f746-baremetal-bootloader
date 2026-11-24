# M1 — Basic Startup

## 1. 目標

建立 STM32F746 的最小 Bare-metal Runtime，使 MCU 在 Reset 後能夠：
1. 從 Vector Table 取得初始 Stack Pointer 與 Reset Handler Address
2. 執行自訂 Reset Handler
3. 完成 `.data` 與 `.bss` 的初始化
4. 進入 `main()`，開始執行 Application Code

## 2. 要解決的問題

### 2.1 Entry Point
在一般 Linux User-space Program 中，程式由作業系統載入，並透過 ELF Entry Point 進入 Runtime Startup Code，最後呼叫 `main()`。
但是 Bare-metal 環境沒有作業系統協助載入與初始化程式。
因此，第一個問題是：MCU 在 Reset 後，如何知道第一條指令應該從哪裡開始執行?

### 2.2 SRAM 初始狀態

SRAM 是揮發性記憶體。剛上電時，SRAM內的值是什麼? 完全不知道
然而 C 程式可能包含：
```c
int counter = 123;
uint8_t buffer[256];
```

根據 C 語言的初始化規則，進入 `main()` 時：
- `counter` 必須等於 `123`。
- `buffer` 的所有元素必須等於 `0`。

因此，即使沒有作業系統，Bare-metal Runtime 仍必須建立符合 C 語言要求的初始記憶體狀態。

### 2.3 Memory Layout
編譯器可以將 C Code 編譯成 Machine Code，但還有一個問題 - 如何決定 Code、Global Variables 與 Stack 應該位於哪個 Memory Address?
此外，已初始化的 Global Variables 在執行期間位於 SRAM，但其初始資料必須保存在非揮發性記憶體中。
這代表 Image 的儲存位置與 Runtime 使用的位置可能不同

---

## 3. Solution & 設計思路

### 3.1 Vector Table 與 Reset Handler
根據 ARMv7-M Architecture Reference Manual（DDI0403E）的 Exception Model，Cortex-M 的 Vector Table 包含初始 Stack Pointer Value 與各個 Exception Handler Address。
Vector Table 開頭的兩個 Word 為：
| Offset | Content |
|---|---|
| `0x00` | Initial Main Stack Pointer (MSP) |
| `0x04` | Reset Handler Address |

MCU會在 Reset 啟動流程中，使用這兩個值建立初始執行狀態。所以，專案把 Vector Table 放置於 Boot Image 的起始位置，並將第二個 Word 指向自訂的 Reset Handler
Reset Handler 負責初始化 C Runtime 所需的記憶體，最後呼叫 `main()`

### 3.2 Boot Address

下一個問題是：MCU 如何知道應該從哪個 Memory Address 取得 Vector Table?
根據 STM32F746 Reference Manual（RM0385）Chapter 2.5，STM32F7 支援透過 Boot Configuration 選擇啟動位址。
Boot Address 由 `BOOT_ADD0`、`BOOT_ADD1` 等設定，以及 BOOT Pin 的取樣結果決定
本專案使用 STM32F746G-DISCO，並採用開發板的預設 Boot Configuration：
- BOOT Pin接地，因此選擇 Boot Address 0
- 預設 Boot Address 0 對應 `0x00200000`

因此，本專案將 Boot Image 的 Vector Table 配置在 `0x00200000`

### 3.3 SRAM 初始化

為了建立正確的 C Runtime Environment，Reset Handler 需要處理兩個 Section。

### Data Section: Initialized Global / Static Variables

例如：
```c
int counter = 123;
```
`counter` 的 Runtime Address 位於 SRAM，但初始值 `123` 必須保存在 Flash Image 中。
因此，Reset Handler 必須將 Flash 中的初始資料複製到對應的 SRAM Address。

### BSS - Zero-initialized Global / Static Variables

例如：
```c
uint8_t buffer[256];
```

這類變數不需要在 Flash Image 中保存一份完整的零值資料。
Reset Handler 只需要將 `.bss` 對應的 SRAM 區域清零。
如此一來，進入 `main()` 時，程式便能觀察到符合 C 語言規則的初始值。
本階段只初始化必要的 C Runtime Sections，不會清除整個 SRAM。

### 3.4 Linker Script 與 Memory Layout

Linker Script 負責決定各個 Output Section 的配置位置，以及相關 Linker Symbols 的 Address
換句話說，他負責決定Section在Image裡面放哪裡、Runtime要放哪裡
本專案將程式與資料區域分別配置於 Internal Flash 與 SRAM

| Section | Runtime Location | Purpose |
|---|---|---|
| `.isr_vector` | Flash | Vector Table |
| `.text` | Flash | Executable Code |
| `.rodata` | Flash | Read-only Data |
| `.data` | SRAM | Initialized Global / Static Data |
| `.bss` | SRAM | Zero-initialized Global / Static Data |

其中，這裡會涉及兩個不同的位址概念:

- **LMA (Load Memory Address)**：初始資料在 Flash Image 中的儲存位址。
- **VMA (Virtual Memory Address)**：Section 在 Runtime 使用的位址。

在本專案中，`.data` 的 LMA 位於 Flash，VMA 位於 SRAM。
Reset Handler 會透過 Linker Script 提供的 Symbols，取得 `.data` 的來源與目的位址，以及 `.bss` 的範圍
因此，Linker Script 不只是決定 Section 的位置，也提供 Startup Code 初始化 Runtime 所需的 Address Information

---

## 4. 驗證方式

### 4.1 Reset Handler Verification

透過 ST-LINK GDB Server 與 GDB 連線至 Target
在 Reset Handler 設定 Breakpoint，並確認 Reset 後能進入預期的 Startup Code。

```gdb
(gdb) break reset_handler
(gdb) c
```

### 4.2 `.data` Initialization Verification

觀察具有非零初始值的 Global Variable:

```c
int counter = 123;
```

在 `.data` Copy Loop 執行前，觀察 `counter` 所在的 SRAM Address。
此時該位置的內容不能被假設為 `123`，也不能假設一定為零。
接著執行 `.data` Copy Loop，並確認：

```gdb
(gdb) break reset_handler
(gdb) print counter
(gdb) break main
(gdb) print counter
```

結果分別為 `任意值` 與 `123`。前者是因為，剛開始的時候，SRAM內部的值是未知，因此結果為任意值；後者則是因為counter已經被reset_handler給設定為初始值 `123`

### 4.3 `.bss` Initialization Verification

使用未明確初始化的 Global Variable：

```c
uint8_t buffer[256];
```

在 `.bss` Zero-initialization Loop 執行前，刻意將測試範圍填入非零資料。
執行 Zero-initialization Loop 後，確認整個 `buffer` 均為零。
這可以避免測試結果只是碰巧遇到原本已經為零的 SRAM。

---

## 5. References

- ARM DDI0403E — ARMv7-M Architecture Reference Manual：Exception Model、Vector Table、Reset Behavior
- ARM DDI0489F — Cortex-M7 Technical Reference Manual：Processor Implementation
- ST RM0385 — STM32F74xxx/75xxx Reference Manual：Chapter 2.2.2 Memory Map、Chapter 2.5 Boot Configuration
- ST PM0253 — STM32F7 Programming Manual：Cortex-M7 Programming Model

## 6. Debug

Toolchain提供了許多工具，可用於檢查Image內的編譯結果。在編譯成功後，使用 `arm-none-eabi-readelf`、`arm-none-eabi-objdump` 或 `arm-none-eabi-nm` 檢查 ELF Image:

1. Vector Table 是否位於預期的 Flash Address
2. Reset Handler Symbol 是否存在，且其 Address 正確
3. `.data` 的 LMA 與 VMA 是否符合設計
4. `.bss` 是否配置於 SRAM
