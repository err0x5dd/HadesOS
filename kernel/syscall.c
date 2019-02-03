#include "include/syscall.h"
#include "include/console.h"
#include "include/stdint.h"
#include "include/multitasking.h"

struct cpu_state* syscall(struct cpu_state* cpu) {
    struct cpu_state* new_cpu = cpu;

    switch(cpu->eax) {
        case SYSCALL_PRINTC:
            kprintf("%c", cpu->ebx);
            break;
        case SYSCALL_PRINTS:
            kprintf("%s", &(cpu->ebx));
            break;
        case SYSCALL_SWITCH_TASK:
            new_cpu = schedule(cpu);
    }

    return new_cpu;
}


