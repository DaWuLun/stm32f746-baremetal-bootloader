#include <stdint.h>
#include <faults.h>

enum {
    FAULT_NMI = 1,
    FAULT_HARD,
    FAULT_MEM,
    FAULT_BUS,
    FAULT_USAGE
};

__attribute__((noreturn))
static void fault_stop(uint32_t type)
{
    (void) type;
    while (1) {
        __asm volatile ("nop");
    }
}

/**
 * After the fault triggered, CPU will enter handler mode. That means,
 * the stack will be changed to MSP. However, the SP could be MSP or PSP
 * before the fault triggered. As a result, we need to know where the stack frame
 * is - MSP or PSP - by checking EXC_RETURN value in LR register.
 * But, for now, a small startup, only MSP is used, hence no need to handle such trouble.
 */
void NMI_Handler(void)
{
    fault_stop(FAULT_NMI);
}

void HardFault_Handler(void)
{
    fault_stop(FAULT_HARD);
}

void MemManage_Handler(void)
{
    fault_stop(FAULT_MEM);
}

void BusFault_Handler(void)
{
    fault_stop(FAULT_BUS);
}

void UsageFault_Handler(void)
{
    fault_stop(FAULT_USAGE);
}

void Default_Handler(void)
{
    while (1) {
        __asm volatile ("nop");
    }
}

