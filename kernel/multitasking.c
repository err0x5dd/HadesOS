#include "include/console.h"
#include "include/system.h"
#include "include/mm.h"
#include "include/multitasking.h"
#include "include/keyboard.h"
#include "include/syscall.h"
#include "include/multiboot.h"
#include "include/loader.h"

static struct task* first_task = NULL;
static struct task* current_task = NULL;

static uint8_t flags;

static void task_a(void) {
    while(1) {
        kprintf("A");
        switch_task();
    }

    while(1);
}

static void task_b(void) {
    while(1) {
        kprintf("B");
        switch_task();
    }

    while(1);
}

struct task* init_task(void* entry) {
    
    //uint8_t* stack = pmm_alloc();
    uint8_t* stack = vmm_alloc(NULL);
    //uint8_t* user_stack = pmm_alloc();
    uint8_t* user_stack = vmm_alloc(NULL);
    
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
    
    //struct task* task = pmm_alloc();
    struct task* task = vmm_alloc(NULL);
    task->cpu_state = state;
    task->next = first_task;
    first_task = task;
    
    return task;
}

void init_multitasking(struct multiboot_info* mb_info) {
    flags = 0x00;
    //init_task(task_a);
    //init_task(task_b);
    //init_task(task_shell);
    
    if(mb_info->mbs_mods_count == 0) {
        kprintf("Starting test tasks a and b\n");
        init_task(task_a);
        init_task(task_b);
    } else {
        kprintf("Starting first module\n");
        struct multiboot_mods* modules = mb_info->mbs_mods_addr;
        init_mod(modules);
    }
}

void switch_task(void) {
    asm volatile("mov %0, %%eax;"
                 "int $0x30" : : "i" (SYSCALL_SWITCH_TASK));
}

uint8_t get_schedule_flags(void) {
    return flags;
}

void set_schedule_flags(uint8_t new_flags) {
    flags = new_flags;
}

struct cpu_state* schedule(struct cpu_state* cpu) {
    // only switch when scheduling is allowed
    if(!(flags & SCHEDULE_FLAG_DO_NOT_DISTURB)) {
        // save current cpu state
        if(current_task != NULL) {
            current_task->cpu_state = cpu;
        }
        
        if(current_task == NULL) {      // when there is no current task, start with the first task
            current_task = first_task;
        } else {                        // set next task as current task
            current_task = current_task->next;
            if(current_task == NULL) {  // when there is no next task, jump to first task
                current_task = first_task;
            }
        }
        
        cpu = current_task->cpu_state;
    }
    
    return cpu;
}
