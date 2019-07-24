// vmm = virtual memory manager

#include <stdint.h>
#include <mm.h>
#include <multiboot.h>
#include <system.h>
#include <console.h>
#include <mmap.h>

// Uncomment for debug output
#define DEBUG
//#define DEBUG_LVL2

extern const void kernel_start;
extern const void kernel_end;

#define VMM_FLAG_ACTIVE 0x01
static uint8_t flags = 0x00;
static struct vmm_context* kernel_context;
struct vmm_context* active_context;

void wait(void) {
    kprintf("--[WAIT]--\n");
    for(uint32_t i = 0; i < 0x07FFFFFF; i++);
}
void waitfor(uint32_t max) {
    for(uint32_t i = 0; i < max; i++);
}

uintptr_t vmm_get_mapping(struct vmm_context* context, uintptr_t virt);

void* vmm_alloc_kernel(struct vmm_context* context) {
    if(!(flags & VMM_FLAG_ACTIVE)) {
	#ifdef DEBUG
	kprintf("[DEBUG] [vmm] Paging not yet enabled -> use pmm\n");
	#endif
	return pmm_alloc();
    }

    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] Kernelspace allocation start\n");
    #endif

    if(context == NULL || context == active_context) {
        context = (struct vmm_context*) MAP_VMM_CONTEXT;
    } else {
        vmm_map_context(active_context, MAP_VMM_TEMP_CONTEXT, context);
        context = (struct vmm_context*) MAP_VMM_TEMP_CONTEXT;
    }

    uintptr_t memreq = pmm_alloc();

    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] vmm_alloc: %x\n", memreq);
    #endif
    
    kprintf("[TODO] Check for max kernelspace range\n");
    vmm_map_page(active_context, context->next_kernelspace, memreq);
    context->next_kernelspace += PAGE_SIZE;
    
    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] Will return %x\n", (context->next_kernelspace - PAGE_SIZE));
    #endif

    return (uintptr_t) (context->next_kernelspace - PAGE_SIZE);
}

void* vmm_alloc_at(struct vmm_context* context, uintptr_t vaddr) {
    if(context == NULL) {
        context = active_context;
    }
    
    vaddr &= ~0xFFF;
    uintptr_t pmem = pmm_alloc();

    uintptr_t phys = vmm_get_mapping(context, vaddr);
    if(phys != 0 && phys != 0xffffffff) {
        return NULL;
    }

    vmm_map_page(context, vaddr, pmem);

    return vaddr;
}

void* vmm_alloc(struct vmm_context* context) {
    if(!(flags & VMM_FLAG_ACTIVE)) {
	#ifdef DEBUG
	kprintf("[DEBUG] [vmm] Paging not yet enabled -> use pmm\n");
	#endif
	return pmm_alloc();
    }

    if(context == NULL)
        context = kernel_context;
    
    struct vmm_context* temp_context;

    if(context == active_context) {
	temp_context = (struct vmm_context*) MAP_VMM_CONTEXT;
    } else {
	temp_context = (struct vmm_context*) MAP_VMM_TEMP_CONTEXT;
	vmm_map_page(active_context, temp_context, context);
    }

    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] Start request of physical memory\n");
    #endif

    uintptr_t memreq = pmm_alloc();

    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] Got physical address %x\n", memreq);
    #endif
    
    vmm_map_page(context, context->next_userspace, memreq);
    context->next_userspace += PAGE_SIZE;

    return (context->next_userspace - PAGE_SIZE);
}

int vmm_map_page(struct vmm_context* context, uintptr_t virt, uintptr_t phys) {
    // Make sure we use the 4k boundary
    virt &= ~(0xFFF);
    phys &= ~(0xFFF);

    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] Map physical addr %x to virtual addr %x\n", phys, virt);
    #endif

    uint32_t page_index = virt / 0x1000;
    uint32_t pd_index = page_index / 1024;
    uint32_t pt_index = page_index % 1024;
    
    uint32_t* page_table;
    
    if(context == NULL)
        context = active_context;

    struct vmm_context* temp_context;
    uint32_t* temp_pagedir;

    if(context == active_context) {
	temp_context = (struct vmm_context*) MAP_VMM_CONTEXT;
    } else {
	temp_context = (struct vmm_context*) MAP_VMM_TEMP_CONTEXT;
	vmm_map_page(active_context, temp_context, context);
    }

    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] Search/Create page table\n");
    #endif

    uint32_t* pagedir;
    if(flags & VMM_FLAG_ACTIVE) {
    kprintf("[TEMP] paging active\n");
	if(context == active_context) {
	kprintf("[TEMP] active context\n");
	    pagedir = (uint32_t*) MAP_VMM_PAGEDIR;
	} else {
	kprintf("[TEMP] other context\n");
	    pagedir = (uint32_t*) MAP_VMM_TEMP_PAGEDIR;
	    vmm_map_page(active_context, pagedir, temp_context->pagedir);
	}
    } else {
    kprintf("[TEMP] pageing not active\n");
	pagedir = context->pagedir;
    }

    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] *pagedir set to %x\n", pagedir);
    #endif

    // Search/Create page table
    if(pagedir[pd_index] & PTE_PRESENT) {
	#ifdef DEBUG
	kprintf("[DEBUG] [vmm] PTE_PRESENT flag is set\n");
	#endif

	if(flags & VMM_FLAG_ACTIVE) {
	    if(context == active_context) {
		page_table = (uint32_t*) (MAP_VMM_PAGETABLE + (PAGE_SIZE * pd_index));
	    } else {
		page_table = (uint32_t*) MAP_VMM_TEMP_PAGETABLE;
		vmm_map_page(active_context, page_table, (pagedir[pd_index] & ~0xFFF));
	    }
	} else {
	    page_table = (uint32_t*) (pagedir[pd_index] & ~0xFFF);
	}

	#ifdef DEBUG
	kprintf("[DEBUG] [vmm] *page_table set to %x\n", page_table);
	#endif
    } else {
	#ifdef DEBUG
	kprintf("[DEBUG] [vmm] PTE_PRESENT flag isn't set\n");
	kprintf("[DEBUG] [vmm] Create page_table\n");
	#endif

        page_table = pmm_alloc();

	#ifdef DEBUG
	kprintf("[DEBUG] [vmm] Create page_table at %x\n", page_table);
	#endif

	kprintf("[TODO] [vmm] [mapping] Make page_table flags (PTE_USER/PTE_WRITE) editable\n");

        pagedir[pd_index] = (uint32_t) page_table | PTE_PRESENT | PTE_WRITE | PTE_USER;
	
	if(context == active_context) {
	    vmm_map_page(context, (uintptr_t) (MAP_VMM_PAGETABLE + (PAGE_SIZE * pd_index)), page_table);
	    page_table = (uintptr_t) (MAP_VMM_PAGETABLE + (PAGE_SIZE * pd_index));
	} else {
	    vmm_map_page(active_context, (uintptr_t) MAP_VMM_TEMP_PAGETABLE, page_table);
	    page_table = (uint32_t*) MAP_VMM_TEMP_PAGETABLE;
	}

	#ifdef DEBUG
	kprintf("[DEBUG] [vmm] pagedir[pd_index %d]: %x\n", pd_index, pagedir[pd_index]);
	#endif
    }
    
    // Add new mapping
    page_table[pt_index] = phys | PTE_PRESENT | PTE_WRITE | PTE_USER;
    //page_table[pt_index] = phys | PTE_PRESENT | PTE_WRITE;

    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] Add new mapping in page_table[pt_index %d] %x\n", pt_index, page_table[pt_index]);
    kprintf("[DEBUG] [vmm] Inval old mapping\n");
    #endif

    asm volatile("invlpg %0" : : "m" (*(char*) virt));
    
    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] Mapping for virt %x to phys %x done\n", virt, phys);
    #endif
    
    return 0;
}

void vmm_activate_context(struct vmm_context* context) {
    struct vmm_context* temp_context;
    uint32_t* temp_pagedir;

    if(flags & VMM_FLAG_ACTIVE) {
        if(context == active_context) {
            temp_context = (struct vmm_context*) MAP_VMM_CONTEXT;
            temp_pagedir = temp_context->pagedir;
        } else {
            temp_context = (struct vmm_context*) MAP_VMM_TEMP_CONTEXT;
            vmm_map_page(active_context, temp_context, context);
            temp_pagedir = temp_context->pagedir;
            vmm_map_page(active_context, temp_pagedir, temp_context->pagedir);
        }
    } else {
        temp_context = context;
        temp_pagedir = context->pagedir;
    }

    vmm_map_context(context);

    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] New context mapped\n");
    #endif

    active_context = context;

    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] Load %x as new page directory\n", temp_context->pagedir);
    #endif

    asm volatile("mov %0, %%cr3" : : "r" (temp_context->pagedir));

    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] New pagedir loaded\n");
    #endif

}

struct vmm_context* vmm_create_context(void) {
    struct vmm_context* context = pmm_alloc();
    struct vmm_context* temp_context;
    struct vmm_context* temp_active = (struct vmm_context*) MAP_VMM_CONTEXT;
    uint32_t* temp_pagedir;
    uint32_t* temp_active_pagedir = (uint32_t*) MAP_VMM_PAGEDIR;

    if(flags & VMM_FLAG_ACTIVE) {
	temp_context = (struct vmm_context*) MAP_VMM_TEMP_CONTEXT;
	//kprintf("[TEMP] active_context: %x\n", active_context);
	vmm_map_page(active_context, MAP_VMM_TEMP_CONTEXT, context);
	//vmm_map_context(active_context, temp_context, context);
    } else {
	temp_context = context;
    }

    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] Got %x as context addr\n", context);
    #endif
    
    temp_pagedir = pmm_alloc();

    if(flags & VMM_FLAG_ACTIVE) {
	//kprintf("[TEMP] temp_contex:  %x\n", temp_context);
	//kprintf("[TEMP] temp_pagedir: %x\n", temp_pagedir);
	temp_context->pagedir = temp_pagedir;
	//kprintf("[TEMP] temp_context->pagedir: %x\n", temp_context->pagedir);
	temp_pagedir = (uint32_t*) MAP_VMM_TEMP_PAGEDIR;
	vmm_map_page(active_context, MAP_VMM_TEMP_PAGEDIR, temp_context->pagedir);
    } else {
	context->pagedir = temp_pagedir;
    }

    #ifdef DEBUG
    if(flags & VMM_FLAG_ACTIVE) {
	kprintf("[DEBUG] [vmm] Got %x as context->pagedir addr\n", temp_context->pagedir);
    } else {
	kprintf("[DEBUG] [vmm] Got %x as context->pagedir addr\n", context->pagedir);
    }
    #endif

    //kprintf("\n"); //TEMP
    for(int i = 0; i < 1024; i++) {
	//kprintf("[TEMP] temp_pagedir:    %x\n", temp_pagedir);
	//kprintf("[TEMP] temp_pagedir[%d]: %x\n", i, temp_pagedir[i]);
	// When pagetable is in kernel range (<1GB), copy entry from active context
	if(i < 256) {
	    //kprintf("[TEMP] context->pagedir[%d]: %x\n", i, temp_active_pagedir[i]);
	    temp_pagedir[i] = temp_active_pagedir[i];
	} else {
	    temp_pagedir[i] = NULL;
	}
	//kprintf("[TEMP] temp_pagedir[i]: %d\n", temp_pagedir[i]);
    }
    temp_context->next_userspace = 0x40000000;
    temp_context->next_kernelspace = MAP_VMM_KSPACE_START;

    return context;
}

uintptr_t vmm_get_mapping(struct vmm_context* context, uintptr_t virt) {
    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] Get mapping for virt %x\n", virt);
    #endif

    uint32_t page_index = virt / 0x1000;
    uint32_t pd_index = page_index / 1024;
    uint32_t pt_index = page_index % 1024;

    if(!(flags & VMM_FLAG_ACTIVE)) {
	#ifdef DEBUG
	kprintf("[DEBUG] [vmm] Paging not enabled\n");
	#endif
	return NULL;
    }

    uintptr_t* pagedir;
    if(context = active_context) {
	#ifdef DEBUG
	kprintf("[DEBUG] [vmm] context is active_context\n");
	#endif
	pagedir = MAP_VMM_PAGEDIR;
    } else {
	#ifdef DEBUG
	kprintf("[DEBUG] [vmm] Map temporary context %x\n", context);
	#endif

	vmm_map_page(active_context, MAP_VMM_TEMP_CONTEXT, context);

	#ifdef DEBUG
	kprintf("[DEBUG] [vmm] Map temporary pagedir %x\n", context->pagedir);
	#endif
	
	vmm_map_page(active_context, MAP_VMM_TEMP_PAGEDIR, context->pagedir);
	pagedir = MAP_VMM_TEMP_PAGEDIR;
    }
    uintptr_t* page_table;
    
    if(pagedir[pd_index] & PTE_PRESENT) {
	if(context == active_context) {
	    #ifdef DEBUG
	    kprintf("[DEBUG] [vmm] active_context, find page_table in pagedir[pd_index %d]\n", pd_index);
	    #endif
	    
	    page_table = (uintptr_t) (MAP_VMM_PAGETABLE + (pd_index * 0x1000));

	    #ifdef DEBUG
	    kprintf("[DEBUG] [vmm] Got page_table: %x\n", page_table);
	    #endif
	    
	} else {
	    #ifdef DEBUG
	    kprintf("[DEBUG] [vmm] Map temporary pagetable\n");
	    #endif
	    
	    vmm_map_page(active_context, MAP_VMM_TEMP_PAGETABLE, (uintptr_t) (pagedir[pd_index] & ~0xFFF));
	    page_table = MAP_VMM_TEMP_PAGETABLE;
	}
    } else {
	return 0xFFFFFFFF;
    }

    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] Try accessing page_table[pt_index %d] on %x\n", pt_index, page_table);
    kprintf("[DEBUG] [vmm] Physical addr is %x\n", page_table[pt_index] & ~0xFFF);
    #endif

    return (page_table[pt_index] & ~0xFFF);
}

void vmm_map_context(struct vmm_context* context) {
    if(context == NULL) {
	uint8_t color = kgetcolor();
	ksetcolor(COLOR_FG_RED | COLOR_BG_BLACK);
	kprintf("[vmm] No valid context!\n");
	ksetcolor(color);
	return;
    }
    
    if(flags & VMM_FLAG_ACTIVE) {
	    #ifdef DEBUG
	    kprintf("[DEBUG] [vmm] Mapping of context %x -> %x\n", context, MAP_VMM_CONTEXT);
	    #endif
        
	    vmm_map_page(active_context, MAP_VMM_CONTEXT, context);
	
        #ifdef DEBUG
        kprintf("[DEBUG] [vmm] Mapping of context->pagedir %x -> %x\n", ((struct vmm_context*) MAP_VMM_CONTEXT)->pagedir, MAP_VMM_PAGEDIR);
        #endif

	    vmm_map_page(active_context, MAP_VMM_PAGEDIR, ((struct vmm_context*) MAP_VMM_CONTEXT)->pagedir);
    
	    for(uint16_t i = 0; i < 1024; i++) {
	        if(((uintptr_t*) MAP_VMM_PAGEDIR)[i] & PTE_PRESENT) {
		        #ifdef DEBUG
		        kprintf("[DEBUG] [vmm] Mapping of page_table from context->pagedir[%d] %x -> %x\n", i, ((uintptr_t*) MAP_VMM_PAGEDIR)[i], MAP_VMM_PAGETABLE + (i * 0x1000));
		        #endif
		        vmm_map_page(active_context, MAP_VMM_PAGETABLE + (i * 0x1000), ((uintptr_t*) MAP_VMM_PAGEDIR)[i]);
	        } else {
		        #ifdef DEBUG_LVL2
		        kprintf("[DEBUG_LVL2] [vmm] page_table at pagedir[%d] not yet set\n", i);
		        #endif
	        }
	    }
    } else {
	    #ifdef DEBUG
	    kprintf("[DEBUG] [vmm] Mapping of context %x -> %x\n", context, MAP_VMM_CONTEXT);
	    #endif

	    vmm_map_page(active_context, MAP_VMM_CONTEXT, context);
	
	    #ifdef DEBUG
	    kprintf("[DEBUG] [vmm] Mapping of context->pagedir %x -> %x\n", context->pagedir, MAP_VMM_PAGEDIR);
	    #endif

	    vmm_map_page(active_context, MAP_VMM_PAGEDIR, context->pagedir);
	
	    for(uint16_t i = 0; i < 1024; i++) {
	        if(((uintptr_t*) MAP_VMM_PAGEDIR)[i] & PTE_PRESENT) {
		        #ifdef DEBUG
		        kprintf("[DEBUG] [vmm] Mapping of page_table from context->pagedir[%d] %x -> %x\n", i, context->pagedir[i], MAP_VMM_PAGETABLE + (i * 0x1000));
		        #endif
		        vmm_map_page(active_context, MAP_VMM_PAGETABLE + (i * 0x1000), context->pagedir[i]);
	        } else {
		        #ifdef DEBUG_LVL2
		        kprintf("[DEBUG_LVL2] [vmm] page_table at pagedir[%d] not yet set\n", i);
		        #endif
	        }
	    }
    }
}

void vmm_init(struct multiboot_info* mb_info) {
    uint32_t cr0;
    
    kernel_context = vmm_create_context();
    active_context = kernel_context;

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
                } else if(addr >= (void*) mb_info && addr <= (void*) (mb_info + sizeof(struct multiboot_info))) { // exclude multiboot structure
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
    
    // Map multiboot structure
    for(uint32_t addr = mb_info; addr <= (mb_info + sizeof(struct multiboot_info)); addr += 0x1000) {
        vmm_map_page(kernel_context, (uintptr_t) addr, (uintptr_t) addr);
    }
    for(uint32_t addr = mmap; addr <= mmap_end; addr += 0x1000) {
        vmm_map_page(kernel_context, (uintptr_t) addr, (uintptr_t) addr);
    }
    for(uint32_t addr = mb_info->mbs_mods_addr; addr <= (mb_info->mbs_mods_addr + (mb_info->mbs_mods_count * sizeof(struct multiboot_mods))); addr += 0x1000) {
        vmm_map_page(kernel_context, (uintptr_t) addr, (uintptr_t) addr);
    }
    for(uint32_t i = 0; i < mb_info->mbs_mods_count; i++) {
        struct multiboot_mods* mb_mods = (mb_info->mbs_mods_addr + ((i - 1) * sizeof(struct multiboot_mods)));
	for(uint32_t addr = mb_mods->mod_start; addr <= mb_mods->mod_end; addr += 0x1000) {
	    #ifdef DEBUG
	    kprintf("[DEBUG] [vmm] Map mod structure at %x\n", addr);
	    #endif
            vmm_map_page(kernel_context, (uintptr_t) addr, (uintptr_t) addr);
	}
    }

    // Map kernel_context
    vmm_map_context(kernel_context);

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
    active_context = kernel_context;
    kernel_context = MAP_VMM_CONTEXT;
}

