#ifndef _USART_H_
#define _USART_H_
#include <stdint.h>

#define USART6_BASE_ADDRESS 0x40011400UL

// Refer to RM0385 Ch 31.8
typedef struct {
    volatile uint32_t CR1;
    volatile uint32_t CR2;
    volatile uint32_t CR3;
    volatile uint32_t BRR;
    volatile uint32_t GTPR;
    volatile uint32_t RTOR;
    volatile uint32_t RQR;
    volatile uint32_t ISR;
    volatile uint32_t ICR;
    volatile uint32_t RDR;
    volatile uint32_t TDR;
} usart_t;

#define USART6 ((usart_t *) USART6_BASE_ADDRESS)

#endif
