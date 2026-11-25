#include <stdint.h>
#include <main.h>
#include <faults.h>

// LD symbols
// LMA address of data section
extern uint8_t _sidata;

// VMA start and end address of data section
extern uint8_t _sdata;
extern uint8_t _edata;
extern uint8_t _sbss;
extern uint8_t _ebss;

extern void EXTI15_10_IRQHandler(void);

#define DTCM_STACK_TOP 0x20010000UL
__attribute__((used, section(".isr_vector")))
const uintptr_t vector_table[] = {
    [0]  = DTCM_STACK_TOP,
    [1]  = (uintptr_t)reset_handler,
    [2]  = (uintptr_t)NMI_Handler,
    [3]  = (uintptr_t)HardFault_Handler,
    [4]  = (uintptr_t)MemManage_Handler,
    [5]  = (uintptr_t)BusFault_Handler,
    [6]  = (uintptr_t)UsageFault_Handler,
    // [7..10] Reserved
    [11] = (uintptr_t)Default_Handler, // SVCall
    [12] = (uintptr_t)Default_Handler, // Debug Monitor
    // [13] Reserved
    [14] = (uintptr_t)Default_Handler, // PendSV
    [15] = (uintptr_t)Default_Handler, // SysTick

    // External interrupts: Vector Index = IRQn + 16
    [16 + 0]  = (uintptr_t)Default_Handler, // WWDG
    [16 + 1]  = (uintptr_t)Default_Handler, // PVD
    [16 + 2]  = (uintptr_t)Default_Handler, // TAMP_STAMP
    [16 + 3]  = (uintptr_t)Default_Handler, // RTC_WKUP
    [16 + 4]  = (uintptr_t)Default_Handler, // FLASH
    [16 + 5]  = (uintptr_t)Default_Handler, // RCC
    [16 + 6]  = (uintptr_t)Default_Handler, // EXTI0
    [16 + 7]  = (uintptr_t)Default_Handler, // EXTI1
    [16 + 8]  = (uintptr_t)Default_Handler, // EXTI2
    [16 + 9]  = (uintptr_t)Default_Handler, // EXTI3
    [16 + 10] = (uintptr_t)Default_Handler, // EXTI4
    [16 + 11] = (uintptr_t)Default_Handler, // DMA1_Stream0
    [16 + 12] = (uintptr_t)Default_Handler, // DMA1_Stream1
    [16 + 13] = (uintptr_t)Default_Handler, // DMA1_Stream2
    [16 + 14] = (uintptr_t)Default_Handler, // DMA1_Stream3
    [16 + 15] = (uintptr_t)Default_Handler, // DMA1_Stream4
    [16 + 16] = (uintptr_t)Default_Handler, // DMA1_Stream5
    [16 + 17] = (uintptr_t)Default_Handler, // DMA1_Stream6
    [16 + 18] = (uintptr_t)Default_Handler, // ADC
    [16 + 19] = (uintptr_t)Default_Handler, // CAN1_TX
    [16 + 20] = (uintptr_t)Default_Handler, // CAN1_RX0
    [16 + 21] = (uintptr_t)Default_Handler, // CAN1_RX1
    [16 + 22] = (uintptr_t)Default_Handler, // CAN1_SCE
    [16 + 23] = (uintptr_t)Default_Handler, // EXTI9_5
    [16 + 24] = (uintptr_t)Default_Handler, // TIM1_BRK_TIM9
    [16 + 25] = (uintptr_t)Default_Handler, // TIM1_UP_TIM10
    [16 + 26] = (uintptr_t)Default_Handler, // TIM1_TRG_COM_TIM11
    [16 + 27] = (uintptr_t)Default_Handler, // TIM1_CC
    [16 + 28] = (uintptr_t)Default_Handler, // TIM2
    [16 + 29] = (uintptr_t)Default_Handler, // TIM3
    [16 + 30] = (uintptr_t)Default_Handler, // TIM4
    [16 + 31] = (uintptr_t)Default_Handler, // I2C1_EV
    [16 + 32] = (uintptr_t)Default_Handler, // I2C1_ER
    [16 + 33] = (uintptr_t)Default_Handler, // I2C2_EV
    [16 + 34] = (uintptr_t)Default_Handler, // I2C2_ER
    [16 + 35] = (uintptr_t)Default_Handler, // SPI1
    [16 + 36] = (uintptr_t)Default_Handler, // SPI2
    [16 + 37] = (uintptr_t)Default_Handler, // USART1
    [16 + 38] = (uintptr_t)Default_Handler, // USART2
    [16 + 39] = (uintptr_t)Default_Handler, // USART3
    [16 + 40] = (uintptr_t)EXTI15_10_IRQHandler, // EXTI15_10
    [16 + 41] = (uintptr_t)Default_Handler, // RTC_Alarm
    [16 + 42] = (uintptr_t)Default_Handler, // OTG_FS_WKUP
    [16 + 43] = (uintptr_t)Default_Handler, // TIM8_BRK_TIM12
    [16 + 44] = (uintptr_t)Default_Handler, // TIM8_UP_TIM13
    [16 + 45] = (uintptr_t)Default_Handler, // TIM8_TRG_COM_TIM14
    [16 + 46] = (uintptr_t)Default_Handler, // TIM8_CC
    [16 + 47] = (uintptr_t)Default_Handler, // DMA1_Stream7
    [16 + 48] = (uintptr_t)Default_Handler, // FMC
    [16 + 49] = (uintptr_t)Default_Handler, // SDMMC1
    [16 + 50] = (uintptr_t)Default_Handler, // TIM5
    [16 + 51] = (uintptr_t)Default_Handler, // SPI3
    [16 + 52] = (uintptr_t)Default_Handler, // UART4
    [16 + 53] = (uintptr_t)Default_Handler, // UART5
    [16 + 54] = (uintptr_t)Default_Handler, // TIM6_DAC
    [16 + 55] = (uintptr_t)Default_Handler, // TIM7
    [16 + 56] = (uintptr_t)Default_Handler, // DMA2_Stream0
    [16 + 57] = (uintptr_t)Default_Handler, // DMA2_Stream1
    [16 + 58] = (uintptr_t)Default_Handler, // DMA2_Stream2
    [16 + 59] = (uintptr_t)Default_Handler, // DMA2_Stream3
    [16 + 60] = (uintptr_t)Default_Handler, // DMA2_Stream4
    [16 + 61] = (uintptr_t)Default_Handler, // ETH
    [16 + 62] = (uintptr_t)Default_Handler, // ETH_WKUP
    [16 + 63] = (uintptr_t)Default_Handler, // CAN2_TX
    [16 + 64] = (uintptr_t)Default_Handler, // CAN2_RX0
    [16 + 65] = (uintptr_t)Default_Handler, // CAN2_RX1
    [16 + 66] = (uintptr_t)Default_Handler, // CAN2_SCE
    [16 + 67] = (uintptr_t)Default_Handler, // OTG_FS
    [16 + 68] = (uintptr_t)Default_Handler, // DMA2_Stream5
    [16 + 69] = (uintptr_t)Default_Handler, // DMA2_Stream6
    [16 + 70] = (uintptr_t)Default_Handler, // DMA2_Stream7
    [16 + 71] = (uintptr_t)Default_Handler, // USART6
    [16 + 72] = (uintptr_t)Default_Handler, // I2C3_EV
    [16 + 73] = (uintptr_t)Default_Handler, // I2C3_ER
    [16 + 74] = (uintptr_t)Default_Handler, // OTG_HS_EP1_OUT
    [16 + 75] = (uintptr_t)Default_Handler, // OTG_HS_EP1_IN
    [16 + 76] = (uintptr_t)Default_Handler, // OTG_HS_WKUP
    [16 + 77] = (uintptr_t)Default_Handler, // OTG_HS
    [16 + 78] = (uintptr_t)Default_Handler, // DCMI
    [16 + 79] = (uintptr_t)Default_Handler, // CRYP
    [16 + 80] = (uintptr_t)Default_Handler, // RNG
    [16 + 81] = (uintptr_t)Default_Handler, // FPU
    [16 + 82] = (uintptr_t)Default_Handler, // UART7
    [16 + 83] = (uintptr_t)Default_Handler, // UART8
    [16 + 84] = (uintptr_t)Default_Handler, // SPI4
    [16 + 85] = (uintptr_t)Default_Handler, // SPI5
    [16 + 86] = (uintptr_t)Default_Handler, // SPI6
    [16 + 87] = (uintptr_t)Default_Handler, // SAI1
    [16 + 88] = (uintptr_t)Default_Handler, // LTDC
    [16 + 89] = (uintptr_t)Default_Handler, // LTDC_ER
    [16 + 90] = (uintptr_t)Default_Handler, // DMA2D
    [16 + 91] = (uintptr_t)Default_Handler, // SAI2
    [16 + 92] = (uintptr_t)Default_Handler, // QUADSPI
    [16 + 93] = (uintptr_t)Default_Handler, // LPTIM1
    [16 + 94] = (uintptr_t)Default_Handler, // CEC
    [16 + 95] = (uintptr_t)Default_Handler, // I2C4_EV
    [16 + 96] = (uintptr_t)Default_Handler, // I2C4_ER
    [16 + 97] = (uintptr_t)Default_Handler, // SPDIF_RX
};

void reset_handler(void)
{
    uint8_t *src = &_sidata;
    uint8_t *dst = &_sdata;
    uint8_t *end = &_edata;

    while (dst < end) {
        *dst++ = *src++;
    }

    /* Zero .bss */
    dst = &_sbss;

    while ((uintptr_t)dst < (uintptr_t)&_ebss) {
        *dst++ = 0;
    }

    main();

    while (1) {
    }
}
