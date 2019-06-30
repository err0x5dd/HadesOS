#include "include/console.h"
#include "include/interrupt.h"
#include "include/system.h"
#include "include/multitasking.h"
#include "include/mm.h"
#include "include/multiboot.h"
#include "include/keyboard.h"

void init(struct multiboot_info *mb_info) {
    kclean();
    
    for(int i = 0; i < 80; i++) {
        kprintf("-");
    }
    kprintf("\n");
    
    kprintf("Starte HadesOS...\n");
    
    // Test
//    kprintf("Test 0x: %0x !\n", 0xbadc0de);
//    kprintf("Test  x: %x !\n", 0xbadc0de);
//    kprintf("Test 0s: %0s !\n", "Teststring");
//    kprintf("Test  s: %s !\n", "Teststring");
//    kprintf("Test 0b: %0b !\n", 0xbadc0de);
//    kprintf("Test  b: %b !\n", 0xbadc0de);
    
    kprintf("Aktiviere PMM - physical memory management...\n");
    pmm_init(mb_info);
    //mb_info = NULL;
    kprintf("PMM aktiviert\n");
    
    kprintf("Aktiviere Paging...\n");
    vmm_init(mb_info);
    kprintf("Paging aktiviert\n");
    
    kprintf("Lade GDT...\n");
    init_gdt();
    kprintf("GDT geladen\n");

    kprintf("Lade IDT...\n");
    init_idt();
    kprintf("IDT geladen\n");

    kprintf("Aktiviere Tastatur...\n");
    kbd_init();
    kprintf("Tastatur aktiviert\n");

    kprintf("Aktiviere Multitasking...\n");
    init_multitasking(mb_info);
    kprintf("Multitasking aktiviert\n");
    
    kprintf("Aktiviere Interrupts...\n");
    asm volatile("sti");
    kprintf("Interrupts aktiviert\n");

    kprintf("INIT_END!\n");
    while(1);
}
