#include <stdint.h>
#include <stddef.h>

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

#define DTCM_STACK_TOP 0x20010000UL

__attribute__((section(".isr_vector"), used))
const uintptr_t vector_table[] = {
    DTCM_STACK_TOP,
    (uintptr_t) reset_handler,
    (uintptr_t) NMI_Handler,
    (uintptr_t) HardFault_Handler,
    (uintptr_t) MemManage_Handler,
    (uintptr_t) BusFault_Handler,
    (uintptr_t) UsageFault_Handler,
    (uintptr_t) NULL, // Reserved
    (uintptr_t) NULL, // Reserved
    (uintptr_t) NULL, // Reserved
    (uintptr_t) NULL, // Reserved
    (uintptr_t) Default_Handler, // SV Call
    (uintptr_t) Default_Handler, // Debug Monitor
    (uintptr_t) NULL, // Reserved
    (uintptr_t) Default_Handler, // Pend SV
    (uintptr_t) Default_Handler, // Sys Tick
    // HW IRQs. Skip for now.
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
