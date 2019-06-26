// pmm = physical memory manager

#include "../include/stdint.h"
#include "../include/mm.h"
#include "../include/multiboot.h"
#include "../include/system.h"

// Auskommentieren für eine erweiterte Ausgabe
#define DEBUG

#ifdef DEBUG
#include "../include/console.h"
#endif

// Moved to mm.h
//struct memory_stack {
//    void*   start;
//    void*   prev_memstack;
//} __attribute__((packed));

struct memory_stack* memstack_current = (void*) 0xbadc0de;

extern const void kernel_start;
extern const void kernel_end;

void pmm_init(struct multiboot_info* mb_info) {
    struct multiboot_mmap* mmap = mb_info->mbs_mmap_addr;
    struct multiboot_mmap* mmap_end = (void*)
        ((uintptr_t) mb_info->mbs_mmap_addr + mb_info->mbs_mmap_length);
    
    struct multiboot_mods* mods = mb_info->mbs_mods_addr; 
    
    kprintf("Modules loaded: %d\n", mb_info->mbs_mods_count);
    
    for(uint32_t i = 0; i < mb_info->mbs_mods_count; i++) {
        kprintf("Module %d '%s':\n", i, mods[i].string);
        kprintf("  Start: %x\n", mods[i].mod_start);
        kprintf("  End:   %x\n", mods[i].mod_end);
    }
    
    uint32_t mods_counter = 0;

    while(mmap < mmap_end) {
        if(mmap->type == 1) {
            void* addr = (void*) mmap->base;
            void* end_addr = (void*) ((uint64_t) addr + mmap->length);
            

            while(addr < end_addr) {
                if(addr >= (void*) &kernel_start && addr <= (void*) &kernel_end) { // exclude kernel
                    #ifdef DEBUG
                    kprintf("%x is used by kernel -> no free memory page\n", addr);
                    #endif
                } else if(addr >= (void*) &mb_info && addr <= (void*) (&mb_info + sizeof(struct multiboot_info))) { // exclude multiboot structure
                    #ifdef DEBUG
                    kprintf("%x is used by multiboot structure -> no free memory page\n", addr);
                    #endif 
                } else if(addr >= (void*) mb_info->mbs_mmap_addr && addr <= (void*) (mb_info->mbs_mmap_addr + mb_info->mbs_mmap_length)) { // exclude mmap structure
                    #ifdef DEBUG
                    kprintf("%x is used by mmap structure -> no free memory page\n", addr);
                    #endif 
                } else if(addr >= (void*) mb_info->mbs_mods_addr && addr <= (void*) (mb_info->mbs_mods_addr + (sizeof(struct multiboot_mods) * mb_info->mbs_mods_count))) { // exclude mmap structure
                    #ifdef DEBUG
                    kprintf("%x is used by mods structure -> no free memory page\n", addr);
                    #endif 
                } else if(addr >= (void*) mods[mods_counter].mod_start && addr <= (void*) mods[mods_counter].mod_end) { // exclude modules
                    #ifdef DEBUG
                    kprintf("%x is used by module %s\n", addr, mods[mods_counter].string);
                    #endif
                    
                    // Increment module counter when this page is the module end
                    if((addr + PAGE_SIZE) > (void*) mods[mods_counter].mod_end && (mods_counter + 1) < mb_info->mbs_mods_count) {
                        mods_counter++;
                    }
                } else {
                    if(memstack_current == 0xbadc0de) {
                        #ifdef DEBUG
                        kprintf("%x will be used for starting point of memory stack\n", addr);
                        #endif
                        stack_init(addr);
                    } else {
                        pmm_free(addr);
                    }
                }
                addr += PAGE_SIZE;
            }
        }
        mmap++;
    }

    #ifdef DEBUG
    kprintf("memstack_current: %x\n", memstack_current);
    kprintf("kernel_start: %x\n", &kernel_start);
    kprintf("kernel_end: %x\n", &kernel_end);
    #endif

}


void* pmm_alloc(void) {
    struct memory_stack* memstack_alloc = memstack_current;
    
    #ifdef DEBUG
    kprintf("memstack_current: %x\n", memstack_current);
    kprintf("memstack_alloc: %x\n", memstack_alloc);
    kprintf("memstack_alloc->start: %x\n", memstack_alloc->start);
    kprintf("memstack_alloc->prev_memstack: %x\n", memstack_alloc->prev_memstack);
    
    for(int i = 0; i < sizeof(struct memory_stack); i++) {
        kprintf("%0x", memstack_current[i]);
        if(i % 2 == 1)
            kprintf(" ");
    }
    kprintf("\n");
    
    #endif

    if((uintptr_t) memstack_alloc->prev_memstack == (uintptr_t) 0xbadc0de) {
        #ifdef DEBUG
        kprintf("No free memory pages!\n");
        #endif
        return NULL;
    }

    //struct memory_stack* memstack = memstack_alloc->prev_memstack;
    
    if(memstack_current == MEMSTACK_ADDR) {
        kprintf("memstack_current: %x\n", memstack_current);
        kprintf("memstack_current: %0x\n", memstack_current);
        vmm_map_page(NULL, MEMSTACK_ADDR, memstack_alloc->prev_memstack);
    } else {
        memstack_current = memstack_alloc->prev_memstack;
    }
    
    #ifdef DEBUG
    kprintf("alloc: return address: %x\n", memstack_alloc->start);
    kprintf("alloc: new memstack_current: %x\n", memstack_current);
    #endif
    return (void*) memstack_alloc->start;
}

void pmm_free(uintptr_t page) {
    struct memory_stack* memstack = (void*) page;
/*
    kprintf("memstack: %x\n", memstack);
    kprintf("memstack2: %x\n", &memstack);
    kprintf("memstack3: %x\n", *memstack);
    kprintf("memstack_current: %x\n", memstack_current);
*/
    memstack->start = memstack;
    memstack->prev_memstack = memstack_current;

    memstack_current = memstack->start;
}

void stack_init(void* page) {
    memstack_current = (void*) page;
    struct memory_stack* memstack = memstack_current;

    memstack->start = memstack;
    memstack->prev_memstack = 0xbadc0de;
    
    #ifdef DEBUG
    kprintf("memstack_current: %x\n", memstack_current);
    kprintf("memstack: %x\n", memstack);
    kprintf("memstack->start: %x\n", memstack->start);
    kprintf("memstack->prev_memstack: %x\n", memstack->prev_memstack);
    #endif
}
