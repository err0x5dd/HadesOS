#include "multiboot.h"

#ifndef MM_H
#define MM_H

#define NULL ((void*) 0)

// set the page size as 4 kiB
#define PAGE_SIZE 4096

#define PTE_PRESENT 0x1
#define PTE_WRITE 0x2
#define PTE_USER 0x4

struct vmm_context {
    uint32_t* pagedir;
    uintptr_t next_userspace;
    uintptr_t next_kernelspace;
} __attribute__((packed));

void pmm_init(struct multiboot_info* mb_info);
uint32_t pmm_alloc(void);
void pmm_free(uintptr_t page);

void vmm_init(struct multiboot_info* mb_info);
void* vmm_alloc(struct vmm_context* context);
void* vmm_alloc_kernel(struct vmm_context* context);
void* vmm_alloc_at(struct vmm_context* context, uintptr_t vaddr);
int vmm_map_page(struct vmm_context* context, uintptr_t virt, uintptr_t phys);

#endif

