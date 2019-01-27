#include "include/keyboard.h"
#include "include/system.h"
#include "include/console.h"
#include "include/mm.h"

// Uncomment for debug output
#define DEBUG

uint8_t* kbuff;
uint8_t* kbuff_head;
uint8_t* kbuff_tail;

static void send_keyboard_command(uint8_t command) {
    while(inb(0x64) & 0x02);
    outb(0x60, command);
}

void init_keyboard(void) {
    
    kbuff = (uint8_t*) pmm_alloc();
    memset(kbuff, 0x00, PAGE_SIZE);

    kbuff_head = kbuff;
    kbuff_tail = kbuff;

    #ifdef DEBUG
    kprintf("kbuff: %x\n", kbuff);
    kprintf("kbufff_head: %x\n", kbuff_head);
    kprintf("kbuff_tail: %x\n", kbuff_tail);

    kprintf("Printing allocated keyboard buffer!\n");
    for(int i = 0; i < PAGE_SIZE; i++) {
        kprintf("%x: %x\n", &kbuff[i], kbuff[i]);
    }
    #endif

    while(inb(0x64) & 0x01) {
        inb(0x60);
    }
    
    send_keyboard_command(0xf4);
}

void keyboard_isr(struct cpu_state* cpu) {

    uint8_t scancode;
    uint8_t keycode = 0;
    int break_code = 0;
    
    static int e0 = 0;
    static int e1 = 0;
    static uint16_t e1_ext = 0;
    
    scancode = inb(0x60);
    
    if((scancode & 0x80) &&
       (e1 || (scancode != 0xe1)) &&
       (e0 || (scancode != 0xe0))) {
        break_code = 1;
        scancode &= ~0x80;
    }
    
    if(e0) {
        if((scancode == 0x2a) || (scancode == 0x36)) {
            e0 = 0;
            return;
        }
        
        // TODO Add a funktion to interpret the scancode
        e0 = 0;
    } else if(e1 == 2) {
        e1_ext |= ((uint16_t) scancode << 8);
        // TODO Add a funktion to interpret the scancode
        e1 = 0;
    } else if(e1 == 1) {
        e1_ext = scancode;
        e1++;
    } else if(scancode == 0xe0) {
        e0 = 1;
    } else if(scancode == 0xe1) {
        e1 = 1;
    } else {
        // TODO Add a funktion to interpret the scancode
    }
    
    keycode = scancode;
    
    // Prüfen, ob Speicherende erreicht wurde
    if(kbuff_tail == kbuff + (PAGE_SIZE - 1)) {
        // Prüfen, ob Speicherbeginn leer ist
        if(kbuff_head > kbuff) {
            // TODO Verschiebe alle Buffer-Einträge an den Anfang
            int n = kbuff_head - kbuff;
            for(int i = n; i < PAGE_SIZE; i++) {
                *kbuff[i - n] = *kbuff[i]
            }
            kbuff_head = kbuff;
            kbuff_tail = kbuff_tail - n;
            
        } else {
            return;
        }
    }
    /*
    // Füge Keycode dem Buffer hinzu
    key_buffer[key_buffer_last] = keycode;
    key_buffer_last++;
    kprintf("key_buffer_first: %d\n", key_buffer_first);
    kprintf("key_buffer_flast: %d\n", key_buffer_last);
    kprintf("Key Interrupt %x\n", keycode);
    */
}

char getc() {
/*
    char retc;
    
    while(key_buffer_last == 0);
    
    retc = key_buffer[key_buffer_first];
    key_buffer_first++;
    
    kprintf("Key getc %x\n", retc);
    
    return retc;
*/
}

