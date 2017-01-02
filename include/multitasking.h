#include "system.h"

#ifndef MULTITASKING_H
#define MULTITASKING_H

struct cpu_state* init_task(uint8_t* stack, uint8_t* user_stack, void* entry);
struct cpu_state* schedule(struct cpu_state* cpu);
void init_multitasking(void);

#endif
