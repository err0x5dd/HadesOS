#include "../include/elf.h"
#include "../include/console.h"
#include "../include/system.h"
#include "../include/multitasking.h"
#include "../include/mm.h"

// Uncomment for debug info
#define DEBUG

void init_elf(void* file) {
    Elf32_Ehdr* ehdr = file;

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
    
    kprintf("[DEBUG] [loader] [elf] e_ident:     ");
    for(uint8_t i = 0; i < EI_PAD; i++) {
	kprintf("%x ", ehdr->e_ident[i]);
    }
    kprintf("\n");
    kprintf("[DEBUG] [loader] [elf] e_type:      %x\n", ehdr->e_type);
    kprintf("[DEBUG] [loader] [elf] e_machine:   %x\n", ehdr->e_machine);
    kprintf("[DEBUG] [loader] [elf] e_version:   %x\n", ehdr->e_version);
    kprintf("[DEBUG] [loader] [elf] e_entry:     %x\n", ehdr->e_entry);
    kprintf("[DEBUG] [loader] [elf] e_phoff:     %x\n", ehdr->e_phoff);
    kprintf("[DEBUG] [loader] [elf] e_shoff:     %x\n", ehdr->e_shoff);
    kprintf("[DEBUG] [loader] [elf] e_flags:     %x\n", ehdr->e_flags);
    kprintf("[DEBUG] [loader] [elf] e_ehsize:    %x\n", ehdr->e_ehsize);
    kprintf("[DEBUG] [loader] [elf] e_phentsize: %x\n", ehdr->e_phentsize);
    kprintf("[DEBUG] [loader] [elf] e_phnum:     %x\n", ehdr->e_phnum);
    kprintf("[DEBUG] [loader] [elf] e_shentsize: %x\n", ehdr->e_shentsize);
    kprintf("[DEBUG] [loader] [elf] e_shnum:     %x\n", ehdr->e_shnum);
    kprintf("[DEBUG] [loader] [elf] e_shstrndx:  %x\n", ehdr->e_shstrndx);
    kprintf("\n");
    kprintf("Loop\n");
    while(1);
    #endif

    //kprintf("[loader] [elf] Start new task at %x\n", file);
    init_task((void*) file);
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


