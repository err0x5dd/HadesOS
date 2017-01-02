.section multiboot
#define MB_MAGIC 0x1badb002
#define MB_FLAGS 0x0
#define MB_CHECKSUM -(MB_MAGIC + MB_FLAGS)

// Multiboot-Header
.align 4
.int    MB_MAGIC
.int    MB_FLAGS
.int    MB_CHECKSUM

.section .text

// void init(void) aus init.c einbinden
.extern init

// startpunkt des kernels
.global _start
_start:
    // Stack an die Startposition des Stack setzen
    mov $kernel_stack, %esp
    
    call init
    
    jmp .

// 8 kiB fuer den Stack reservieren
// Note: Stack waechst nach unten
.section .bss
.space 8192
kernel_stack:

