#include "include/interrupt.h"
#include "include/system.h"
#include "include/console.h"
#include "include/multitasking.h"
#include "include/keyboard.h"
#include "include/syscall.h"

extern void isr_0(void);
extern void isr_1(void);
extern void isr_2(void);
extern void isr_3(void);
extern void isr_4(void);
extern void isr_5(void);
extern void isr_6(void);
extern void isr_7(void);
extern void isr_error_8(void);
extern void isr_9(void);
extern void isr_error_10(void);
extern void isr_error_11(void);
extern void isr_error_12(void);
extern void isr_error_13(void);
extern void isr_error_14(void);
extern void isr_15(void);
extern void isr_16(void);
extern void isr_error_17(void);
extern void isr_18(void);

extern void isr_32(void);
extern void isr_33(void);
extern void isr_34(void);
extern void isr_35(void);
extern void isr_36(void);
extern void isr_37(void);
extern void isr_38(void);
extern void isr_39(void);
extern void isr_40(void);
extern void isr_41(void);
extern void isr_42(void);
extern void isr_43(void);
extern void isr_44(void);
extern void isr_45(void);
extern void isr_46(void);
extern void isr_47(void);

extern void isr_48(void);

#define GDT_FLAG_DATASEG    0x02
#define GDT_FLAG_CODESEG    0x0a
#define GDT_FLAG_TSS        0x09

#define GDT_FLAG_SEGMENT    0x10
#define GDT_FLAG_RING0      0x00
#define GDT_FLAG_RING3      0x60
#define GDT_FLAG_PRESENT    0x80

#define GDT_FLAG_4K_GRAN    0x800
#define GDT_FLAG_32_BIT     0x400

#define GDT_ENTRIES         6
static uint64_t gdt[GDT_ENTRIES];
static uint32_t tss[32] = {0, 0, 0x10};

#define IDT_ENTRIES 256
static long long unsigned int idt[IDT_ENTRIES];

static void set_gdt_entry(int i, unsigned int base, unsigned int limit, int flags) {
    gdt[i] = limit & 0xffffLL;
    gdt[i] |= (base & 0xffffffLL) << 16;
    gdt[i] |= (flags & 0xffLL) << 40;
    gdt[i] |= ((limit >> 16) & 0xfLL) << 48;
    gdt[i] |= ((flags >> 8 )& 0xffLL) << 52;
    gdt[i] |= ((base >> 24) & 0xffLL) << 56;
}

static void load_gdt(void) {
    struct {
        uint16_t limit;
        void* pointer;
    } __attribute__((packed)) gdtp = {
        .limit = GDT_ENTRIES * 8 - 1,
        .pointer = gdt,
    };
    
    asm volatile("lgdt %0" : : "m" (gdtp));
    
    asm volatile(
        "mov $0x10, %ax;"
        "mov %ax, %ds;"
        "mov %ax, %es;"
        "mov %ax, %ss;"
        "ljmp $0x8, $.1;"
        ".1:"
    );
    
    asm volatile("ltr %%ax" : : "a" (5 << 3));
}

void init_gdt(void) {
    set_gdt_entry(0, 0, 0, 0);
    set_gdt_entry(1, 0, 0xfffff, GDT_FLAG_SEGMENT | GDT_FLAG_32_BIT |
        GDT_FLAG_CODESEG | GDT_FLAG_4K_GRAN | GDT_FLAG_PRESENT);
    set_gdt_entry(2, 0, 0xfffff, GDT_FLAG_SEGMENT | GDT_FLAG_32_BIT |
        GDT_FLAG_DATASEG | GDT_FLAG_4K_GRAN | GDT_FLAG_PRESENT);
    set_gdt_entry(3, 0, 0xfffff, GDT_FLAG_SEGMENT | GDT_FLAG_32_BIT |
        GDT_FLAG_CODESEG | GDT_FLAG_4K_GRAN | GDT_FLAG_PRESENT | GDT_FLAG_RING3);
    set_gdt_entry(4, 0, 0xfffff, GDT_FLAG_SEGMENT | GDT_FLAG_32_BIT |
        GDT_FLAG_DATASEG | GDT_FLAG_4K_GRAN | GDT_FLAG_PRESENT | GDT_FLAG_RING3);
    set_gdt_entry(5, (uint32_t) tss, sizeof(tss), GDT_FLAG_TSS |
        GDT_FLAG_PRESENT | GDT_FLAG_RING3);
    load_gdt();
}

#define IDT_FLAG_INTERRUPT_GATE 0xe
#define IDT_FLAG_PRESENT 0x80
#define IDT_FLAG_RING0 0x00
#define IDT_FLAG_RING3 0x60





static void set_idt_entry(int i, void (*f)(), unsigned int selector, int flags) {
    unsigned long int handler = (unsigned long int) f;
    idt[i] = handler & 0xffffLL;
    idt[i] |= (selector & 0xffffLL) << 16;
    idt[i] |= (flags & 0xffLL) << 40;
    idt[i] |= ((handler >> 16) & 0xffffLL) << 48;
}

static void init_pic(void) {
    outb(0x20, 0x11);
    outb(0x21, 0x20);
    outb(0x21, 0x04);
    outb(0x21, 0x01);
    
    outb(0xa0, 0x11);
    outb(0xa1, 0x28);
    outb(0xa1, 0x02);
    outb(0xa1, 0x01);
    
    outb(0x20, 0x0);
    outb(0xa0, 0x0);
}

static void load_idt(void) {
    struct {
        unsigned short int limit;
        void* pointer;
    } __attribute__((packed)) idtp = {
        .limit = IDT_ENTRIES * 8 - 1,
        .pointer = idt,
    };
    
    init_pic();
    
    asm volatile("lidt %0" : : "m" (idtp));
}

void init_idt(void) {
    set_idt_entry(0, isr_0, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(1, isr_1, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(2, isr_2, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(3, isr_3, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(4, isr_4, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(5, isr_5, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(6, isr_6, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(7, isr_7, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(8, isr_error_8, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(9, isr_9, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(10, isr_error_10, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(11, isr_error_11, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(12, isr_error_12, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(13, isr_error_13, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(14, isr_error_14, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(15, isr_15, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(16, isr_16, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(17, isr_error_17, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(18, isr_18, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    
    set_idt_entry(32, isr_32, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(33, isr_33, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(34, isr_34, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(35, isr_35, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(36, isr_36, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(37, isr_37, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(38, isr_38, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(39, isr_39, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(40, isr_40, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(41, isr_41, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(42, isr_42, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(43, isr_43, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(44, isr_44, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(45, isr_45, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(46, isr_46, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_idt_entry(47, isr_47, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    
    // Syscall should be in ring 3
    set_idt_entry(48, isr_48, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING3 | IDT_FLAG_PRESENT);
    
    load_idt();
}

struct cpu_state* handler(struct cpu_state* cpu) {
    
    struct cpu_state* new_cpu = cpu;
    
    if(cpu->interrupt <= 0x1f) { // Exceptions
        ksetcolor(0x04);
        kprintf("Exception %x, Kernel angehalten!\n", cpu->interrupt);
        
        kprintf("eax: %x\n", cpu->eax);
        kprintf("ebx: %x\n", cpu->ebx);
        kprintf("ecx: %x\n", cpu->ecx);
        kprintf("edx: %x\n", cpu->edx);
        
        kprintf("cs: %x\n", cpu->cs);
        kprintf("ss: %x\n", cpu->ss);
        
        kprintf("esi: %x\n", cpu->esi);
        kprintf("edi: %x\n", cpu->edi);
        
        kprintf("ebp: %x\n", cpu->ebp);
        kprintf("esp: %x\n", cpu->esp);
        kprintf("eip: %x\n", cpu->eip);
        
        kprintf("error: %x\n", cpu->error);

        while(1) {
            asm volatile("cli; hlt");
        }
    } else if(cpu->interrupt >= 0x20 && cpu->interrupt <= 0x2f) { // IRQs
        
        if(cpu->interrupt == 0x20) {
            
            new_cpu = schedule(cpu);
            tss[1] = (uint32_t) (new_cpu + 1);
            
        } else if(cpu->interrupt == 0x21) {
            kbd_isr();
        } else {
            kprintf("IRQ %x\n", cpu->interrupt);
        }
        
        
        // End of Interrupt an PIC senden
        if(cpu->interrupt >= 0x28) {
            outb(0xa0, 0x20);
        }
        outb(0x20, 0x20);
    } else {
        if(cpu->interrupt == 0x30) {
            new_cpu = syscall(cpu);
            tss[1] = (uint32_t) (new_cpu + 1);
        }
    }
    
    return new_cpu;
}

