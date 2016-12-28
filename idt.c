#include "include/idt.h"
#include "include/system.h"

#define IDT_FLAG_INTERRUPT_GATE 0xe
#define IDT_FLAG_PRESENT 0x80
#define IDT_FLAG_RING0 0x00
#define IDT_FLAG_RING3 0x60

#define IDT_ENTRIES 5

static long long unsigned int idt[IDT_ENTRIES];

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

static void set_entry(int i, void (*f)(), unsigned int selector, int flags) {
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
    set_entry(0, isr_0, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(1, isr_1, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(2, isr_2, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(3, isr_3, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(4, isr_4, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(5, isr_5, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(6, isr_6, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(7, isr_7, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(8, isr_error_8, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(9, isr_9, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(10, isr_error_10, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(11, isr_error_11, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(12, isr_error_12, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(13, isr_error_13, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(14, isr_error_14, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(15, isr_15, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(16, isr_16, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(17, isr_error_17, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(18, isr_18, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    
    set_entry(32, isr_32, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(33, isr_33, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(34, isr_34, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(35, isr_35, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(36, isr_36, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(37, isr_37, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(38, isr_38, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(39, isr_39, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(40, isr_40, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(41, isr_41, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(42, isr_42, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(43, isr_43, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(44, isr_44, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(45, isr_45, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(46, isr_46, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    set_entry(47, isr_47, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    
    set_entry(48, isr_48, 0x8, IDT_FLAG_INTERRUPT_GATE | IDT_FLAG_RING0 | IDT_FLAG_PRESENT);
    
    load_idt();
}
