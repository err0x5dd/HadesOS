// vmm = virtual memory manager

#include "../include/stdint.h"
#include "../include/mm.h"
#include "../include/multiboot.h"
#include "../include/system.h"
#include "../include/console.h"

// Uncomment for debug output
#define DEBUG

extern const void kernel_start;
extern const void kernel_end;

#define VMM_FLAG_ACTIVE 0x01
static uint8_t flags = 0x00;
static struct vmm_context* kernel_context;


void* vmm_alloc(struct vmm_context* context) {
    if(!(flags & VMM_FLAG_ACTIVE)) {
	#ifdef DEBUG
	kprintf("[DEBUG] [vmm] Paging not yet enabled -> use pmm\n");
	#endif
	return pmm_alloc();
    }

    kprintf("[DEBUG] [vmm] Allocation start\n");
    if(context == NULL)
        context = kernel_context;
    kprintf("[DEBUG] [vmm] Start request of physical memory\n");
    uintptr_t memreq = pmm_alloc();
    kprintf("[DEBUG] [vmm] Got physical address %x\n", memreq);
    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] vmm_alloc: %x\n", memreq);
    #endif
    
    if(context == kernel_context) {
        #ifdef DEBUG
        kprintf("[DEBUG] [vmm] Still in kernel_context -> Mapping the address %x 1:1\n", memreq);
        #endif
        uint8_t map_ret = vmm_map_page(context, memreq, memreq);
	if(map_ret != 0) {
	    uint8_t color = kgetcolor();
	    ksetcolor(COLOR_FG_RED | COLOR_BG_BLACK);
	    kprintf("[vmm] Mapping unsuccessful: %d\n", map_ret);
	    ksetcolor(color);
	}
	return memreq;
    } else {
	// Find a way to store the next user addr
	//if(map_ret != 0) {
	//    uint8_t color = kgetcolor();
	//    ksetcolor(COLOR_FG_RED | COLOR_BG_BLACK);
	//    kprintf("[vmm] Mapping unsuccessful: %d\n", map_ret);
	//    ksetcolor(color);
	//}
	return NULL;
    }
}

int vmm_map_page(struct vmm_context* context, uintptr_t virt, uintptr_t phys) {
    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] Map physical addr %x to virtual addr %x\n", phys, virt);
    #endif
    uint32_t page_index = virt / 0x1000;
    uint32_t pd_index = page_index / 1024;
    uint32_t pt_index = page_index % 1024;
    
    uint32_t* page_table;
    
    if((virt & 0xFFF) || (phys & 0xFFF)) {
        uint8_t color = kgetcolor();
        ksetcolor(COLOR_FG_RED | COLOR_BG_BLACK);
        kprintf("[ERROR] [vmm] vmm_map_page error\n");
        ksetcolor(color);
        return -1;
    }
    
    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] Search/Create page table\n");
    #endif
    // Search/Create page table
    if(context->pagedir[pd_index] & PTE_PRESENT) {
	#ifdef DEBUG
	kprintf("[DEBUG] [vmm] PTE_PRESENT flag is set\n");
	#endif
        page_table = (uint32_t*) (context->pagedir[pd_index] & ~0xFFF);
	#ifdef DEBUG
	kprintf("[DEBUG] [vmm] PTE_PRESENT flag is set - 2\n");
	#endif
    } else {
	#ifdef DEBUG
	kprintf("[DEBUG] [vmm] PTE_PRESENT flag isn't set\n");
	kprintf("[DEBUG] [vmm] Create page_table\n");
	#endif
        page_table = pmm_alloc();
	kprintf("[DEBUG] [vmm] Create page_table at %x\n", page_table);
        for(int i = 0; i < 1024; i++) {
            page_table[i] = 0;
        }
        context->pagedir[pd_index] = (uint32_t) page_table | PTE_PRESENT | PTE_WRITE | PTE_USER;
	kprintf("[DEBUG] [vmm] context->pagedir[pd_index %d]: %x\n", pd_index, context->pagedir[pd_index]);
    }
    
    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] Add new mapping in page_table[pt_index %d] %x\n", pt_index, page_table[pt_index]);
    #endif
    // Add new mapping
    page_table[pt_index] = phys | PTE_PRESENT | PTE_WRITE | PTE_USER;
    kprintf("[DEBUG] [vmm] Add new mapping in page_table[pt_index %d] %x - 2\n", pt_index, page_table[pt_index]);
    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] Inval old mapping\n");
    #endif
    asm volatile("invlpg %0" : : "m" (*(char*) virt));
    
    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] Mapping for %x to %x done\n", virt, phys);
    #endif
    
    return 0;
}

void vmm_activate_context(struct vmm_context* context) {
    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] Load %x as new page directory\n", context->pagedir);
    #endif
    
    asm volatile("mov %0, %%cr3" : : "r" (context->pagedir));
    
    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] New pagedir loaded\n");
    #endif
}

struct vmm_context* vmm_create_context(void) {
    struct vmm_context* context = pmm_alloc();
    
    kprintf("[DEBUG] [vmm] Got %x as context addr\n", context);

    context->pagedir = pmm_alloc();
    kprintf("[DEBUG] [vmm] Got %x as context->pagedir addr\n", context->pagedir);
    for(int i = 0; i < 1024; i++) {
        context->pagedir[i] = NULL;
    }
    
    return context;
}


void vmm_init(struct multiboot_info* mb_info) {
    uint32_t cr0;
    
    kernel_context = vmm_create_context();
    
    //// Map first 4 MB for kernel
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
                    kprintf("[DEBUG] [vmm] %x is used by kernel -> 1:1 Mapping\n", addr);
                    #endif
                    vmm_map_page(kernel_context, (uintptr_t) addr, (uintptr_t) addr);
                } else if(addr >= (void*) &mb_info && addr <= (void*) (&mb_info + sizeof(struct multiboot_info))) { // exclude multiboot structure
                    #ifdef DEBUG
                    kprintf("[DEBUG] [vmm] %x is used by multiboot structure -> 1:1 Mapping\n", addr);
                    #endif 
                    vmm_map_page(kernel_context, (uintptr_t) addr, (uintptr_t) addr);
                } else if(addr >= (void*) mb_info->mbs_mmap_addr && addr <= (void*) (mb_info->mbs_mmap_addr + mb_info->mbs_mmap_length)) { // exclude mmap structure
                    #ifdef DEBUG
                    kprintf("[DEBUG] [vmm] %x is used by mmap structure -> 1:1 Mapping\n", addr);
                    #endif 
                    vmm_map_page(kernel_context, (uintptr_t) addr, (uintptr_t) addr);
                } else if(addr >= (void*) mb_info->mbs_mods_addr && addr <= (void*) (mb_info->mbs_mods_addr + (sizeof(struct multiboot_mods) * mb_info->mbs_mods_count))) { // exclude mmap structure
                    #ifdef DEBUG
                    kprintf("[DEBUG] [vmm] %x is used by mods structure -> 1:1 Mapping\n", addr);
                    #endif 
                    vmm_map_page(kernel_context, (uintptr_t) addr, (uintptr_t) addr);
                } else if(addr >= (void*) mods[mods_counter].mod_start && addr <= (void*) mods[mods_counter].mod_end) { // exclude modules
                    #ifdef DEBUG
                    kprintf("[DEBUG] [vmm] %x is used by module %s\n", addr, mods[mods_counter].string);
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
    
    // Map kernel_context
    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] kernel_context mapping: %x\n", kernel_context);
    #endif
    vmm_map_page(kernel_context, kernel_context, kernel_context);
    
    // Map kernel_context->pagedir
    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] kernel_context->pagedir mapping: %x\n", kernel_context->pagedir);
    #endif
    vmm_map_page(kernel_context, kernel_context->pagedir, kernel_context->pagedir);
    
    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] kernel_context: %x\n", kernel_context);
    kprintf("[DEBUG] [vmm] &kernel_context: %x\n", &kernel_context);
    kprintf("[DEBUG] [vmm] kernel_context->pagedir: %x\n", kernel_context->pagedir);
    kprintf("[DEBUG] [vmm] &kernel_context->pagedir: %x\n", &kernel_context->pagedir);
    #endif
    for(int i = 0; i < 1024; i++) {
	#ifdef DEBUG
	kprintf("[DEBUG] [vmm] TEST kernel_context->pagedir[%d]: %x\n", i, kernel_context->pagedir[i]);
	kprintf("[DEBUG] [vmm] TEST &kernel_context->pagedir[%d]: %x\n", i, &kernel_context->pagedir[i]);
	kprintf("[DEBUG] [vmm] TEST (uint32_t*) (kernel_context->pagedir[%d] & ~0xFFF): %x\n", i, (uint32_t*) (kernel_context->pagedir[i] & ~0xFFF));
	#endif
	vmm_map_page(kernel_context, (uint32_t*) (kernel_context->pagedir[i] & ~0xFFF), (uint32_t*) (kernel_context->pagedir[i] & ~0xFFF));
    }
    
    vmm_activate_context(kernel_context);
    
    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] Activate kernel context...\n");
    #endif
    
    asm volatile("mov %%cr0, %0" : "=r" (cr0));
    cr0 |= (1 << 31);
    asm volatile("mov %0, %%cr0" : : "r" (cr0));
    
    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] Kernel context activated\n");
    #endif
    
    flags |= VMM_FLAG_ACTIVE;
}

uintptr_t vmm_get_phys_map(struct vmm_context* context, uintptr_t virt) {
    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] Get mapped physical address for virtual address %x\n", virt);
    #endif
    
    uint32_t page_index = virt / 0x1000;
    uint32_t pd_index = page_index / 1024;
    uint32_t pt_index = page_index % 1024;
    
    if(context == NULL)
        context = kernel_context;
    
    uint32_t* page_table = (uint32_t*) (context->pagedir[pd_index] & ~0xFFF);
    //uint32_t phys = page_table[pt_index] & ~0xFFF;
    uint32_t phys = page_table[pt_index];
    
    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] Physical address %x is mapped at virtual address %x\n", phys, virt);
    #endif
    
    return phys;
}
