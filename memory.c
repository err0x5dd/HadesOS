// memory

#include "include/stdint.h"
#include "include/mm.h"
#include "include/multiboot.h"
#include "include/system.h"

// Auskommentieren für eine erweiterte Ausgabe
#define DEBUG

#ifdef DEBUG
#include "include/console.h"
#endif

extern const void kernel_start;
extern const void kernel_end;

void memory_detect(struct multiboot_info* mb_info) {
    struct multiboot_mmap* mmap = mb_info->mbs_mmap_addr;
    struct multiboot_mmap* mmap_end = (void*)
        ((uintptr_t) mb_info->mbs_mmap_addr + mb_info->mbs_mmap_length);
   uint64_t detected_mem = 0x00; 
    while(mmap < mmap_end) {
        uintptr_t addr_start = mmap->base;
        uintptr_t addr_end = addr_start + mmap->length - 1;
        if (mmap->type == 1) {
            detected_mem += mmap->length;
        }
        kprintf("%x - %x Type: %x\n", addr_start, addr_end, mmap->type);
        
        mmap++;
    }
    kprintf("Detected memory: %d MiB\n", detected_mem / 1024 / 1024);
    
}

