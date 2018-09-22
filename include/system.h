#include "stdint.h"

#ifndef SYSTEM_H
#define SYSTEM_H

static inline void outb(uint16_t port, uint8_t data) {
    asm volatile("outb %0, %1" : : "a" (data), "Nd" (port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t data;
    asm volatile("inb %1, %0" : "=a" (data) : "Nd" (port));
    return data;
}

// buf = Zeiger auf Speicheradresse
// c = zu setzender Wert
// n = größe des Speichers
static inline void* memset(void* buf, int c, int n) {
    unsigned char* p = buf;
    
    while (n--) {
        *p++ = c;
    }
    
    return buf;
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
