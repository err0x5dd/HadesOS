// pmm = physical memory manager

#include <stdint.h>
#include <mm.h>
#include <multiboot.h>
#include <system.h>
#include <console.h>

// Uncomment for debug output
//#define DEBUG
//#define DEBUG_LVL2
//#define DEBUG_LVL3

extern const void kernel_start;
extern const void kernel_end;

#define PMM_BITMAP_SIZE 32768
// 0 -> page is used
// 1 -> page is free
static uint32_t bitmap[PMM_BITMAP_SIZE];
static uint32_t next_bit = 0xbadc0de;

void pmm_init(struct multiboot_info* mb_info) {
    struct multiboot_mmap* mmap = mb_info->mbs_mmap_addr;
    struct multiboot_mmap* mmap_end = (void*)
        ((uintptr_t) mb_info->mbs_mmap_addr + mb_info->mbs_mmap_length);
    
    struct multiboot_mods* mods = mb_info->mbs_mods_addr; 
    
    kprintf("[pmm] Modules loaded: %d\n", mb_info->mbs_mods_count);
    
    for(uint32_t i = 0; i < mb_info->mbs_mods_count; i++) {
        kprintf("[pmm] Module %d '%s':\n", i, mods[i].string);
        kprintf("[pmm]  Start: %x\n", mods[i].mod_start);
        kprintf("[pmm]  End:   %x\n", mods[i].mod_end);
    }
    
    uint32_t mods_counter = 0;

    while(mmap < mmap_end) {
        if(mmap->type == 1) {
            void* addr = (void*) mmap->base;
            void* end_addr = (void*) ((uint64_t) addr + mmap->length);

            while(addr < end_addr) {
                if(addr >= (void*) &kernel_start && addr <= (void*) &kernel_end) { // exclude kernel
                    #ifdef DEBUG
                    kprintf("[DEBUG] [pmm] %x is used by kernel -> no free memory page\n", addr);
                    #endif
                //} else if(addr >= (void*) &mb_info && addr <= (void*) (&mb_info + sizeof(struct multiboot_info))) { // exclude multiboot structure
                } else if(addr >= (void*) mb_info && addr <= (void*) (mb_info + sizeof(struct multiboot_info))) { // exclude multiboot structure
                    #ifdef DEBUG
                    kprintf("[DEBUG] [pmm] %x is used by multiboot structure -> no free memory page\n", addr);
                    #endif 
                } else if(addr >= (void*) mb_info->mbs_mmap_addr && addr <= (void*) (mb_info->mbs_mmap_addr + mb_info->mbs_mmap_length)) { // exclude mmap structure
                    #ifdef DEBUG
                    kprintf("[DEBUG] [pmm] %x is used by mmap structure -> no free memory page\n", addr);
                    #endif 
                } else if(addr >= (void*) mb_info->mbs_mods_addr && addr <= (void*) (mb_info->mbs_mods_addr + (sizeof(struct multiboot_mods) * mb_info->mbs_mods_count))) { // exclude mmap structure
                    #ifdef DEBUG
                    kprintf("[DEBUG] [pmm] %x is used by mods structure -> no free memory page\n", addr);
                    #endif 
                } else if(addr >= (void*) mods[mods_counter].mod_start && addr <= (void*) mods[mods_counter].mod_end) { // exclude modules
                    #ifdef DEBUG
                    kprintf("[DEBUG] [pmm] %x is used by module %s\n", addr, mods[mods_counter].string);
                    #endif
                    
                    // Increment module counter when this page is the module end
                    if((addr + PAGE_SIZE) > (void*) mods[mods_counter].mod_end && (mods_counter + 1) < mb_info->mbs_mods_count) {
                        mods_counter++;
                    }
                } else {
                    pmm_free(addr);
                }
                addr += PAGE_SIZE;
            }
        }
        mmap++;
    }
    
    pmm_search_page();
}


uint32_t pmm_alloc(void) {
    uint32_t page = NULL;
    
    #ifdef DEBUG_LVL2
    kprintf("[DEBUG_LVL2] [pmm] Get new address\n");
    #endif
    
    if(next_bit == 0xbadc0de) {
	pmm_search_page();
    }
    
    #ifdef DEBUG_LVL3
    kprintf("[DEBUG_LVL3] [pmm] next_bit: %d / %x\n", next_bit, next_bit);
    #endif

    page = next_bit * PAGE_SIZE;
    bitmap[next_bit / 32] &= ~(1 << (next_bit % 32));

    next_bit = 0xbadc0de;

    #ifdef DEBUG
    kprintf("[DEBUG] [pmm] Got address %x\n", page);
    #endif
    
    return page;
}

void pmm_free(uintptr_t page) {
    #ifdef DEBUG_LVL2
    kprintf("[DEBUG_LVL2] [pmm] Free %x\n", page);
    #endif

    uint16_t bitmap_selector = (uint32_t) page / PAGE_SIZE / 32;
    uint8_t bit_selector = (uint32_t) (page / PAGE_SIZE) % 32;
    
    #ifdef DEBUG_LVL3
    kprintf("[DEBUG_LVL3] [pmm] bitmap_selector: %d\n", bitmap_selector);
    kprintf("[DEBUG_LVL3] [pmm] bit_selector: %d\n", bit_selector);
    kprintf("[DEBUG_LVL3] [pmm] next_bit: %d / %x\n", next_bit, next_bit);
    #endif

    bitmap[bitmap_selector] |= (1 << bit_selector);

    if(next_bit == 0xbadc0de) {
        next_bit = (uint32_t) page / PAGE_SIZE;
    }
}

void pmm_search_page(void) {
    #ifdef DEBUG_LVL2
    kprintf("[DEBUG_LVL2] [pmm] Searching for next free page\n");
    #endif
    // Start search at 1 MiB
    for(uint32_t i = 8; i < PMM_BITMAP_SIZE; i++) {
	#ifdef DEBUG_LVL3
	kprintf("[DEBUG_LVL3] [pmm] bitmap[%d]: %x / %b\n", i, bitmap[i], bitmap[i]);
	#endif
	if(bitmap[i] != 0x00000000) { // check for free pages
	    for(uint8_t j = 0; j < 32; j++) {
		if(bitmap[i] & (1<<j)) {
		    next_bit = i*32 + j;
		    #ifdef DEBUG_LVL3
		    kprintf("[DEBUG_LVL3] [pmm] New next Bit: %d / %x\n", next_bit, next_bit);
		    #endif
		    return;
		} else {
		    #ifdef DEBUG_LVL3
		    kprintf("[DEBUG_LVL3] [pmm] Bit %d / %x is not free\n", i * 32 + j, i * 32 + j);
		    #endif
		}
	    }
	}
    }
}
