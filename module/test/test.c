#include "../../kernel/include/stdint.h"
#include "../../kernel/include/syscall.h"

#define PAGE_SIZE 4096

void print(char* str) {
    // request memory to store the string
    char* mem;
    asm volatile("mov %1, %%eax;"
                 "int $0x30;"
                 "mov %%eax, %0" : "=m" (mem) : "i" (SYSCALL_PMM_ALLOC));
    
    char c = str[0];
    for(int i = 0; c != '\0' && i < PAGE_SIZE; i++) {
        mem[i] = str[i];
        c = str[i];
    }
    
    asm volatile("mov %0, %%eax;"
                 "mov %1, %%ebx;"
                 "int $0x30" : : "i" (SYSCALL_PRINTS), "m" (mem));
}

void _start(void) {
    print("Hello World\n");
    
    while(1);
}
