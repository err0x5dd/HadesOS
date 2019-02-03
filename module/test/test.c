#include "../../kernel/include/stdint.h"
#include "../../kernel/include/syscall.h"

void _start(void) {
    char* str = "Hello World\n";

    asm volatile("mov %0, %%eax;"
                 "mov %1, %%ebx;"
                 "int $0x30" : : "i" (SYSCALL_PRINTS), "m" (str));
    while(1);
}
