// vmm = virtual memory manager

#include "../include/stdint.h"
#include "../include/mm.h"
#include "../include/multiboot.h"
#include "../include/system.h"
#include "../include/console.h"
#include "../include/mmap.h"

// Uncomment for debug output
#define DEBUG

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
void vmm_break(struct vmm_context* context) {
    kprintf("\n---BREAKPOINT---\n");
    kprintf("MAP_VMM_CONTEXT:  %x\n", MAP_VMM_CONTEXT);
    kprintf("MAP_VMM_PAGEDIR:  %x\n", MAP_VMM_PAGEDIR);
    kprintf("context:          %x\n", context);
    
    kprintf("context phys:     %x\n", vmm_get_mapping(context, MAP_VMM_CONTEXT));
    
    kprintf("context->pagedir: %x\n", context->pagedir);
    while(1);
}

void* vmm_alloc_kernel(struct vmm_context* context) {
    if(!(flags & VMM_FLAG_ACTIVE)) {
	#ifdef DEBUG
	kprintf("[DEBUG] [vmm] Paging not yet enabled -> use pmm\n");
	#endif
	return pmm_alloc();
    }

    kprintf("[DEBUG] [vmm] Kernelspace allocation start\n");
    if(context == NULL)
        context = (struct vmm_context*) MAP_VMM_CONTEXT;

    uintptr_t memreq = pmm_alloc();
    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] vmm_alloc: %x\n", memreq);
    #endif
    
    kprintf("[TODO] Check for max kernelspace range\n");
    //vmm_break(context);
    vmm_map_page(context, context->next_kernelspace, memreq);
    context->next_kernelspace += PAGE_SIZE;

    kprintf("[DEBUG] [vmm] Will return %x\n", (context->next_kernelspace - PAGE_SIZE));

    return (context->next_kernelspace - PAGE_SIZE);
}

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
        context = kernel_context;

    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] Search/Create page table\n");
    #endif
    uint32_t* pagedir;
    if(flags & VMM_FLAG_ACTIVE) {
	pagedir = MAP_VMM_PAGEDIR;
    } else {
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
	if(flags & VMM_FLAG_ACTIVE)
	    page_table = (uint32_t*) (MAP_VMM_PAGETABLE + (PAGE_SIZE * pd_index));
	else
	    page_table = (uint32_t*) (pagedir[pd_index] & ~0xFFF);
	#ifdef DEBUG
	kprintf("[DEBUG] [vmm] *page_table set to %x\n", page_table);
	#endif
    } else {
	#ifdef DEBUG
	kprintf("[DEBUG] [vmm] PTE_PRESENT flag isn't set\n");
	kprintf("[DEBUG] [vmm] Create page_table\n");
	#endif
	//kprintf("[DEBUG] [vmm] NOP Loop\n");
	//for(uint32_t i = 0; i < 0x0FFFFFFF; i++);
        page_table = pmm_alloc();
	kprintf("[DEBUG] [vmm] Create page_table at %x\n", page_table);
	//vmm_map_page(context, page_table, page_table);
        //for(int i = 0; i < 1024; i++) {
        //    page_table[i] = 0;
        //}
        //context->pagedir[pd_index] = (uint32_t) page_table | PTE_PRESENT | PTE_WRITE | PTE_USER;
        pagedir[pd_index] = (uint32_t) page_table | PTE_PRESENT | PTE_WRITE | PTE_USER;
	vmm_map_page(context, (uintptr_t) (MAP_VMM_PAGETABLE + (PAGE_SIZE * pd_index)), page_table);
	page_table = (uintptr_t) (MAP_VMM_PAGETABLE + (PAGE_SIZE * pd_index));
        //pagedir[pd_index] = (uint32_t) page_table | PTE_PRESENT | PTE_WRITE;
	//kprintf("[DEBUG] [vmm] context->pagedir[pd_index %d]: %x\n", pd_index, context->pagedir[pd_index]);
	kprintf("[DEBUG] [vmm] pagedir[pd_index %d]: %x\n", pd_index, pagedir[pd_index]);
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
    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] Load %x as new page directory\n", context->pagedir);
    #endif
    //while(1);
    asm volatile("mov %0, %%cr3" : : "r" (context->pagedir));
    
    active_context = context;

    #ifdef DEBUG
    kprintf("[DEBUG] [vmm] New pagedir loaded\n");
    #endif

    vmm_map_context(context);

    kprintf("[DEBUG] [vmm] New context mapped\n");
}

struct vmm_context* vmm_create_context(void) {
    struct vmm_context* context = pmm_alloc();
    
    kprintf("[DEBUG] [vmm] Got %x as context addr\n", context);

    context->pagedir = pmm_alloc();
    kprintf("[DEBUG] [vmm] Got %x as context->pagedir addr\n", context->pagedir);
    for(int i = 0; i < 1024; i++) {
        context->pagedir[i] = NULL;
    }
    context->next_userspace = 0x40000000;
    context->next_kernelspace = MAP_VMM_KSPACE_START;

    return context;
}

uintptr_t vmm_get_mapping(struct vmm_context* context, uintptr_t virt) {
    kprintf("[DEBUG] [vmm] Get mapping for virt %x\n", virt);
    
    uint32_t page_index = virt / 0x1000;
    uint32_t pd_index = page_index / 1024;
    uint32_t pt_index = page_index % 1024;

    if(!(flags & VMM_FLAG_ACTIVE)) {
	kprintf("[DEBUG] [vmm] Paging not enabled\n");
	return NULL;
    }

    uintptr_t* pagedir;
    if(context = active_context) {
	kprintf("[DEBUG] [vmm] context is active_context\n");
	pagedir = MAP_VMM_PAGEDIR;
    } else {
	kprintf("[DEBUG] [vmm] Map temporary context %x\n", context);
	
	vmm_map_page(active_context, MAP_VMM_TEMP_CONTEXT, context);
	kprintf("[DEBUG] [vmm] Map temporary pagedir %x\n", context->pagedir);
	
	vmm_map_page(active_context, MAP_VMM_TEMP_PAGEDIR, context->pagedir);
	pagedir = MAP_VMM_TEMP_PAGEDIR;
    }
    uintptr_t* page_table;
    
    if(pagedir[pd_index] & PTE_PRESENT) {
	if(context == active_context) {
	    kprintf("[DEBUG] [vmm] active_context, find page_table in pagedir[pd_index %d]\n", pd_index);
	    
	    //page_table = (pagedir[pd_index] & ~0xFFF);
	    page_table = (uintptr_t) (MAP_VMM_PAGETABLE + (pd_index * 0x1000));
	    kprintf("[DEBUG] [vmm] Got page_table: %x\n", page_table);
	    
	} else {
	    kprintf("[DEBUG] [vmm] Map temporary pagetable\n");
	    
	    vmm_map_page(active_context, MAP_VMM_TEMP_PAGETABLE, (uintptr_t) (pagedir[pd_index] & ~0xFFF));
	    page_table = MAP_VMM_TEMP_PAGETABLE;
	}
    } else {
	return 0xFFFFFFFF;
    }

    kprintf("[DEBUG] [vmm] Try accessing page_table[pt_index %d] on %x\n", pt_index, page_table);
    kprintf("[DEBUG] [vmm] Physical addr is %x\n", page_table[pt_index] & ~0xFFF);
    
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
	kprintf("[DEBUG] [vmm] Mapping of context %x -> %x\n", context, MAP_VMM_CONTEXT);
	vmm_map_page(active_context, MAP_VMM_CONTEXT, context);
	
	kprintf("[DEBUG] [vmm] Mapping of context->pagedir %x -> %x\n", ((struct vmm_context*) MAP_VMM_CONTEXT)->pagedir, MAP_VMM_PAGEDIR);
	vmm_map_page(active_context, MAP_VMM_PAGEDIR, ((struct vmm_context*) MAP_VMM_CONTEXT)->pagedir);
	
	for(uint16_t i = 0; i < 1024; i++) {
	    if(((uintptr_t*) MAP_VMM_PAGEDIR)[i] & PTE_PRESENT) {
		kprintf("[DEBUG] [vmm] Mapping of page_table from context->pagedir[%d] %x -> %x\n", i, ((uintptr_t*) MAP_VMM_PAGEDIR)[i], MAP_VMM_PAGETABLE + (i * 0x1000));
		vmm_map_page(active_context, MAP_VMM_PAGETABLE + (i * 0x1000), ((uintptr_t*) MAP_VMM_PAGEDIR)[i]);
	    } else {
		kprintf("[DEBUG] [vmm] page_table at pagedir[%d] not yet set\n", i);
	    }
	}
    } else {
	kprintf("[DEBUG] [vmm] Mapping of context %x -> %x\n", context, MAP_VMM_CONTEXT);
	vmm_map_page(active_context, MAP_VMM_CONTEXT, context);
	
	kprintf("[DEBUG] [vmm] Mapping of context->pagedir %x -> %x\n", context->pagedir, MAP_VMM_PAGEDIR);
	vmm_map_page(active_context, MAP_VMM_PAGEDIR, context->pagedir);
	
	for(uint16_t i = 0; i < 1024; i++) {
	    if(((uintptr_t*) MAP_VMM_PAGEDIR)[i] & PTE_PRESENT) {
		kprintf("[DEBUG] [vmm] Mapping of page_table from context->pagedir[%d] %x -> %x\n", i, context->pagedir[i], MAP_VMM_PAGETABLE + (i * 0x1000));
		vmm_map_page(active_context, MAP_VMM_PAGETABLE + (i * 0x1000), context->pagedir[i]);
	    } else {
		kprintf("[DEBUG] [vmm] page_table at pagedir[%d] not yet set\n", i);
	    }
	}
    }
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
                //} else if(addr >= (void*) &mb_info && addr <= (void*) (&mb_info + sizeof(struct multiboot_info))) { // exclude multiboot structure
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
	    kprintf("[DEBUG] [vmm] Map mod structure at %x\n", addr);
            vmm_map_page(kernel_context, (uintptr_t) addr, (uintptr_t) addr);
	}
    }

    // Map kernel_context
    vmm_map_context(kernel_context);
    #ifdef DEBUG
    //kprintf("[DEBUG] [vmm] kernel_context mapping: %x at %x\n", kernel_context, MAP_VMM_CONTEXT);
    #endif
    //vmm_map_page(kernel_context, MAP_VMM_CONTEXT, kernel_context);
    
    // Map kernel_context->pagedir
    #ifdef DEBUG
    //kprintf("[DEBUG] [vmm] kernel_context->pagedir mapping: %x at %x\n", kernel_context->pagedir, MAP_VMM_PAGEDIR);
    #endif
    //vmm_map_page(kernel_context, MAP_VMM_PAGEDIR, kernel_context->pagedir);
    
    //vmm_map_page(kernel_context, MAP_VMM_PAGEDIR, (uint32_t*) (kernel_context->pagedir[i] & ~0xFFF));
    
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
    
    
    //vmm_break(kernel_context);
}

