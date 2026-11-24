#ifndef _RCCf_H_
#define _RCCf_H_

#include <stdint.h>
#define RCC_BASEADDRESS 0x40023800U

typedef struct {
    volatile uint32_t CR;           // 0x00 Clock control
    volatile uint32_t PLLCFGR;      // 0x04 PLL configuration
    volatile uint32_t CFGR;         // 0x08 Clock configuration
    volatile uint32_t CIR;          // 0x0C Clock interrupt

    volatile uint32_t AHB1RSTR;     // 0x10 AHB1 reset
    volatile uint32_t AHB2RSTR;     // 0x14 AHB2 reset
    volatile uint32_t AHB3RSTR;     // 0x18 AHB3 reset
    uint32_t RESERVED0;             // 0x1C

    volatile uint32_t APB1RSTR;     // 0x20 APB1 reset
    volatile uint32_t APB2RSTR;     // 0x24 APB2 reset
    uint32_t RESERVED1[2];          // 0x28-0x2C

    volatile uint32_t AHB1ENR;      // 0x30 AHB1 clock enable
    volatile uint32_t AHB2ENR;      // 0x34 AHB2 clock enable
    volatile uint32_t AHB3ENR;      // 0x38 AHB3 clock enable
    uint32_t RESERVED2;             // 0x3C

    volatile uint32_t APB1ENR;      // 0x40 APB1 clock enable
    volatile uint32_t APB2ENR;      // 0x44 APB2 clock enable
    uint32_t RESERVED3[2];          // 0x48-0x4C

    volatile uint32_t AHB1LPENR;    // 0x50 AHB1 low-power enable
    volatile uint32_t AHB2LPENR;    // 0x54 AHB2 low-power enable
    volatile uint32_t AHB3LPENR;    // 0x58 AHB3 low-power enable
    uint32_t RESERVED4;             // 0x5C

    volatile uint32_t APB1LPENR;    // 0x60 APB1 low-power enable
    volatile uint32_t APB2LPENR;    // 0x64 APB2 low-power enable
    uint32_t RESERVED5[2];          // 0x68-0x6C

    volatile uint32_t BDCR;         // 0x70 Backup domain control
    volatile uint32_t CSR;          // 0x74 Clock control & status
    uint32_t RESERVED6[2];          // 0x78-0x7C

    volatile uint32_t SSCGR;        // 0x80 Spread spectrum clock
    volatile uint32_t PLLI2SCFGR;   // 0x84 PLLI2S configuration
    volatile uint32_t PLLSAICFGR;   // 0x88 PLLSAI configuration
    volatile uint32_t DCKCFGR1;     // 0x8C Dedicated clocks config 1
    volatile uint32_t DCKCFGR2;     // 0x90 Dedicated clocks config 2
} rcc_t;

#define RCC ((rcc_t *)(RCC_BASEADDRESS))

#endif
