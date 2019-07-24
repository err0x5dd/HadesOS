#include <console.h>
#include <system.h>
#include <mm.h>
#include <multitasking.h>
#include <keyboard.h>
#include <syscall.h>
#include <multiboot.h>
#include <loader.h>
#include <mmap.h>

static struct task* first_task = NULL;
static struct task* current_task = NULL;

static uint8_t flags;
static uint32_t last_pid = 0;

extern struct vmm_context* active_context;

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

static void task_counter(void) {
    for(uint32_t i = 0; i <= 0xffffffff; i++) {
        kprintf("%x ", i);
    }

    while(1);
}

struct task* init_task(void* entry, uint32_t pid) {
    if(pid == NULL)
	pid = ++last_pid;

    struct vmm_context* context;

    if(pid == 1) {
	kprintf("[DEBUG] [multitasking] PID 1 use first context at: %x\n", active_context);
	context = active_context;
    } else {
	kprintf("[DEBUG] [multitasking] Create new context\n");
	context = vmm_create_context();
    }
    
    //uint8_t* stack = pmm_alloc();
    uint8_t* stack = vmm_alloc_at(context, MAP_VMM_STACK);
    //uint8_t* user_stack = pmm_alloc();
    uint8_t* user_stack = vmm_alloc_at(context, MAP_VMM_USERSTACK);
    
    struct cpu_state new_state = {
        .eax = 0,
        .ebx = 0,
        .ecx = 0,
        .edx = 0,
        .esi = 0,
        .edi = 0,
        .ebp = 0,
        .esp = (uint32_t) user_stack + (PAGE_SIZE - 1),
        .eip = (uint32_t) entry,
        
        .cs = 0x18 | 0x03,
        .ss = 0x20 | 0x03,
        
        .eflags = 0x200,
    };
    
    struct cpu_state* state = (void*) (stack + PAGE_SIZE - sizeof(new_state));
    *state = new_state;
    
    //struct task* task = pmm_alloc();
    struct task* task = vmm_alloc_kernel(active_context);
    kprintf("[DEBUG] [multitasking] task is at %x\n", task);
    task->pid = pid;
    task->cpu_state = state;
    task->context = context;
    task->next = first_task;
    first_task = task;
    
    return task;
}

void init_multitasking(struct multiboot_info* mb_info) {
    flags = 0x00;
    //init_task(task_a);
    //init_task(task_b);
    //init_task(task_shell);
    
    //if(mb_info->mbs_mods_count == 0) {
        kprintf("\n[DEBUG] [multitasking] Starting task a\n");
        init_task(task_a, NULL);
        kprintf("\n[DEBUG] [multitasking] Starting task counter\n");
        init_task(task_counter, NULL);
        kprintf("\n[DEBUG] [multitasking] Starting task b\n");
        init_task(task_b, NULL);
    //} else {
    //    kprintf("Starting first module\n");
    //    struct multiboot_mods* modules = mb_info->mbs_mods_addr;
    //    init_mod(modules);
    //}
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
    kprintf("[DEBUG] [scheduler] Starting Scheduler\n");
    // only switch when scheduling is allowed
    if(!(flags & SCHEDULE_FLAG_DO_NOT_DISTURB)) {
        // save current cpu state
        if(current_task != NULL) {
            kprintf("[DEBUG] [scheduler] Save current cpu state\n");
            current_task->cpu_state = cpu;
        }
        
        if(current_task == NULL) {      // when there is no current task, start with the first task
            kprintf("[DEBUG] [scheduler] Start with first task\n");
            current_task = first_task;
            kprintf("1\n");
            kprintf("2: %x\n", current_task->context);
            vmm_activate_context(current_task->context);
            kprintf("3\n");
        } else {                        // set next task as current task
            if(current_task == NULL) {  // when there is no next task, jump to first task
                kprintf("[DEBUG] [scheduler] Continue with first task\n");
                current_task = first_task;
            } else {
                kprintf("[DEBUG] [scheduler] Continue with next task\n");
                current_task = current_task->next;
            }
            vmm_activate_context(current_task->context);
        }
        
        //vmm_activate_context(current_task->context);
        kprintf("[DEBUG] [scheduler] Get new cpu state\n");
        cpu = current_task->cpu_state;
    }
    
    return cpu;
}
