#include <multiboot.h>

#ifndef LOADER_H
#define LOADER_H

void init_elf(void* image);
void init_mod(struct multiboot_mods* mod);

#endif
