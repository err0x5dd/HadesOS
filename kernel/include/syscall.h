#include "system.h"


#ifndef SYSCALL_H
#define SYSCALL_H

#define SYSCALL_PRINTC 0
#define SYSCALL_PRINTS 1
#define SYSCALL_SWITCH_TASK 2
#define SYSCALL_PMM_ALLOC 3

struct cpu_state* syscall(struct cpu_state* cpu);

#endif
