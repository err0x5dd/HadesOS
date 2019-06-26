// vmm = virtual memory manager

#include "../include/stdint.h"
#include "../include/mm.h"
#include "../include/multiboot.h"
#include "../include/system.h"

// Auskommentieren für eine erweiterte Ausgabe
#define DEBUG

#ifdef DEBUG
#include "../include/console.h"
#endif

extern struct memory_stack* memstack_current;

extern const void kernel_start;
extern const void kernel_end;

// Moved to mm.h
//struct vmm_context {
//    uint32_t* pagedir;
//    uintptr_t next_user_addr;
//};

static struct vmm_context* kernel_context;


void* vmm_alloc(struct vmm_context* context) {
    if(context == NULL)
        context = kernel_context;
    
    void* memreq = pmm_alloc();
    
    #ifdef DEBUG
    kprintf("vmm_alloc: %x\n", memreq);
    #endif
    
    vmm_map_page(context, memreq, memreq);
    
    return memreq;
}


int vmm_map_page(struct vmm_context* context, uintptr_t virt, uintptr_t phys) {
    #ifdef DEBUG
    kprintf("Map physical addr %x to virtual addr %x\n", phys, virt);
    #endif
    uint32_t page_index = virt / 0x1000;
    uint32_t pd_index = page_index / 1024;
    uint32_t pt_index = page_index % 1024;
    
    uint32_t* page_table;
    
    if((virt & 0xFFF) || (phys & 0xFFF)) {
        kprintf("vmm_map_page error\n");
        return -1;
    }
    
    // Page Table heraussuchen bzw. anlegn
    if(context->pagedir[pd_index] & PTE_PRESENT) {
        page_table = (uint32_t*) (context->pagedir[pd_index] & ~0xFFF);
    } else {
        page_table = pmm_alloc();
        for(int i = 0; i < 1024; i++) {
            page_table[i] = 0;
        }
        context->pagedir[pd_index] = (uint32_t) page_table | PTE_PRESENT | PTE_WRITE | PTE_USER;
    }
    
    // Neues Mapping eintragen
    page_table[pt_index] = phys | PTE_PRESENT | PTE_WRITE | PTE_USER;
    asm volatile("invlpg %0" : : "m" (*(char*) virt));
    
    #ifdef DEBUG
    kprintf("Mapping for %x to %x done\n", virt, phys);
    #endif
    
    return 0;
}

void vmm_activate_context(struct vmm_context* context) {
    #ifdef DEBUG
    kprintf("Load %x as new page directory\n");
    #endif
    
    asm volatile("mov %0, %%cr3" : : "r" (context->pagedir));
    
    #ifdef DEBUG
    kprintf("New pagedir loaded\n");
    #endif
}

struct vmm_context* vmm_create_context(void) {
    struct vmm_context* context = pmm_alloc();
    
    context->pagedir = pmm_alloc();
    for(int i = 0; i < 1024; i++) {
        context->pagedir[i] = 0;
    }
    
    context->next_user_addr = 0x40000000; // Set first virtual user addr to 1 GiB
    
    return context;
}


void vmm_init(struct multiboot_info* mb_info) {
    uint32_t cr0;
    
    kernel_context = vmm_create_context();
    
    // Map first 4 MB for kernel
    //for(int i = 0; i < 4096 * 1024; i += 0x1000) {
    //    vmm_map_page(kernel_context, i, i);
    //}
    
    struct multiboot_mmap* mmap = mb_info->mbs_mmap_addr;
    struct multiboot_mmap* mmap_end = (void*)
        ((uintptr_t) mb_info->mbs_mmap_addr + mb_info->mbs_mmap_length);
    
    struct multiboot_mods* mods = mb_info->mbs_mods_addr; 
    
    uint32_t mods_counter = 0;

    while(mmap < mmap_end) {
        if(mmap->type == 1) {
            void* addr = (void*) mmap->base;
            void* end_addr = (void*) ((uint64_t) addr + mmap->length);
            

            while(addr < end_addr) {
                if(addr >= (void*) &kernel_start && addr <= (void*) &kernel_end) { // exclude kernel
                    #ifdef DEBUG
                    kprintf("%x is used by kernel -> 1:1 Mapping\n", addr);
                    #endif
                    vmm_map_page(kernel_context, (uintptr_t) addr, (uintptr_t) addr);
                } else if(addr >= (void*) &mb_info && addr <= (void*) (&mb_info + sizeof(struct multiboot_info))) { // exclude multiboot structure
                    #ifdef DEBUG
                    kprintf("%x is used by multiboot structure -> 1:1 Mapping\n", addr);
                    #endif 
                    vmm_map_page(kernel_context, (uintptr_t) addr, (uintptr_t) addr);
                } else if(addr >= (void*) mb_info->mbs_mmap_addr && addr <= (void*) (mb_info->mbs_mmap_addr + mb_info->mbs_mmap_length)) { // exclude mmap structure
                    #ifdef DEBUG
                    kprintf("%x is used by mmap structure -> 1:1 Mapping\n", addr);
                    #endif 
                    vmm_map_page(kernel_context, (uintptr_t) addr, (uintptr_t) addr);
                } else if(addr >= (void*) mb_info->mbs_mods_addr && addr <= (void*) (mb_info->mbs_mods_addr + (sizeof(struct multiboot_mods) * mb_info->mbs_mods_count))) { // exclude mmap structure
                    #ifdef DEBUG
                    kprintf("%x is used by mods structure -> 1:1 Mapping\n", addr);
                    #endif 
                    vmm_map_page(kernel_context, (uintptr_t) addr, (uintptr_t) addr);
                } else if(addr >= (void*) mods[mods_counter].mod_start && addr <= (void*) mods[mods_counter].mod_end) { // exclude modules
                    #ifdef DEBUG
                    kprintf("%x is used by module %s\n", addr, mods[mods_counter].string);
                    #endif
                    
                    // Increment module counter when this page is the module end
                    if((addr + PAGE_SIZE) > (void*) mods[mods_counter].mod_end && (mods_counter + 1) < mb_info->mbs_mods_count) {
                        mods_counter++;
                    }
                    vmm_map_page(kernel_context, (uintptr_t) addr, (uintptr_t) addr);
                }
                addr += PAGE_SIZE;
            }
        }
        mmap++;
    }
    
    // Map video buffer
    for(uint32_t addr = 0xb8000; addr <= 0xbffff; addr += 0x1000) {
        vmm_map_page(kernel_context, (uintptr_t) addr, (uintptr_t) addr);
    }
    
    // Map current pmm stack to 0x40000000 - PAGE_SIZE;
    #ifdef DEBUG
    kprintf("Memstack address in virtual memory: %x\n", MEMSTACK_ADDR);
    kprintf("memstack_current: %x\n", memstack_current->start);
    #endif
    vmm_map_page(kernel_context, (uintptr_t) MEMSTACK_ADDR, (uintptr_t) memstack_current->start);
    
    // Map kernel_context->pagedir
    kprintf("pagedir mapping: %x\n", kernel_context->pagedir);
    vmm_map_page(kernel_context, kernel_context->pagedir, kernel_context->pagedir);
    
    vmm_activate_context(kernel_context);
    
    #ifdef DEBUG
    kprintf("Activate kernel context...\n");
    #endif
    
    asm volatile("mov %%cr0, %0" : "=r" (cr0));
    cr0 |= (1 << 31);
    asm volatile("mov %0, %%cr0" : : "r" (cr0));
    
    #ifdef DEBUG
    kprintf("Kernel context activated\n");
    #endif
    
    // Set memstack_current to virtual address
    vmm_map_page(kernel_context, memstack_current, memstack_current);
    kprintf("vmm_init: memstack_current: %x\n", memstack_current);
    
    for(int i = 0; i < sizeof(struct memory_stack); i++) {
        kprintf("%0x", memstack_current[i]);
        if(i % 2 == 1)
            kprintf(" ");
    }
    kprintf("\n");
    
    memstack_current = MEMSTACK_ADDR;
    kprintf("vmm_init: memstack_current: %x\n", memstack_current);
    
    for(int i = 0; i < sizeof(struct memory_stack); i++) {
        kprintf("%0x", memstack_current[i]);
        if(i % 2 == 1)
            kprintf(" ");
    }
    kprintf("\n");
    
}
