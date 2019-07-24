#include <elf.h>
#include <console.h>
#include <system.h>
#include <multitasking.h>
#include <mm.h>
#include <multiboot.h>

extern struct vmm_active_context* active_context;

void init_mod(struct multiboot_mods* mod) {
    kprintf("[loader] [mod] Initialize module: %s\n", mod->string);

    kprintf("[TODO] [loader] [mod] Add context generation\n");

    //struct vmm_context* context = vmm_create_context();

    //kprintf("[DEBUG] [loader] [mod] New context: %x\n", context);

    //init_elf(active_context, mod->mod_start);
    init_elf(mod->mod_start);
}
