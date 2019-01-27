// pmm = physical memory manager

#include "include/stdint.h"
#include "include/mm.h"
#include "include/multiboot.h"
#include "include/system.h"

// Auskommentieren für eine erweiterte Ausgabe
#define DEBUG

#ifdef DEBUG
#include "include/console.h"
#endif

struct memory_stack {
    void*   start;
    void*   prev_memstack;

} __attribute__((packed));

static struct memory_stack* memstack_current = (void*) 0xbadc0de;

extern const void kernel_start;
extern const void kernel_end;

void pmm_init(struct multiboot_info* mb_info) {
    struct multiboot_mmap* mmap = mb_info->mbs_mmap_addr;
    struct multiboot_mmap* mmap_end = (void*)
        ((uintptr_t) mb_info->mbs_mmap_addr + mb_info->mbs_mmap_length);
    
    while(mmap < mmap_end) {
        if(mmap->type == 1) {
            void* addr = (void*) mmap->base;
            void* end_addr = (void*) ((uint64_t) addr + mmap->length);
            

            while(addr < end_addr) {
                if(addr >= (void*) &kernel_start && addr <= (void*) &kernel_end) // Exclude Kernel
                {
                    #ifdef DEBUG
                    kprintf("%x is used by kernel -> no free memory page\n", addr);
                    #endif
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
    #endif

    if((void*) memstack_alloc->prev_memstack == (void*) memstack_alloc->start) {
        #ifdef DEBUG
        kprintf("No free memory pages!\n");
        #endif
        return NULL;
    }

    struct memory_stack* memstack = memstack_alloc->prev_memstack;
    memstack_current = memstack->start;
    
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
    memstack->prev_memstack = memstack->start;
    
    #ifdef DEBUG
    kprintf("memstack_current: %x\n", memstack_current);
    kprintf("memstack: %x\n", memstack);
    kprintf("memstack->start: %x\n", memstack->start);
    kprintf("memstack->prev_memstack: %x\n", memstack->prev_memstack);
    #endif
}
