#include <elf.h>
#include <console.h>
#include <system.h>
#include <multitasking.h>
#include <mm.h>
#include <multiboot.h>

void init_mod(struct multiboot_mods* mod) {
    kprintf("[LOADER] [mod] Initialize module: %s\n", mod->string);


    //init_elf(NULL);
}
