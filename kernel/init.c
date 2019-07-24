#include <console.h>
#include <interrupt.h>
#include <system.h>
#include <multitasking.h>
#include <mm.h>
#include <multiboot.h>
#include <keyboard.h>

// Uncomment for debug output
#define DEBUG
//#define DEBUG_LVL2

extern void wait(void);
extern void vmm_break(struct vmm_context* context);
extern struct vmm_context* active_context;

void init(struct multiboot_info *mb_info) {
    kclean();

    #ifdef DEBUG
    kprintf("[DEBUG] [init] mb_info:  %x\n", mb_info);
    #endif

    for(int i = 0; i < 80; i++) {
        kprintf("-");
    }
    kprintf("\n");
    
    kprintf("[init] Starting HadesOS...\n");
    
    kprintf("[init] Activating PMM...\n");
    pmm_init(mb_info);
    //mb_info = NULL;
    kprintf("[init] PMM activated\n");
    
    kprintf("[init] Activate Paging...\n");
    vmm_init(mb_info);
    kprintf("[init] Paging activated\n");
    
    #ifdef DEBUG_LVL2
    uintptr_t page;
    uintptr_t page_free;
    kprintf("[DEBUG] [init] Request memory page\n");
    wait();
    page = vmm_alloc(NULL);
    kprintf("[DEBUG] [init] Got page  %x\n", page);
    wait();
    page_free = vmm_alloc(NULL);
    kprintf("[DEBUG] [init] Got %x\n", page_free);
    wait();
    page = vmm_alloc(NULL);
    kprintf("[DEBUG] [init] Got %x\n", page);
    wait();
    page = vmm_alloc(NULL);
    kprintf("[DEBUG] [init] Got %x\n", page);
    wait();
    page = vmm_alloc(NULL);
    kprintf("[DEBUG] [init] Got %x\n", page);
    wait();
    #endif
    
    kprintf("[init] Loading GDT...\n");
    init_gdt();
    kprintf("[init] GDT loaded\n");
    //wait();

    kprintf("[init] Loading IDT...\n");
    init_idt();
    kprintf("[init] IDT loaded\n");
    //wait();

    //kprintf("[init] Endless Loop\n");
    //while(1);

    //kprintf("[init] Activating simple keyboard driver...\n");
    //kbd_init();
    //kprintf("[init] Keyboard driver activated\n");
    //wait();

    kprintf("[init] Activating multitasking...\n");
    //wait();
    init_multitasking(mb_info);
    kprintf("[init] Multitasking activated\n");
    //wait();
    
    kprintf("[init] Activate interrupts...\n");
    //wait();
    asm volatile("sti");
    kprintf("[init] Interrupts activated\n");
    //wait();

    kprintf("[init] INIT_END!\n");
    while(1);
}
