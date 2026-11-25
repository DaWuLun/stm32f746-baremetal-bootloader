# M3 — GPIO Interrupt

## 1. 目標
在前兩個 Milestone 建立的 Bare-metal Runtime 與 Peripheral Driver 基礎上，完成 GPIO Interrupt 的初始化與處理
- 設定 GPIO External Interrupt 與 NVIC
- 實作 Shared IRQ 的 Interrupt Source Identification
- 使用開發板上的 User Button 觸發 Interrupt
- 觀察並驗證 Cortex-M7 Exception Entry 時，由硬體建立的 Exception Stack Frame

## 2. 要解決的問題

### 2.1 Interrupt Routing
GPIO Pin 的訊號變化並不會直接觸發 CPU 執行 Interrupt Handler
在 STM32F746 中，GPIO External Interrupt 涉及多個硬體模組，包括 SYSCFG、EXTI 與 NVIC
因此，需要了解這些模組之間的關係，以及如何將 GPIO Pin 的訊號變化轉換成 Processor 可以處理的 Interrupt Request

### 2.2 Shared IRQ
STM32F746 的 EXTI Line 10 至 Line 15 共用同一個 IRQ Number
當 Processor 進入對應的 IRQ Handler 時，僅透過 IRQ Number 無法得知實際觸發 Interrupt 的 EXTI Line
因此，IRQ Handler 必須具備辨識 Interrupt Source 的能力

### 2.3 Processor State Preservation
Interrupt 可能發生在正常程式執行期間。為了讓 Processor 在 Interrupt Handler 執行完畢後回到原本的執行流程，Cortex-M7 會在 Exception Entry 時透過硬體保存部分 Processor Context
本階段透過實際觀察 Stack Memory，驗證 ARM Architecture Manual 所描述的 Exception Stack Frame

---

## 3. Solution & 設計思路

### 3.1 Hardware Configuration
STM32F746G-DISCO 已提供一個連接至 PI11 的 User Button，就在Reset Btn旁邊，可直接作為interrupt source

| Resource | Configuration |
|---|---|
| Interrupt Source | User Button (B1) |
| GPIO Pin | PI11 |
| EXTI Line | EXTI11 |
| IRQ | EXTI15_10 |
| IRQ Number | 40 |

### 3.2 Interrupt Routing
STM32F746 的 GPIO External Interrupt Routing 如下

```text
User Button (B1)
        |
        v
    GPIO PI11
        |
        v
      SYSCFG
  (EXTI Line Mapping)
        |
        v
      EXTI11
  (Edge Detection)
  (Pending Register)
        |
        v
  EXTI15_10 IRQ
        |
        v
       NVIC
        |
        v
   Cortex-M7
        |
        v
   IRQ Handler
```

各個硬體模組負責不同的功能：

| Module | Responsibility |
|---|---|
| GPIO | 接收外部 Pin Signal |
| SYSCFG | 選擇 EXTI Line 對應的 GPIO Port |
| EXTI | Edge Detection、Interrupt Mask 與 Pending Status |
| NVIC | Interrupt Enable、Priority 與 Exception Activation |
| Vector Table | 提供對應的 IRQ Handler Address |

### 3.3 GPIO 與 EXTI Configuration
首先透過 RCC 啟用 GPIOI 與 SYSCFG 的 Peripheral Clock，並將 PI11 設定為 Input Mode
由於不同 GPIO Port 的相同 Pin Number 可以對應至同一條 EXTI Line，因此需要透過 SYSCFG 選擇實際的 GPIO Port
本專案透過 `SYSCFG_EXTICR3`，將 EXTI11 的 Input Source 設定為 GPIOI

接著設定 EXTI：
- 透過 `EXTI_RTSR` 或 `EXTI_FTSR` 選擇所需的 Trigger Edge
- 清除既有的 Pending Status
- 透過 `EXTI_IMR` 解除 EXTI11 的 Interrupt Mask

完成上述設定後，當 PI11 發生符合設定條件的訊號變化時，EXTI Controller 便會產生對應的 Interrupt Request

### 3.4 NVIC Configuration
NVIC 負責管理 Processor 的 External Interrupts
根據 STM32F746 的 Interrupt Vector Table，EXTI Line 10 至 Line 15 對應 IRQ Number 40
NVIC 的 Interrupt Set-enable Registers (`NVIC_ISERn`) 每個 Register 管理 32 個 External Interrupts
每個Bit對應到一個Interrupt
因此，可以透過以下方式計算 IRQ Number 對應的 Register Index 與 Bit Position

```c
register_index = irq_number / 32;
bit_position   = irq_number % 32;
```

對於 EXTI15_10 IRQ：

| Parameter | Value |
|---|---|
| IRQ Number | 40 |
| ISER Index | 1 |
| Bit Position | 8 |
| Exception Number | 56 |

除了啟用 NVIC 中對應的 Interrupt，也需要在 Vector Table 中配置正確的 IRQ Handler Address
External IRQ Number 與 Vector Table Index 相差16，因為前 16 個 Vector Table Entries 保留給 Initial MSP 與 System Exceptions

### 3.5 Shared IRQ Handling
EXTI Line 10 至 Line 15 共用 EXTI15_10 IRQ
因此，當 Processor 進入 IRQ Handler 時，不能直接假設 Interrupt 一定來自 EXTI11
EXTI Controller 提供 `EXTI_PR`（Pending Register），用於記錄各個 EXTI Line 的 Pending Status
IRQ Handler 可以透過檢查對應的 Pending Bit，判斷是否存在來自 EXTI11 的 Interrupt Event
基本處理流程如下：

```text
Enter EXTI15_10 IRQ Handler
        |
        v
Read EXTI Pending Register
        |
        v
Is EXTI11 pending?
    |           |
   Yes          No
    |           |
    v           |
Handle Event    |
    |           |
Clear EXTI11    |
Pending Bit     |
    |           |
    +-----------+
        |
        v
Exception Return
```

`EXTI_PR` 採用 Write-one-to-clear (W1C) 的操作方式，因此需要對欲清除的 Pending Bit 寫入 `1`
這是一種常見的 Shared IRQ 設計 - 多個 Interrupt Sources 共用同一條 Interrupt Request，而軟體透過 Peripheral Status Register 辨識實際的事件來源
本階段只處理 EXTI11，不實作其他 EXTI Lines 的 Interrupt Handling

### 3.6 Exception Stack Frame
根據 ARMv7-M Architecture Reference Manual（DDI0403E），Cortex-M Processor 在 Exception Entry 時，會透過硬體將部分 Processor Context 保存至 Stack
Basic Exception Stack Frame 包含以下內容：

| Offset | Register |
|---|---|
| `0x00` | R0 |
| `0x04` | R1 |
| `0x08` | R2 |
| `0x0C` | R3 |
| `0x10` | R12 |
| `0x14` | LR |
| `0x18` | Return PC |
| `0x1C` | xPSR |

上述 Offset 以 Basic Exception Stack Frame 的起始位址為基準，不包含可能的 Stack Alignment；我們沒有開啟FPU，因此不用煩惱 Extended Floating-point Stack Frame
在進入 IRQ Handler 時，LR 會包含特殊的 `EXC_RETURN` Value，而不是一般 Function Call 所使用的 Return Address
透過 `EXC_RETURN`，軟體可以辨識:
- Exception Return 所使用的 Stack Pointer (MSP 或 PSP)
- Exception Return 所返回的 Processor Mode
- Stack Frame 的類型 (Floating enabled or not)

本專案在 IRQ Handler Entry 保留原始 Stack Frame 的位址，避免後續 C Function Prologue 改變 Stack Pointer 後，無法直接定位硬體建立的 Exception Stack Frame
透過此方式，可以檢查 Interrupt 發生時所保存的 Register Values，並與 ARM Architecture Manual 所描述的 Stack Frame Layout 進行比對

---

## 4. 驗證方式

### 4.1 GPIO Interrupt Verification
透過 GDB 連線至 Target，並在 EXTI15_10 IRQ Handler 設定 Breakpoint
執行 Firmware 後，按下開發板上的 User Button

預期結果：
- Processor 進入對應的 IRQ Handler
- EXTI Pending Register 顯示 EXTI11 的 Pending Status
- IRQ Handler 能夠辨識並處理 EXTI11 Event
- Pending Bit 清除後，Processor 能夠返回原本的執行流程

此測試可驗證 GPIO Configuration、SYSCFG Routing、EXTI Configuration、NVIC Enable 與 Vector Table Configuration

### 4.2 Exception Stack Frame Verification
在 IRQ Handler Entry 保存原始 Exception Stack Frame 的位址
透過 GDB 觀察該位址的 Memory Content，或透過 M2 建立的 USART Output 功能輸出保存的 Register Values
對於 Basic Exception Stack Frame，可以使用以下 GDB 指令檢查stack frame內的資料

```gdb
(gdb) break exti11_top_half
(gdb) print frame->pc
```

將讀取結果依照 Exception Stack Frame Layout 解析，並確認:
- Stacked PC 是否對應 Interrupt 發生前的程式執行位置
- Stacked xPSR 是否符合被中斷時的 Processor State
- Stack Frame Layout 是否符合 ARMv7-M Architecture Reference Manual 的定義
- EXC_RETURN 所描述的 Stack Pointer 與 Frame Type 是否符合實際執行環境
- LR是否與EXC_RETURN格式相符

此測試的目的不只是確認 Interrupt Handler 能夠執行，也在於驗證 Processor 在 Exception Entry 時的硬體 Context Preservation 行為

---

## 5. References

- ARM DDI0403E — ARMv7-M Architecture Reference Manual: Exception Model, Exception Entry and Return, NVIC
- ARM DDI0489F — Cortex-M7 Technical Reference Manual
- ST PM0253 — Cortex-M7 Programming Model and NVIC
- ST RM0385 — Reset and Clock Control (RCC)
- ST RM0385 — System Configuration Controller (SYSCFG)
- ST RM0385 — Interrupts and Events (EXTI)
- ST MB1191 — STM32F746G-DISCO Board Connectors and Electrical Schematics
