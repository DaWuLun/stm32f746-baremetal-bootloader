#ifndef _EXTI_H_
#define _EXTI_H_

#include <stdint.h>
#define EXTI_BASEADDRESS 0x40013C00U

typedef struct {
    volatile uint32_t IMR;
    volatile uint32_t EMR;
    volatile uint32_t RTSR;
    volatile uint32_t FTSR;
    volatile uint32_t SWIER;
    volatile uint32_t PR;
} exti_t;

#define EXTI ((exti_t *)(EXTI_BASEADDRESS))

#endif
