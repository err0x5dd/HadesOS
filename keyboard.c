#include "include/keyboard.h"
#include "include/system.h"
#include "include/console.h"

static void send_keyboard_command(uint8_t command) {
    while(inb(0x64) & 0x02);
    outb(0x60, command);
}

void init_keyboard(void) {
    keyboard_buffer_entrys = 0;
    
    while(inb(0x64) & 0x01) {
        inb(0x60);
    }
    
    send_keyboard_command(0xF4);
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
       (e1 || (scancode != 0xE1)) &&
       (e0 || (scancode != 0xE0))) {
        break_code = 1;
        scancode &= ~0x80;
    }
    
    if(e0) {
        if((scancode == 0x2A) || (scancode == 0x36)) {
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
    } else if(scancode == 0xE0) {
        e0 = 1;
    } else if(scancode == 0xE1) {
        e1 = 1;
    } else {
        // TODO Add a funktion to interpret the scancode
    }
    
    keycode = scancode;
    
    if(keyboard_buffer_entrys < 256) {
        keyboard_buffer[keyboard_buffer_entrys] = keycode;
    }
}
