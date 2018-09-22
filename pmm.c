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

// 32 (breite pro eintrag) * 32768 (einträge) * 4096 (page größe) = 4 GiB
// 0 = Speicher belegt / 1 = Speicher frei
// 1 Bit representiert eine Page von 4096 Byte
#define BITMAP_SIZE 32768
static uint32_t bitmap[BITMAP_SIZE];

static void pmm_mark_used(void* page);

extern const void kernel_start;
extern const void kernel_end;

void pmm_init(struct multiboot_info* mb_info) {
    struct multiboot_mmap* mmap = mb_info->mbs_mmap_addr;
    struct multiboot_mmap* mmap_end = (void*)
        ((uintptr_t) mb_info->mbs_mmap_addr + mb_info->mbs_mmap_length);
    
    memset(bitmap, 0, sizeof(bitmap));
    
    while(mmap < mmap_end) {
        if(mmap->type == 1) {
            uintptr_t addr = mmap->base;
            uintptr_t end_addr = addr + mmap->length;
            
            while(addr < end_addr) {
                pmm_free((void*) addr);
                addr += 0x1000;
            }
        }
        mmap++;
    }
    
    uintptr_t addr = (uintptr_t) &kernel_start;
    while(addr < (uintptr_t) &kernel_end) {
        pmm_mark_used((void*) addr);
        addr += 0x1000;
    }
}

void* pmm_alloc(void) {
    int i, j;
    
    for(i = 0; i < BITMAP_SIZE; i++) {
        if(bitmap[i] > 0) { // Prüfe ob ein Bit frei ist
            for(j = 0; j < 32; j++) {
                if(bitmap[i] & (1 << j)) {
                    bitmap[i] &= ~(1 << j);
                    #ifdef DEBUG
                    kprintf("%x wird uebergeben\n", (uintptr_t) ((i * 32 + j) * 4096));
                    #endif
                    return (void*) ((i * 32 + j) * 4096);
                }
            }
        }
    }
    return NULL;
}

static void pmm_mark_used(void* page) {
    uintptr_t index = (uintptr_t) page / 4096;
    bitmap[index / 32] |= (1 << (index % 32));
    #ifdef DEBUG
    //kprintf("%x als belegt markiert\n", (uintptr_t)page);
    #endif
}

void pmm_free(void* page) {
    uintptr_t index = (uintptr_t) page / 4096;
    bitmap[index / 32] &= ~(1 << (index % 32));
    #ifdef DEBUG
    //kprintf("%x als frei markiert\n", (uintptr_t)page);
    #endif
}
