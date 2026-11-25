MEMORY
{
    /* Bootloader uses first 32 KB sector */
    ITCM_FLASH (rx) : ORIGIN = 0x00200000, LENGTH = 32K

    /* Data Tightly Coupled Memory */
    DTCM (rw) : ORIGIN = 0x20000000, LENGTH = 64K

    /* Instruction Tightly Coupled Memory */
    ITCM (rwx) : ORIGIN = 0x00000000, LENGTH = 16K

    /* SRAM1 */
    SRAM1 (rwx) : ORIGIN = 0x20010000, LENGTH = 240K
}

SECTIONS
{
    /* Collect input .isr_vector sections */
    .isr_vector_ld :
    {
        KEEP(*(.isr_vector))
    } > ITCM_FLASH

    /* Collect executable code and read-only data */
    .text :
    {
        *(.text*)
        *(.rodata*)
    } > ITCM_FLASH

    /* Initialized global/static variables */
    .data :
    {
        _sdata = .;
        *(.data*)
        _edata = .;
    } > DTCM AT > ITCM_FLASH
    _sidata = LOADADDR(.data);

    /* Zero-initialized global/static variables */
    .bss (NOLOAD) :
    {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > DTCM
}
