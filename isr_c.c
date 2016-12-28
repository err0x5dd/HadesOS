#include "include/system.h"
#include "include/console.h"

void handler(struct cpu_state* cpu) {
    
    kprintf("Interrupt aufgetreten\n");
    long i;
    for(i = 0; i < 1000000000; i++);
    
    if(cpu->interrupt <= 0x1f) { // Exceptions
        kprintf("Exception %x, Kernel angehalten!\n", cpu->interrupt);
        
        while(1) {
            asm volatile("cli; hlt");
        }
    } else if(cpu->interrupt >= 0x20 && cpu->interrupt <= 0x2f) { // IRQs
        
        kprintf("IRQ %x", cpu->interrupt);
        
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
        }
    }
}
