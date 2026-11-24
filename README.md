# STM32F746 bare metal bootloader
此專案基於 `STM32F746G-DISCO` 開發板，進行Bare metal的開發。目前已開發的功能包含了:

## Basic Startup
從0開始，到能夠成功進入Main function

## Peripheral Bring-up
開啟GPIO Output功能，觀察LED燈
開啟USART 6 Polling TX功能

# 環境需求

## 硬體環境
STM32F746 discovery board + USB Mini cable * 1
麵包板
UART to USB
邏輯分析儀

## 軟體環境
此專案透過WSL編譯以及Build image，再透過Windows進行燒錄與Debug

## 需要查閱的文件

### STM相關文件
rm0385-stm32f75xxx-and-stm32f74xxx-advanced-armbased-32bit-mcus-stmicroelectronics
pm0253-stm32f7-series-and-stm32h7-series-cortexm7-processor-programming-manual-stmicroelectronics
mb1191-f746ngh6-c03-schematic (*此板經過多次改版，下載線路圖前須注意硬體編號)
DS10916_stm32f745ie_CPU_data_sheet

### Arm相關文件
DDI0489F_cortex_m7_trm
DDI0403E_e_armv7m_arm

