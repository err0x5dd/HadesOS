#include "include/console.h"
#include "include/gdt.h"
#include "include/idt.h"
#include "include/system.h"
#include "include/multitasking.h"

void init(void) {
    kclean();
    
    kprintf("Starte HadesOS...\n");
    
    kprintf("Lade GDT...\n");
    init_gdt();
    kprintf("GDT geladen\n");

    kprintf("Lade IDT...\n");
    init_idt();
    kprintf("IDT geladen\n");
    
    kprintf("PIT auf 1 Hz setzen...\n");
    int counter = 1193182 / 1;
    outb(0x43, 0x34);
    outb(0x40, counter & 0xFF);
    outb(0x40, counter >> 8);
    kprintf("PIT auf 1 Hz gesetzt\n");
    
    kprintf("Aktiviere Interrupts...\n");
    asm volatile("sti");
    kprintf("Interrupts aktiviert\n");
    
    kprintf("Aktiviere Multitasking...\n");
    init_multitasking();
    kprintf("Multitasking aktiviert\n");
}
