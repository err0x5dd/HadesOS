#include <syscall.h>
#include <console.h>
#include <stdint.h>
#include <multitasking.h>
#include <mm.h>
#include <keyboard.h>

// Uncomment for debug output
//#define DEBUG

struct cpu_state* syscall(struct cpu_state* cpu) {
    struct cpu_state* new_cpu = cpu;

    switch(cpu->eax) {
        case SYSCALL_PRINTC:
            kprintf("%c", cpu->ebx);
            break;
        case SYSCALL_PRINTS:
            #ifdef DEBUG
            kprintf("Read from Addr: %x\n", cpu->ebx);
            #endif
            kprintf("%s", cpu->ebx);
            break;
        case SYSCALL_SWITCH_TASK:
            new_cpu = schedule(cpu);
            break;
        case SYSCALL_PAGE_ALLOC:
            #ifdef DEBUG
            kprintf("eax: %x\n", new_cpu->eax);
            #endif
            ;
            //void* req = pmm_alloc();
            void* req = vmm_alloc(NULL);
            new_cpu->eax = (uint32_t) req;
            #ifdef DEBUG
            kprintf("Addr: %x\n", (uint32_t) req);
            kprintf("eax: %x\n", new_cpu->eax);
            #endif
            break;
	case SYSCALL_GETC:
	    ;
	    char c = getc();
	    new_cpu->eax = (uint32_t) c;
	    break;
    }

    return new_cpu;
}


