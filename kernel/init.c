#include "include/console.h"
#include "include/interrupt.h"
#include "include/system.h"
#include "include/multitasking.h"
#include "include/mm.h"
#include "include/multiboot.h"
#include "include/keyboard.h"

// Uncomment for debug output
//#define DEBUG

void init(struct multiboot_info *mb_info) {
    kclean();
    
    for(int i = 0; i < 80; i++) {
        kprintf("-");
    }
    kprintf("\n");
    
    kprintf("[init] Starting HadesOS...\n");
    
    kprintf("[init] Activating PMM...\n");
    pmm_init(mb_info);
    //mb_info = NULL;
    kprintf("[init] PMM activated\n");
    
    #ifdef DEBUG
    uintptr_t page;
    uintptr_t page_free;
    kprintf("[DEBUG] [init] Request memory page\n");
    page = pmm_alloc();
    kprintf("[DEBUG] [init] Got page  %x\n", page);
    page_free = pmm_alloc();
    kprintf("[DEBUG] [init] Got %x\n", page_free);
    page = pmm_alloc();
    kprintf("[DEBUG] [init] Got %x\n", page);
    pmm_free(page_free);
    kprintf("[DEBUG] [init] Free %x\n", page_free);
    page = pmm_alloc();
    kprintf("[DEBUG] [init] Got %x\n", page);
    page = pmm_alloc();
    kprintf("[DEBUG] [init] Got %x\n", page);
    #endif
    
//    kprintf("[init] Activate Paging...\n");
//    vmm_init(mb_info);
//    kprintf("[init] Paging activated\n");
    
    kprintf("[init] Loading GDT...\n");
    init_gdt();
    kprintf("[init] GDT loaded\n");

    kprintf("[init] Loading IDT...\n");
    init_idt();
    kprintf("[init] IDT loaded\n");

    kprintf("[init] Activating simple keyboard driver...\n");
    kbd_init();
    kprintf("[init] Keyboard driver activated\n");

    kprintf("[init] Activating multitasking...\n");
    init_multitasking(mb_info);
    kprintf("[init] Multitasking activated\n");
    
    kprintf("[init] Activate interrupts...\n");
    asm volatile("sti");
    kprintf("[init] Interrupts activated\n");

    kprintf("[init] INIT_END!\n");
    while(1);
}
