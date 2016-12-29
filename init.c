#include "include/console.h"
#include "include/gdt.h"
#include "include/idt.h"

void init(void) {
    kclean();
    
    kprintf("Starte HadesOS...\n");
    
    kprintf("Lade GDT...\n");
    init_gdt();
    kprintf("GDT geladen\n");

    kprintf("Lade IDT...\n");
    init_idt();
    kprintf("IDT geladen\n");
    
    
    // continue before this point with kernel initialisation
    
    kprintf("Interrupts aktivieren...\n");
    asm volatile("sti");
    
    while(1) {
    }
}
