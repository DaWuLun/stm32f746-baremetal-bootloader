# M4 — Application Handoff

## 1. 目標
將原本單一的 Bare-metal Firmware 拆分為兩份獨立編譯、連結及燒錄的 Firmware Images:
- Loader Image
- Application Image

Loader 完成必要的初始化後，將執行控制權移交給 Application，使 Application 能夠建立自己的 Runtime Environment 並進入 `main()`

## 2. 要解決的問題

### 2.1 Independent Firmware Images
在前面的 Milestones 中，Reset Handler 與 `main()` 位於同一份 Firmware Image。
因此，Linker 可以在 Link-time 解析 `main()` 的 Symbol Address，使 Reset Handler 能夠直接呼叫 `main()`
然而，當 Loader 與 Application 分別編譯及連結時，Loader 的 Linker 並不具備 Application 的 Symbol Information
Loader 無法直接取得 Application 中的
- Reset Handler Address
- `main()` Address
- Linker Symbols
- 各個 Section 的 VMA 與 LMA

如果 Application 的程式碼或 Memory Layout 發生變化，這些 Address 也可能隨之改變
因此，需要建立一個不依賴 Application Symbol Table 的 Handoff Mechanism

### 2.2 Runtime Environment
Loader 在執行期間已經建立自己的 Runtime Environment，包括 Stack、Global Variables 與 Vector Table
然而，Application 是另一份獨立的 Firmware Image，具有自己的 Memory Layout 與 Runtime Initialization Requirements
因此，Application Handoff 不只是跳轉至另一個 Code Address，也需要建立適當的 Processor State，使 Application 能夠執行自己的 Startup Sequence

---

## 3. Solution & 設計思路

### 3.1 Loader–Application Interface
為了解決兩份獨立 Image 之間缺乏 Symbol Information 的問題，本專案使用 Cortex-M Vector Table Layout 作為 Loader 與 Application 之間的介面
如 M1 所述，Vector Table 的前兩個 Word 分別為：

| Offset | Content |
|---|---|
| `0x00` | Initial Main Stack Pointer (MSP) |
| `0x04` | Reset Handler Address |

只要 Application 遵循此 Layout，且 Loader 知道 Application Image 的起始位址，就可以取得 Application 啟動所需的資訊
因此，Loader 不需要知道 Application 的 `main()` Address，也不需要解析 Application 的 ELF Symbol Table
Application 內部的 `.data`、`.bss`、VMA、LMA 等資訊，則由 Application 自己的 Linker Script 與 Reset Handler 負責處理
這使得 Loader 與 Application 可以獨立編譯及修改，而不需要在每次 Application 重新編譯後更新 Loader 內部的 Symbol Addresses

### 3.2 Flash Memory Layout
本專案將 Internal Flash 劃分為 Loader 與 Application 兩個區域。

| Image | Start Address | Allocated Size |
|---|---|---|
| Loader | `0x00200000` | 32 KiB |
| Application | `0x00208000` | Remaining Flash |

Memory Layout 如下：

```text
0x00200000  +-------------------------+
            | Loader Vector Table     |
            +-------------------------+
            | Loader Code / Data      |
            |                         |
0x00208000  +-------------------------+
            | Application Vector Table|
            +-------------------------+
            | Application Code / Data |
            |                         |
0x00300000  +-------------------------+
            End of Flash (exclusive)
```

Loader 與 Application 分別透過自己的 Linker Script 配置 Memory Layout
其中，Application 的 Vector Table 必須位於 `0x00208000`，使 Loader 能夠透過固定的 Base Address 取得 Initial MSP 與 Reset Handler Address

### 3.3 Application Handoff
Loader 透過 Application Base Address 讀取 Vector Table 開頭的兩個 Word

```text
Application Base Address = 0x00208000

0x00208000  -> Application Initial MSP
0x00208004  -> Application Reset Handler Address
```

Application Handoff 的基本流程如下:

1. 取得 Application 的 Initial MSP
2. 取得 Application 的 Reset Handler Address
3. 將 VTOR 設定為 Application Vector Table Address
4. 將 MSP 設定為 Application 的 Initial MSP
5. Branch 至 Application Reset Handler

由於 Cortex-M7 使用 Thumb Instruction Set，Reset Handler Address 必須包含正確的 Thumb State Information。
本專案使用 `BX` Instruction，將執行控制權移交給 Application Reset Handler
設定 Application MSP 後，Loader 不應再執行依賴原本 Stack Frame 的操作
此 Handoff 為軟體執行的控制權移交，並非 Hardware Reset。因此，Peripheral Registers、Clock Configuration 與其他 Processor State 不會自動恢復至 Reset State

### 3.4 Application Runtime Initialization
Application 取得控制權後，會執行自己的 Reset Handler
與 M1 的 Startup Sequence 相同，Application 負責
- 將 `.data` 的初始資料從 Flash 複製至 SRAM
- 清除 `.bss` Section
- 執行 Application 所需的初始化
- 進入 Application 的 `main()`

整體執行流程如下:

```text
Hardware Reset
      |
      v
Loader Vector Table
      |
      v
Loader Reset Handler
      |
      v
Loader Runtime Initialization
      |
      v
Loader main()
      |
      v
Application Handoff
      |
      +-- Read Application Vector Table
      +-- Configure VTOR
      +-- Configure MSP
      +-- Branch to Application
      |
      v
Application Reset Handler
      |
      v
Application Runtime Initialization
      |
      v
Application main()
```

透過此設計，Loader 不需要介入 Application 內部的 Runtime Initialization

### 3.5 Firmware Programming
Loader 與 Application 分別產生獨立的 Binary Images:
- `loader.bin`
- `application.bin`

本專案使用 Internal Flash 的 ITCM Address Mapping 進行 Linker Configuration，並透過對應的 AXI Flash Address Mapping 進行 Firmware Programming

| Image | Link Address (ITCM) | Programming Address (AXI) |
|---|---|---|
| Loader | `0x00200000` | `0x08000000` |
| Application | `0x00208000` | `0x08008000` |

兩組 Address Mapping 對應至相同的 Internal Flash Storage
使用 STM32CubeProgrammer CLI 分別燒錄兩份 Binary Images:

```bash
STM32_Programmer_CLI -c port=SWD -w loader.bin 0x08000000 -v
STM32_Programmer_CLI -c port=SWD -w application.bin 0x08008000 -v
```

燒錄時需要確保 Loader 與 Application 的 Image Range 不會重疊，且 Flash Erase 操作不會破壞另一份 Image。

---

## 4. 驗證方式

### 4.1 Image Layout Verification
在燒錄之前，透過 `objdump` 或 `readelf` 分別檢查 Loader 與 Application 的 ELF Files
確認:

- Loader Vector Table 位於 `0x00200000`。
- Application Vector Table 位於 `0x00208000`。
- 兩份 Image 的 Flash Memory Range 不會重疊。
- Application Vector Table 包含有效的 Initial MSP 與 Reset Handler Address。

### 4.2 Application Handoff Verification
利用 M2 建立的 USART6 Serial Output 功能，分別在 Loader 與 Application 中輸出不同的識別字串:
(Application的字串要按按鈕才會輸出)
```text
[Loader] Setting VTOR address
[App] Hello World
```

完成兩份 Image 的燒錄後，Reset MCU，並透過 PC 端的 Serial Terminal 觀察輸出
預期結果:
1. 首先接收到 Loader Image 的識別字串
2. Loader 執行 Application Handoff
3. 按下按鈕後，接收到 Application Image 的識別字串

透過不同 Image 的 Serial Output，可以確認執行控制權已由 Loader 移交至 Application，且 Application 能夠執行自己的程式碼
若識別字串由 Application 的 `main()` 輸出，亦可確認 Application 已執行至 `main()`

---

## 5. Design Scope
本階段主要驗證兩份獨立 Firmware Images 之間的 Application Handoff Mechanism
以下功能是可以在Loader階段實現，但是目前專案沒有實現
- Firmware Update Protocol
- Image Integrity Verification
- Secure Boot

## 6. References

- ARM DDI0403E — ARMv7-M Architecture Reference Manual: Vector Table, Stack Pointer, Branch Instructions
- ARM DDI0489F — Cortex-M7 Technical Reference Manual
- ST PM0253 — Cortex-M7 Programming Model and Vector Table Offset Register
- ST RM0385 — Memory Map and Embedded Flash Memory
- GNU ld Documentation — Linker Scripts, VMA and LMA
