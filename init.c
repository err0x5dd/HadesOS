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
    
    long i;
    int colorSwitch = 0;
    for(i = 0; i < 100000000; i++) {
        if(colorSwitch == 0) {
            ksetcolor(0xF0);
            colorSwitch = 1;
        } else {
            ksetcolor(0x0F);
            colorSwitch = 0;
        }
        kprintf("%x", i);
    }
    
    kprintf("Interrupts aktivieren...\n");
    asm volatile("sti");
    
    while(1) {
    }
}
