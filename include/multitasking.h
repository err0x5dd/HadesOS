#include "system.h"

#ifndef MULTITASKING_H
#define MULTITASKING_H

struct cpu_state* schedule(struct cpu_state* cpu);
void init_multitasking(void);

#endif
