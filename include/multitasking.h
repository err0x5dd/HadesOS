#include "system.h"

#ifndef MULTITASKING_H
#define MULTITASKING_H

#define SCHEDULE_FLAG_DO_NOT_DISTURB 0x01

struct task {
    struct cpu_state*   cpu_state;
    struct task*        next;
};

struct task* init_task(void* entry);
struct cpu_state* schedule(struct cpu_state* cpu);
void init_multitasking(void);
uint8_t get_schedule_flags(void);
void set_schedule_flags(uint8_t new_flags);

#endif
