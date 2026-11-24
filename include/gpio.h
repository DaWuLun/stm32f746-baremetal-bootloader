#ifndef _GPIO_H_
#define _GPIO_H_
#include <stdint.h>

#define GPIOC_BASE_ADDRESS 0x40020800UL
#define GPIOI_BASE_ADDRESS 0x40022000UL

typedef struct {
    volatile uint32_t MODER;
    volatile uint32_t OTYPER;
    volatile uint32_t OSPEEDR;
    volatile uint32_t PUPDR;
    volatile uint32_t IDR;
    volatile uint32_t ODR;
    volatile uint32_t BSRR;
    volatile uint32_t LCKR;
    volatile uint32_t AFRL;
    volatile uint32_t AFRH;
} gpio_t;

#define GPIOC ((gpio_t *)GPIOC_BASE_ADDRESS)
#define GPIOI ((gpio_t *)GPIOI_BASE_ADDRESS)

#endif
