#include "multiboot.h"

#ifndef MM_H
#define MM_H

#define NULL ((void*) 0)

// set the page size as 4 kiB
#define PAGE_SIZE 4096

#define PTE_PRESENT 0x1
#define PTE_WRITE 0x2
#define PTE_USER 0x4

//#define MEMSTACK_ADDR 0x40000000 - sizeof(struct memory_stack)
#define MEMSTACK_ADDR 0x40000000 - PAGE_SIZE

struct memory_stack {
    void*   start;
    void*   prev_memstack;
} __attribute__((packed));

void pmm_init(struct multiboot_info* mb_info);
void* pmm_alloc(void);
void pmm_free(uintptr_t page);



struct vmm_context {
    uint32_t* pagedir;
    uintptr_t next_user_addr;
} __attribute__((packed));

//void vmm_init(void);
void vmm_init(struct multiboot_info* mb_info);
//void* vmm_alloc(void);
void* vmm_alloc(struct vmm_context* context);
int vmm_map_page(struct vmm_context* context, uintptr_t virt, uintptr_t phys);

#endif

