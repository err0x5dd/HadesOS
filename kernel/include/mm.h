#include "multiboot.h"

#ifndef MM_H
#define MM_H

#define NULL ((void*) 0)

// set the page size as 4 kiB
#define PAGE_SIZE 4096

void pmm_init(struct multiboot_info* mb_info);
void* pmm_alloc(void);
void pmm_free(uintptr_t page);

#endif

