#ifndef _SCS_H_
#define _SCS_H_

#include <stdint.h>

#define NVIC_BASEADDRESS 0xE000E100UL

// Skip SCB registers
// Skip SysTick registers

// Refer to DDI0403E Ch B3.4.3 for further infos
typedef struct {
    volatile uint32_t ISER[16];
    volatile uint32_t ICER[16];
    volatile uint32_t ISPR[16];
    volatile uint32_t ICPR[16];
    volatile uint32_t IABR[16];
    // Unused in this project
    // uint32_t padding_0[48];
    // uint32_t IPR[124];
} nvic_t;

#define NVIC ((nvic_t *) NVIC_BASEADDRESS)

#endif
