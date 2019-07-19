#include "../include/elf.h"
#include "../include/console.h"
#include "../include/system.h"
#include "../include/multitasking.h"
#include "../include/mm.h"

void init_elf(void* image) {
    struct elf_header* header = image;
    struct elf_program_header* ph;
    int i;

    if(header->magic != ELF_MAGIC) {
        kprintf("ELF-Magic not valid!\n");
        return;
    }

    ph = (struct elf_program_header*) (((char*) image) + header->ph_offset);
    for(i = 0; i < header->ph_entry_count; i++, ph++) {
        void* dest = (void*) ph->virt_addr;
        void* src = ((char*) image) + ph->offset;
        
        if(ph->type != 1) {
            continue;
        }

	kprintf("[DEBUG] [elf] Alloc pages for module\n");

	for(uint32_t j = 0; j < dest + ph->file_size; j += PAGE_SIZE) {
	    uintptr_t memreq = pmm_alloc();
	    vmm_map_page(NULL, dest + j, memreq);
	}

        memset(dest, 0, ph->mem_size);
        memcpy(dest, src, ph->file_size);
    }

    kprintf("Start new task at %x\n", header->entry);
    init_task((void*) header->entry);
}
