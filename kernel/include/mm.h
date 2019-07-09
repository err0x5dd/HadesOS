#include "multiboot.h"

#ifndef MM_H
#define MM_H

#define NULL ((void*) 0)

// set the page size as 4 kiB
#define PAGE_SIZE 4096

#define PTE_PRESENT 0x1
#define PTE_WRITE 0x2
#define PTE_USER 0x4

//// FIFO stack for pages
//struct stack {
//    uint16_t nextin;	    // 2 Bytes
//    uint16_t nextout;	    // 2 Bytes
//    uint32_t page[1023];    // 4 Bytes * 1023 to fill a page
//} __attribute__((packed));

struct vmm_context {
    uint32_t* pagedir;
} __attribute__((packed));

void pmm_init(struct multiboot_info* mb_info);
uint32_t pmm_alloc(void);
void pmm_free(uintptr_t page);

void vmm_init(struct multiboot_info* mb_info);
void* vmm_alloc(struct vmm_context* context);
int vmm_map_page(struct vmm_context* context, uintptr_t virt, uintptr_t phys);
uintptr_t vmm_get_phys_map(struct vmm_context* context, uintptr_t virt);

#endif

