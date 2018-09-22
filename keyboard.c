#include "include/keyboard.h"
#include "include/system.h"
#include "include/console.h"
#include "include/mm.h"

#define KEY_BUFFER_SIZE 4096
uint8_t* key_buffer;
uint8_t key_buffer_first = 0;
uint8_t key_buffer_last = 0;

static void send_keyboard_command(uint8_t command) {
    while(inb(0x64) & 0x02);
    outb(0x60, command);
}

void init_keyboard(void) {
    
    key_buffer = (uint8_t*) pmm_alloc();
    memset(key_buffer, 0, sizeof(KEY_BUFFER_SIZE));
    for(int i = 0; i < 4096; i++) {
        kprintf("%x ", key_buffer[i]);
    }
    kprintf("\n");
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
    
    // Prüfen, ob Speicherende erreicht wurde
    if(key_buffer_last == KEY_BUFFER_SIZE) {
        // Prüfen, ob Speicherbeginn leer ist
        if(key_buffer_first > 0) {
            // TODO Verschiebe alle Buffer-Einträge an den Anfang
            key_buffer_first = 0; // Setze Anfangsindex auf den Speicherbeginn
        } else {
            return;
        }
    }
    
    // Füge Keycode dem Buffer hinzu
    key_buffer[key_buffer_last] = keycode;
    key_buffer_last++;
    kprintf("key_buffer_first: %d\n", key_buffer_first);
    kprintf("key_buffer_flast: %d\n", key_buffer_last);
    kprintf("Key Interrupt %x\n", keycode);
}

char getc() {
    char retc;
    
    while(key_buffer_last == 0);
    
    retc = key_buffer[key_buffer_first];
    key_buffer_first++;
    
    kprintf("Key getc %x\n", retc);
    
    return retc;
}

