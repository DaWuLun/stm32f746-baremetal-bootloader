# M2 — Peripheral Bring-up

## 1. 目標

在 M1 建立的 Bare-metal Runtime 基礎上，完成基本 Peripheral Bring-up
- 設定 GPIO Output，控制開發板上的 User LED
- 初始化 USART6，透過 USB-to-UART Adapter 與 PC 進行 Serial Communication
- 建立基本的 Serial Log Output，供後續 Milestone 除錯使用

## 2. 要解決的問題

### 2.1 Peripheral Clock
即使 CPU 已經能夠正常執行程式，也不代表所有 Peripheral 都已經準備好
為了降低功耗，STM32F746 的多數 Peripheral Clock 在 Reset 後預設為關閉狀態
因此，在操作 GPIO 或 USART Registers 之前，需要先透過 RCC（Reset and Clock Control）啟用對應的 Peripheral Clock

### 2.2 Pin Multiplexing
MCU 的實體 Pin 數量有限，而內部 Peripheral 可能需要大量的 Input/Output Signals
為了提高 Pin 的使用彈性，同一個 Pin 可以對應多種功能，例如：

- General-purpose GPIO
- USART TX/RX
- SPI
- I2C
- Timer Output

因此，除了初始化 Peripheral 本身之外，還需要設定 GPIO Controller，選擇該 Pin 的 Alternate Function

### 2.3 Board-level Hardware Connection
即使完成了 MCU 內部的 Peripheral Configuration，仍需要確認訊號在 PCB 上實際連接到哪個元件或 Connector
因此，Peripheral Bring-up 除了參考 MCU Reference Manual 與 Datasheet，也需要參考開發板的 User Manual 及 Schematic

---

## 3. Solution & 設計思路

### 3.1 Hardware Configuration
根據 STM32F746G-DISCO 的 schematic (mb1191) ，本階段使用以下 Hardware Resources：

| Function | MCU Pin | Board Connection |
|---|---|---|
| User LED (LD1) | PI1 | On-board LED |
| USART6 TX | PC6 | CN4 — D1 |
| USART6 RX | PC7 | CN4 — D0 |

### 3.2 RCC — Peripheral Clock Enable
STM32F746 的 Peripheral 分別連接至不同的 Bus。
本階段使用的 Peripheral 與對應的 Clock Enable Registers 如下：

| Peripheral | Bus | RCC Register | Enable Field |
|---|---|---|---|
| GPIOC | AHB1 | RCC_AHB1ENR | GPIOCEN |
| GPIOI | AHB1 | RCC_AHB1ENR | GPIOIEN |
| USART6 | APB2 | RCC_APB2ENR | USART6EN |

在初始化 Peripheral 之前，需要先啟用對應的 Clock，並確保滿足 Clock Enable 到 Peripheral Register Access 之間的硬體時序要求 - Enable後需要兩個Cycle才會奏效

相關設定請參考 RM0385 的 Reset and Clock Control (RCC) 章節

### 3.3 GPIO Output
開發板上的 User LED（LD1）連接至 GPIO PI1。
為了透過軟體控制 LED，需要完成以下設定：

1. 啟用 GPIOI Clock
2. 將 PI1 設定為 General-purpose Output Mode
3. 設定適當的 Output Type、Output Speed 與 Pull-up/Pull-down Configuration
4. 控制 GPIO Output Level，使 LED 點亮或熄滅

GPIO Controller 提供 `GPIOx_MODER` 設定 Pin Mode，並提供 `GPIOx_ODR` 與 `GPIOx_BSRR` 控制 Output Level
透過週期性改變 PI1 的 Output Level，即可使 LED 閃爍，作為基本的 Firmware Execution Indicator。不過目前沒有timer，因此實現上是透過busy wait的方式去delay

### 3.4 USART6 — Pin Configuration
USART6 的 TX 與 RX Signals 分別透過 PC6 與 PC7 對外連接
與一般 GPIO Output 不同，這兩個 Pin 必須交由 USART Peripheral 控制
因此，需要

1. 啟用 GPIOC Clock。
2. 將 PC6、PC7 設定為 Alternate Function Mode
3. 透過 `GPIOx_AFRL` 選擇 AF8，將對應的 Pin 連接至 USART6。

Alternate Function Mapping 需參考 STM32F745xx/F746xx Datasheet，而 GPIO Registers 的操作方式則參考 RM0385

### 3.5 USART6 — Peripheral Configuration
完成 Pin Configuration 後，接著初始化 USART6 Controller
基本初始化流程如下:

1. 啟用 USART6 Peripheral Clock
2. 根據 USART Kernel Clock Frequency 設定 Baud Rate Register (`USART_BRR`)
3. 設定 Serial Frame Format 與所需的 Transmission/Reception 功能
4. 啟用 USART Controller
5. 透過 Polling 方式進行資料傳輸

傳送資料時，軟體透過檢查 USART Status Register (`USART_ISR`) 中的 TXE Flag，確認 Transmit Data Register (`USART_TDR`) 可以接收下一筆資料，再寫入欲傳送的內容。

此設計提供基本的 Serial Output 功能。有Log，很多事情都簡單很多

---

## 4. 驗證方式

### 4.1 GPIO Output Verification
燒錄 Firmware 後，觀察開發板上的 User LED (LD1)，他在Reset Btn旁邊
預期結果

- LED 能夠由 Firmware 控制
- LED 持續交替點亮與熄滅

此測試可驗證 GPIO Clock Enable、Pin Mode Configuration 與 Output Control

### 4.2 USART6 Verification
使用支援 3.3 V Logic Level 的 USB-to-UART Adapter，將 USART6 連接至 PC。麵包板、LA也可以一併連接，用於除錯用

| STM32F746G-DISCO | USB-to-UART Adapter |
|---|---|
| PC6 / USART6_TX | RX |
| PC7 / USART6_RX | TX |
| GND | GND |

在 PC 端使用 PuTTY 開啟對應的 COM Port。
Serial Port Configuration：

| Parameter | Value |
|---|---|
| Baud Rate | 115200 |
| Data Bits | 8 |
| Stop Bits | 1 |
| Parity | None |
| Flow Control | None |

執行 Firmware 後，預期可在 PuTTY Terminal 中持續接收到：

```text
Hello World
```

此測試可驗證 USART Clock Configuration、Alternate Function Mapping、Baud Rate Configuration 與 Polling-based Transmission

---

## 5. References

- ST RM0385 — Reset and Clock Control (RCC)
- ST RM0385 — General-purpose I/Os (GPIO)
- ST RM0385 — Universal Synchronous Asynchronous Receiver Transmitter (USART)
- STM32F745xx/F746xx Datasheet — Alternate Function Mapping
- ST MB1191 — STM32F746G-DISCO Board Connectors and Electrical Schematics
