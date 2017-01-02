#include "stdint.h"

#ifndef SYSTEM_H
#define SYSTEM_H

static inline void outb(uint16_t port, uint8_t data) {
    asm volatile("outb %0, %1" : : "a" (data), "Nd" (port));
}

static inline void inb(uint16_t port, uint8_t data) {
    asm volatile("inb %1, %0" : "=a" (data) : "Nd" (port));
}

struct cpu_state {
    uint32_t    eax;
    uint32_t    ebx;
    uint32_t    ecx;
    uint32_t    edx;
    uint32_t    esi;
    uint32_t    edi;
    uint32_t    ebp;
    
    uint32_t    interrupt;
    uint32_t    error;
    
    uint32_t    eip;
    uint32_t    cs;
    uint32_t    eflags;
    uint32_t    esp;
    uint32_t    ss;
};

#endif
