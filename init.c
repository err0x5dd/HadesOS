#include "include/console.h"
#include "include/interrupt.h"
#include "include/system.h"
#include "include/multitasking.h"
#include "include/mm.h"
#include "include/multiboot.h"

void init(struct multiboot_info *mb_info) {
    kclean();
    
    kprintf("Starte HadesOS...\n");
    
    kprintf("Aktiviere PMM - physical memory management...\n");
    pmm_init(mb_info);
    mb_info = NULL;
    kprintf("PMM aktiviert\n");
    
    kprintf("Lade GDT...\n");
    init_gdt();
    kprintf("GDT geladen\n");

    kprintf("Lade IDT...\n");
    init_idt();
    kprintf("IDT geladen\n");
    
    kprintf("Aktiviere Tastatur...\n");
    init_keyboard();
    kprintf("Tastatur aktiviert\n");
    
    kprintf("Aktiviere Interrupts...\n");
    asm volatile("sti");
    kprintf("Interrupts aktiviert\n");
    
    kprintf("Aktiviere Multitasking...\n");
    init_multitasking();
    kprintf("Multitasking aktiviert\n");
}
