.macro isr nr
.global isr_\nr
isr_\nr:
    pushl $0
    pushl $\nr
    jmp pre_handler
.endm

.macro isr_error nr
.global isr_error_\nr
isr_error_\nr:
    pushl $\nr
    jmp pre_handler
.endm

// Exceptions
isr 0
isr 1
isr 2
isr 3
isr 4
isr 5
isr 6
isr 7
isr_error 8
isr 9
isr_error 10
isr_error 11
isr_error 12
isr_error 13
isr_error 14
isr 15
isr 16
isr_error 17
isr 18

// IRQs
isr 32
isr 33
isr 34
isr 35
isr 36
isr 37
isr 38
isr 39
isr 40
isr 41
isr 42
isr 43
isr 44
isr 45
isr 46
isr 47

// Syscall
isr 48
isr 49

.extern handler
pre_handler:
    // CPU-Zustand sichern
    push %ebp
    push %edi
    push %esi
    push %edx
    push %ecx
    push %ebx
    push %eax
    
    // ISR starten
    push %esp
    call handler
    mov %eax, %esp
    
    // CPU-Zusand wiederherstellen
    pop %eax
    pop %ebx
    pop %ecx
    pop %edx
    pop %esi
    pop %edi
    pop %ebp
    
    // Fehlercode und Interruptnummer vom Stack entfernen
    add $8, %esp
    
    iret

