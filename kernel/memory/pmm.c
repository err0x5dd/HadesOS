// pmm = physical memory manager

#include "../include/stdint.h"
#include "../include/mm.h"
#include "../include/multiboot.h"
#include "../include/system.h"

// Uncomment for debug output
//#define DEBUG

#ifdef DEBUG
#include "../include/console.h"
#endif


// Bitmap for pages under 1 MB
static uint32_t bitmap[8]; 

// Pointer to stack for every page over 1 MB
// When there is no free memory page, then stack points to NULL
struct stack {
    struct stack* prev;
    uintptr_t page[127];
} __attribute__((packed));

static struct stack* stack_pointer = NULL;
static uint32_t stack_counter = 0;

extern const void kernel_start;
extern const void kernel_end;

void pmm_init(struct multiboot_info* mb_info) {
    struct multiboot_mmap* mmap = mb_info->mbs_mmap_addr;
    struct multiboot_mmap* mmap_end = (void*)
        ((uintptr_t) mb_info->mbs_mmap_addr + mb_info->mbs_mmap_length);
    
    struct multiboot_mods* mods = mb_info->mbs_mods_addr; 
    
    kprintf("Modules loaded: %d\n", mb_info->mbs_mods_count);
    
    for(uint32_t i = 0; i < mb_info->mbs_mods_count; i++) {
        kprintf("Module %d '%s':\n", i, mods[i].string);
        kprintf("  Start: %x\n", mods[i].mod_start);
        kprintf("  End:   %x\n", mods[i].mod_end);
    }
    
    uint32_t mods_counter = 0;

    while(mmap < mmap_end) {
        if(mmap->type == 1) {
            void* addr = (void*) mmap->base;
            void* end_addr = (void*) ((uint64_t) addr + mmap->length);
            

            while(addr < end_addr) {
                if(addr >= (void*) &kernel_start && addr <= (void*) &kernel_end) { // exclude kernel
                    #ifdef DEBUG
                    kprintf("%x is used by kernel -> no free memory page\n", addr);
                    #endif
                } else if(addr >= (void*) &mb_info && addr <= (void*) (&mb_info + sizeof(struct multiboot_info))) { // exclude multiboot structure
                    #ifdef DEBUG
                    kprintf("%x is used by multiboot structure -> no free memory page\n", addr);
                    #endif 
                } else if(addr >= (void*) mb_info->mbs_mmap_addr && addr <= (void*) (mb_info->mbs_mmap_addr + mb_info->mbs_mmap_length)) { // exclude mmap structure
                    #ifdef DEBUG
                    kprintf("%x is used by mmap structure -> no free memory page\n", addr);
                    #endif 
                } else if(addr >= (void*) mb_info->mbs_mods_addr && addr <= (void*) (mb_info->mbs_mods_addr + (sizeof(struct multiboot_mods) * mb_info->mbs_mods_count))) { // exclude mmap structure
                    #ifdef DEBUG
                    kprintf("%x is used by mods structure -> no free memory page\n", addr);
                    #endif 
                } else if(addr >= (void*) mods[mods_counter].mod_start && addr <= (void*) mods[mods_counter].mod_end) { // exclude modules
                    #ifdef DEBUG
                    kprintf("%x is used by module %s\n", addr, mods[mods_counter].string);
                    #endif
                    
                    // Increment module counter when this page is the module end
                    if((addr + PAGE_SIZE) > (void*) mods[mods_counter].mod_end && (mods_counter + 1) < mb_info->mbs_mods_count) {
                        mods_counter++;
                    }
                } else {
                    pmm_free(addr);
                }
                addr += PAGE_SIZE;
            }
        }
        mmap++;
    }

}


void* pmm_alloc(void) {
    uintptr_t page = NULL;
    
    //kprintf("[TODO] [pmm] Add allocation for pages less 1 MiB\n");
    
    #ifdef DEBUG
    kprintf("[DEBUG] [pmm] Get new address from stack\n");
    #endif
    
    if(stack_counter > 0) {
        stack_counter--;
        page = stack_pointer->page[stack_counter];
    } else {
        page = stack_pointer;
        
        if(stack_pointer->prev == stack_pointer) {
            panic("No free memory pages\n");
        } else {
            stack_pointer = stack_pointer->prev;
            stack_counter = 127;
        }
        
        #ifdef DEBUG
        kprintf("[DEBUG] [pmm] Moved stack to %x\n", stack_pointer);
        #endif
    }
    
    #ifdef DEBUG
    kprintf("[DEBUG] [pmm] Got address %x\n", page);
    #endif
    return page;
}

void pmm_free(uintptr_t page) {
    if(page < 0x100000) { // Pages under 1 MB are managed over bitmap
        #ifdef DEBUG
        kprintf("[DEBUG] [pmm] Address %x is under 1MiB -> use bitmap\n", page);
        #endif
        
        // Mark page in bitmap as free
        bitmap[page / 0x1000 / 32] |= (1 << ((page / 0x1000) % 32));
        
        #ifdef DEBUG
        kprintf("[DEBUG] [pmm] Bit %d in array %d set\n", ((page / 0x1000) % 32), (page / 0x1000 / 32));
        for(int i = 0; i < 8; i++) {
            kprintf("[DEBUG] [pmm] Bitmap[%d]: %b\n", i, bitmap[i]);
        }
        #endif
    } else {
        if(stack_pointer == NULL) {
            #ifdef DEBUG
            kprintf("[DEBUG] [pmm] Create stack at %x\n", page);
            #endif
            stack_pointer = page;
            stack_pointer->prev = stack_pointer;
        } else {
            if(stack_counter == 127) {
                #ifdef DEBUG
                kprintf("[DEBUG] [pmm] Stack is full -> Create new stack at %x\n", page);
                kprintf("[DEBUG] [pmm] stack_pointer: %x\n", stack_pointer);
                kprintf("[DEBUG] [pmm] &stack_pointer: %x\n", &stack_pointer);
                kprintf("[DEBUG] [pmm] stack_pointer->prev: %x\n", stack_pointer->prev);
                kprintf("[DEBUG] [pmm] --------\n");
                #endif

                struct stack* new_stack = page;
                new_stack->prev = stack_pointer;
                stack_pointer = new_stack;
                stack_counter = 0;
                
                #ifdef DEBUG
                kprintf("[DEBUG] [pmm] stack_pointer: %x\n", stack_pointer);
                kprintf("[DEBUG] [pmm] &stack_pointer: %x\n", &stack_pointer);
                kprintf("[DEBUG] [pmm] stack_pointer->prev: %x\n", stack_pointer->prev);
                #endif                
            } else {
                #ifdef DEBUG
                kprintf("[DEBUG] [pmm] Add address %x to stack\n", page);
                #endif
                stack_pointer->page[stack_counter] = page;
                stack_counter++;
            }
        }
    }
    
}

