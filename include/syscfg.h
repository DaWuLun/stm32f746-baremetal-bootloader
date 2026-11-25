#ifndef _SYSCFG_H_
#define _SYSCFG_H_

#include <stdint.h>

#define SYSCFG_BASEADDRESS 0x40013800U

typedef struct {
    volatile uint32_t MEMRMP;
    volatile uint32_t PMC;
    volatile uint32_t EXTICR1;
    volatile uint32_t EXTICR2;
    volatile uint32_t EXTICR3;
    volatile uint32_t EXTICR4;
    volatile uint32_t CMPCR;
} syscfg_t;

#define SYSCFG ((syscfg_t *) SYSCFG_BASEADDRESS)

#endif
