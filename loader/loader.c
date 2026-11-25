#include <stdint.h>
#include <gpio.h>
#include <rcc.h>
#include <usart.h>
#include <faults.h>
#include <scs.h>

#define APP_ADDRESS_ENTRY 0x00208000UL
typedef void (*entry_func)(void);

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
};

static void enable_gpioc() {
    // Enable GPIOC clock, locates at AHB1
    RCC->AHB1ENR |= (1 << 2);
    (void) RCC->AHB1ENR;
}

static void enable_syscfg() {

    // Enable SYSCFG clock, locates at APB2
    RCC->APB2ENR |= (1 << 14);
    (void) RCC->APB2ENR;
}

// TODO: usart6 in STM contains many features.
// We just enable the most basic one.
static void enable_usart6() {
    // Enable USART6 Clock
    RCC->APB2ENR |= 1 << 5;

    // Refer to RM0385 & DS10916 for alternate mode setting and af code table
    // Set GPIOC 6 & 7 to Alternate Mode 8 = USART6
    // Assume GPIOC clock are enabled
    GPIOC->MODER = (GPIOC->MODER & ~(0xF << 12)) | (0xA << 12);
    GPIOC->AFRL = (GPIOC->AFRL & ~(0xFF << 24)) | (0x88 << 24);

    // UART6 Clock source select HSI = 0x2;
    RCC->DCKCFGR2 = (RCC->DCKCFGR2 & ~(0x3 << 10)) | (0x2 << 10);

    // Make sure the HSI is enabled, though they are enabled by reset.
    RCC->CR |= 0x3;

    // M = 0x00
    USART6->CR1 = USART6->CR1 & ~(0x3 << 28);

    // Stop Bit = 1 by set STOP_BIT register = 0;
    USART6->CR2 = USART6->CR2 & ~(0x3 << 11);

    // Hardcode Baudrate = 115200
    // Baudrate = input clock * 2 (if OVER8=1) / BRR
    // input clock = HSI = 16M
    if (USART6->CR1 & (0x1 << 15)) {
        // OVER8 = 1 --> oversampling by 8 --> baudrate = 2 * 16M / BRR
        // 32M / 115200 ~= 0x116 (0d278);
        USART6->BRR = 0x116;
    } else {
        // 16M / 115200 ~= 0x8B (0d139);
        USART6->BRR = 0x008B;
    }

    // Uart Enable bit
    USART6->CR1 |= 1;
}

static void usart6_puts(char *string_to_be_send) {
    int i = 0;
    char c = '\0';

    // Enable flow can be found at RM0385 Ch 31.5.2
    // Enable TE
    USART6->CR1 |= 1 << 3;

    while ( i < 32) {
        c = string_to_be_send[i];
        if (c != '\0') {
            while((USART6->ISR & (1 << 7)) == 0);
            USART6->TDR = c;
        } else {
            break;
        }
        i++;
    }
    // polling wait until TC = 1. That's end of transmit.
    while ((USART6->ISR & (1 << 6)) == 0);

}

// ARM: first input arg = r0
// second input arg = r1
__attribute__((naked, noreturn))
static void jump_to_application(uint32_t app_sp,
                                uint32_t app_entry)
{
    // MSR = move special register, which include MSP register.
    // BX = branch to target address
    __asm volatile(
        "msr msp, r0 \n"
        "bx r1      \n"
    );
}

// Bootloader entry point.
void reset_handler(void) {
    uint32_t *new_vector_table = (uint32_t *) APP_ADDRESS_ENTRY;

    // Skip zero bss & move data section
    // Not used in here.
    // Codes are XIP, hence no need to move them into ITCM.
    enable_gpioc();
    enable_syscfg();
    enable_usart6();

    // Handing off...
    usart6_puts("[Loader] Setting VTOR address\n\r\0");
    SCB->VTOR = APP_ADDRESS_ENTRY;

    // TODO: Deinit HW...
    jump_to_application(*new_vector_table, *(new_vector_table + 1));
}
