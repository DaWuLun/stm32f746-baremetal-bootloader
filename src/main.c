#include <stdint.h>

/* Observe sections */
uint32_t counter = 123;             // .data
uint32_t buffer[256];               // .bss
const uint32_t lookup_table[4] = {  // .rodata
    10, 20, 30, 40
};

void main() {
    while(1) {
        counter ++;
        if (counter >= 0xFFFFFF00) counter = 0;
    }
}
