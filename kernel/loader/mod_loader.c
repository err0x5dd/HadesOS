#include "../include/elf.h"
#include "../include/console.h"
#include "../include/system.h"
#include "../include/multitasking.h"
#include "../include/mm.h"
#include "../include/multiboot.h"

void init_mod(struct multiboot_mods* mod) {
    kprintf("[LOADER] [mod] Initialize module: %s\n", mod->string);


    //init_elf(NULL);
}
