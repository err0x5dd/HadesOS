#include "system.h"
#include "multiboot.h"

#ifndef MULTITASKING_H
#define MULTITASKING_H

#define SCHEDULE_FLAG_DO_NOT_DISTURB 0x01

struct task {
    struct cpu_state*   cpu_state;
    struct task*        next;
};

struct task* init_task(void* entry);
struct cpu_state* schedule(struct cpu_state* cpu);
void init_multitasking(struct multiboot_info* mb_info);
uint8_t get_schedule_flags(void);
void set_schedule_flags(uint8_t new_flags);
void switch_task(void);

#endif
