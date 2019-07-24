#include <system.h>
#include <multiboot.h>
#include <mm.h>

#ifndef MULTITASKING_H
#define MULTITASKING_H

#define SCHEDULE_FLAG_DO_NOT_DISTURB 0x01

struct task {
    uint32_t pid;
    struct cpu_state*   cpu_state;
    struct vmm_context*	context;
    struct task*        next;
    //struct ipc
};

//struct task* init_task(void* entry);
struct task* init_task(void* entry, uint32_t pid);
struct cpu_state* schedule(struct cpu_state* cpu);
void init_multitasking(struct multiboot_info* mb_info);
uint8_t get_schedule_flags(void);
void set_schedule_flags(uint8_t new_flags);
void switch_task(void);

#endif
