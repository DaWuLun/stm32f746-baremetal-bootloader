#include <stdint.h>
#include <rcc.h>
#include <gpio.h>
#include <usart.h>

/* Observe sections */
uint32_t counter = 123;             // .data
uint32_t buffer[256];               // .bss
const uint32_t lookup_table[4] = {  // .rodata
    10, 20, 30, 40
};

static void delay(void)
{
    for (volatile uint32_t i = 0; i < 150000; i++) {
        // Busy wait
    }
}

static void enable_gpioc() {
    // Enable GPIOC clock, locates at AHB1
    RCC->AHB1ENR |= (1 << 2);
    (void) RCC->AHB1ENR;
}

static void enable_gpioi() {
    // Enable GPIOI clock, locates at AHB1
    RCC->AHB1ENR |= (1 << 8);
    (void) RCC->AHB1ENR;
}

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
    RCC->CR |= (1UL << 0);

    while ((RCC->CR & (1UL << 1)) == 0) {
        /* Wait until HSI is stable */
    }

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

int main(void) {

    enable_gpioc();
    enable_gpioi();
    enable_usart6();

    // Set GPIO I1 = output mode
    GPIOI->MODER = (GPIOI->MODER & ~(0x3 << 2)) |  (0x1 << 2);

    while (1) {
        GPIOI->ODR |= (1 << 1);
        delay();

        GPIOI->ODR &= (GPIOI->ODR & ~(0x1 << 1));
        delay();

        counter++;

        usart6_puts("Hello World\r\n\0");
    }
}
