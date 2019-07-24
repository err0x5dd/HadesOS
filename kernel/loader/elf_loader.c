#include <elf.h>
#include <console.h>
#include <system.h>
#include <multitasking.h>
#include <mm.h>
#include <mmap.h>

// Uncomment for debug info
#define DEBUG

extern struct vmm_context* active_context;

//void init_elf(struct vmm_context* context, void* file) {
void init_elf(void* file) {
    Elf32_Ehdr* ehdr = file;
    Elf32_Phdr* phdr;


//--------------------------------
/*
    struct vmm_context* temp_v_active_context = (struct vmm_context*) MAP_VMM_CONTEXT;
    struct vmm_context* temp_v_context = (struct vmm_context*) MAP_VMM_TEMP_CONTEXT;
    uint32_t* temp_v_active_pagedir = (uint32_t*) MAP_VMM_PAGEDIR;
    uint32_t* temp_v_pagedir = (uint32_t*) MAP_VMM_TEMP_PAGEDIR;

    struct vmm_context* temp_p_active_context = active_context;
    struct vmm_context* temp_p_context = context;
    uint32_t* temp_p_active_pagedir = temp_v_active_context->pagedir;
    uint32_t* temp_p_pagedir;

    vmm_map_page(NULL, temp_v_context, temp_p_context);
    temp_p_pagedir = temp_v_context->pagedir;
    vmm_map_page(NULL, temp_v_pagedir, temp_p_pagedir);
*/
//--------------------------------


    if(check_elf(ehdr) != 0) {
	#ifdef DEBUG
	kprintf("[DEBUG] [loader] [elf] Not a valid ELF file\n");
	#endif

	return;
    }

    if(ehdr->e_machine != EM_386) {
	#ifdef DEBUG
	kprintf("[DEBUG] [loader] [elf] ELF file is not for i386\n");
	#endif

	return;
    }

    if(ehdr->e_type != ET_EXEC) {
	#ifdef DEBUG
	kprintf("[DEBUG] [loader] [elf] Not an executable file\n");
	#endif

	return;
    }

    #ifdef DEBUG
    kprintf("\n");
    print_ehdr(ehdr);
    kprintf("\n");

    for(int i = 0; i < ehdr->e_phnum; i++) {
	print_phdr((Elf32_Addr) ehdr + ehdr->e_phoff + (i * ehdr->e_phentsize));
	kprintf("\n");
    }
    #endif

    for(int i = 0; i < ehdr->e_phnum; i++) {
	phdr = ((Elf32_Addr) ehdr + ehdr->e_phoff + (i * ehdr->e_phentsize));
	if(phdr->p_type == PT_LOAD) {
	    kprintf("[DEBUG] [loader] [elf] Load program segment %d at %x\n", i, phdr->p_vaddr);
	    //if(vmm_alloc_at(context, phdr->p_vaddr) != phdr->p_vaddr)
	    if(vmm_alloc_at(NULL, phdr->p_vaddr) != phdr->p_vaddr)
		panic("[loader] [elf] Could not load program segment -> vaddr already in use\n");

	    memcpy(phdr->p_vaddr, ((Elf32_Addr) ehdr + phdr->p_offset), phdr->p_filesz);
	    //kprintf("\n[DEBUG] [loader] [elf] context: %x\n", context);
	    //kprintf("\n[DEBUG] [loader] [elf] context->next_userspace: %x\n", context->next_userspace);
	}
    }

    //kprintf("Loop\n");
    //while(1);
    
    kprintf("[loader] [elf] Start new task at %x\n", file);
    init_task((void*) ehdr->e_entry);
}

int check_elf(Elf32_Ehdr* ehdr) {
    // Check for ELF magic
    if(ehdr->e_ident[EI_MAG0] == ELFMAG0 && ehdr->e_ident[EI_MAG1] == ELFMAG1 && ehdr->e_ident[EI_MAG2] == ELFMAG2 && ehdr->e_ident[EI_MAG3] == ELFMAG3) {
	#ifdef DEBUG
	kprintf("[DEBUG] [loader] [elf] Valid ELF magic\n");
	#endif

	// Check for 32-bit file
	if(ehdr->e_ident[EI_CLASS] == ELFCLASS32) {
	    #ifdef DEBUG
	    kprintf("[DEBUG] [loader] [elf] Valid 32-bit file\n");
	    #endif

	    // Check for LSB encoding
	    if(ehdr->e_ident[EI_DATA] == ELFDATA2LSB) {
		#ifdef DEBUG
		kprintf("[DEBUG] [loader] [elf] Valid LSB encoding\n");
		#endif

		// Check for ELF version
		if(ehdr->e_ident[EI_VERSION] == EV_CURRENT && ehdr->e_version == EV_CURRENT) {
		    #ifdef DEBUG
		    kprintf("[DEBUG] [loader] [elf] Valid ELF version\n");
		    #endif

		    return 0;
		} else {
		    #ifdef DEBUG
		    kprintf("[DEBUG] [loader] [elf] Not a valid ELF version\n");
		    #endif

		    return 4;
		}
	    } else {
		#ifdef DEBUG
		kprintf("[DEBUG] [loader] [elf] Not a valid LSB encoding\n");
		#endif

		return 3;
	    }
	} else {
	    #ifdef DEBUG
	    kprintf("[DEBUG] [loader] [elf] Not a 32-bit file\n");
	    #endif

	    return 2;
	}
    } else {
	#ifdef DEBUG
	kprintf("[DEBUG] [loader] [elf] Not a valid ELF magic\n");
	#endif

	return 1;
    }
}

void print_ehdr(Elf32_Ehdr* ehdr) {
    kprintf("[DEBUG] [loader] [elf] ehdr:        %x\n", ehdr);
    kprintf("[DEBUG] [loader] [elf] e_ident:     ");
    for(uint8_t i = 0; i < EI_PAD; i++) {
	kprintf("%x ", ehdr->e_ident[i]);
    }
    kprintf("\n");
    kprintf("[DEBUG] [loader] [elf] e_type:      %x\n", ehdr->e_type);
    kprintf("[DEBUG] [loader] [elf] e_machine:   %x\n", ehdr->e_machine);
    kprintf("[DEBUG] [loader] [elf] e_version:   %x\n", ehdr->e_version);
    kprintf("[DEBUG] [loader] [elf] e_entry:     %x\n", ehdr->e_entry);
    kprintf("[DEBUG] [loader] [elf] e_phoff:     %x / %d\n", ehdr->e_phoff, ehdr->e_phoff);
    kprintf("[DEBUG] [loader] [elf] e_shoff:     %x / %d\n", ehdr->e_shoff, ehdr->e_shoff);
    kprintf("[DEBUG] [loader] [elf] e_flags:     %x\n", ehdr->e_flags);
    kprintf("[DEBUG] [loader] [elf] e_ehsize:    %x / %d\n", ehdr->e_ehsize, ehdr->e_ehsize);
    kprintf("[DEBUG] [loader] [elf] e_phentsize: %x / %d\n", ehdr->e_phentsize, ehdr->e_phentsize);
    kprintf("[DEBUG] [loader] [elf] e_phnum:     %x / %d\n", ehdr->e_phnum, ehdr->e_phnum);
    kprintf("[DEBUG] [loader] [elf] e_shentsize: %x / %d\n", ehdr->e_shentsize, ehdr->e_shentsize);
    kprintf("[DEBUG] [loader] [elf] e_shnum:     %x / %d\n", ehdr->e_shnum, ehdr->e_shnum);
    kprintf("[DEBUG] [loader] [elf] e_shstrndx:  %x / %d\n", ehdr->e_shstrndx, ehdr->e_shstrndx);
}

void print_phdr(Elf32_Phdr* phdr) {
    kprintf("[DEBUG] [loader] [elf] phdr:        %x\n", phdr);
    kprintf("[DEBUG] [loader] [elf] p_type:      %x\n", phdr->p_type);
    kprintf("[DEBUG] [loader] [elf] p_offset:    %x\n", phdr->p_offset);
    kprintf("[DEBUG] [loader] [elf] p_vaddr:     %x\n", phdr->p_vaddr);
    kprintf("[DEBUG] [loader] [elf] p_paddr:     %x\n", phdr->p_paddr);
    kprintf("[DEBUG] [loader] [elf] p_filesz:    %x\n", phdr->p_filesz);
    kprintf("[DEBUG] [loader] [elf] p_memsz:     %x\n", phdr->p_memsz);
    kprintf("[DEBUG] [loader] [elf] p_flags:     %x\n", phdr->p_flags);
    kprintf("[DEBUG] [loader] [elf] p_align:     %x\n", phdr->p_align);
}
