#include "include/console.h"
#include "include/system.h"
#include "include/mm.h"
#include "include/multitasking.h"
#include "include/keyboard.h"

static struct task* first_task = NULL;
static struct task* current_task = NULL;

static uint8_t flags;

static void task_a(void) {
    while(1) {
        kprintf("A");
    }

    while(1);
}

static void task_b(void) {
    while(1) {
        kprintf("B");
    }

    while(1);
}

static void task_c(void) {
    while(1) {
        kprintf("C");
    }

    while(1);
}


struct task* init_task(void* entry) {
    
    uint8_t* stack = pmm_alloc();
    uint8_t* user_stack = pmm_alloc();
    
    struct cpu_state new_state = {
        .eax = 0,
        .ebx = 0,
        .ecx = 0,
        .edx = 0,
        .esi = 0,
        .edi = 0,
        .ebp = 0,
        .esp = (uint32_t) user_stack + PAGE_SIZE,
        .eip = (uint32_t) entry,
        
        .cs = 0x18 | 0x03,
        .ss = 0x20 | 0x03,
        
        .eflags = 0x200,
    };
    
    struct cpu_state* state = (void*) (stack + PAGE_SIZE - sizeof(new_state));
    *state = new_state;
    
    struct task* task = pmm_alloc();
    task->cpu_state = state;
    task->next = first_task;
    first_task = task;
    
    return task;
}

void init_multitasking(void) {
    flags = 0x00;
    init_task(task_a);
    init_task(task_b);
    init_task(task_c);
    //init_task(shell);
}

uint8_t get_schedule_flags(void) {
    return flags;
}

void set_schedule_flags(uint8_t new_flags) {
    flags = new_flags;
}


struct cpu_state* schedule(struct cpu_state* cpu) {
    if(!(flags & SCHEDULE_FLAG_DO_NOT_DISTURB)) {
        if(current_task != NULL) {
            current_task->cpu_state = cpu;
        }
        
        if(current_task == NULL) {
            current_task = first_task;
        } else {
            current_task = current_task->next;
            if(current_task == NULL) {
                current_task = first_task;
            }
        }
        
        cpu = current_task->cpu_state;
    }
    
    return cpu;
}
