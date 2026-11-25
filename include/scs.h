#ifndef _SCS_H_
#define _SCS_H_

#include <stdint.h>

#define SCB_BASEADDRESS  0xE000ED00UL
#define NVIC_BASEADDRESS 0xE000E100UL

typedef struct {
    volatile uint32_t CPUID;       // 0x000 CPUID Base
    volatile uint32_t ICSR;        // 0x004 Interrupt Control and State
    volatile uint32_t VTOR;        // 0x008 Vector Table Offset
    volatile uint32_t AIRCR;       // 0x00C Application Interrupt and Reset
    volatile uint32_t SCR;         // 0x010 System Control
    volatile uint32_t CCR;         // 0x014 Configuration and Control

    volatile uint32_t SHPR1;       // 0x018 System Handler Priority 1
    volatile uint32_t SHPR2;       // 0x01C System Handler Priority 2
    volatile uint32_t SHPR3;       // 0x020 System Handler Priority 3
    volatile uint32_t SHCSR;       // 0x024 System Handler Control and State

    volatile uint32_t CFSR;        // 0x028 Configurable Fault Status
    volatile uint32_t HFSR;        // 0x02C HardFault Status
    volatile uint32_t DFSR;        // 0x030 Debug Fault Status
    volatile uint32_t MMFAR;       // 0x034 MemManage Fault Address
    volatile uint32_t BFAR;        // 0x038 BusFault Address
    volatile uint32_t AFSR;        // 0x03C Auxiliary Fault Status

    uint32_t reserved[17];         // 0xD40-0xD84 Reserved for CPUID registers.

    // Coprocessor Access Control
    volatile uint32_t CPACR;       // 0x088
} scb_t;

#define SCB ((scb_t *)SCB_BASEADDRESS)

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
