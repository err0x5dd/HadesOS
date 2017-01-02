#include "include/system.h"
#include "include/console.h"
#include "include/multitasking.h"

struct cpu_state* handler(struct cpu_state* cpu) {
    
    struct cpu_state* new_cpu = cpu;
    
    if(cpu->interrupt <= 0x1f) { // Exceptions
        kprintf("Exception %x, Kernel angehalten!\n", cpu->interrupt);
        
        while(1) {
            asm volatile("cli; hlt");
        }
    } else if(cpu->interrupt >= 0x20 && cpu->interrupt <= 0x2f) { // IRQs
        
        if(cpu->interrupt == 0x20) {
            new_cpu = schedule(cpu);
        } else {
            kprintf("IRQ %x\n", cpu->interrupt);
        }
        
        
        // End of Interrupt an PIC senden
        if(cpu->interrupt >= 0x28) {
            outb(0xa0, 0x20);
        }
        outb(0x20, 0x20);
    } else { // Syscalls
        
        kprintf("Syscall %x", cpu->interrupt);
        
        if(cpu->interrupt == 0x30) {
            char* str = (char*) cpu->eax;
            kprintf("%s", str);
        } else if(cpu->interrupt == 0x31) {
            kprintf("#Test#\n");
        }
    }
    
    return new_cpu;
}
