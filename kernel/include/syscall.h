#include "system.h"


#ifndef SYSCALL_H
#define SYSCALL

#define SYSCALL_PRINTC 0
#define SYSCALL_PRINTS 1
#define SYSCALL_SWITCH_TASK 2

struct cpu_state* syscall(struct cpu_state* cpu);

#endif
