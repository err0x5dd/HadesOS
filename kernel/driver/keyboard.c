#include "../include/keyboard.h"
#include "../include/system.h"
#include "../include/console.h"
#include "../include/mm.h"

// Uncomment for debug output
//#define DEBUG


uint8_t scan1[] = {
// 0x00 - 0x07
0x00, KEY_ESC_PRESSED, KEY_1_PRESSED, KEY_2_PRESSED, KEY_3_PRESSED, KEY_4_PRESSED, KEY_5_PRESSED, KEY_6_PRESSED,
// 0x08 - 0x0f
KEY_7_PRESSED, KEY_8_PRESSED, KEY_9_PRESSED, KEY_0_PRESSED, KEY_MINUS_PRESSED, KEY_EQUAL_PRESSED, KEY_BACKSPACE_PRESSED, KEY_TAB_PRESSED,
// 0x10 - 0x17
KEY_Q_PRESSED, KEY_W_PRESSED, KEY_E_PRESSED, KEY_R_PRESSED, KEY_T_PRESSED, KEY_Y_PRESSED, KEY_U_PRESSED, KEY_I_PRESSED,
// 0x18 - 0x1f
KEY_O_PRESSED, KEY_P_PRESSED, KEY_BRACKETOP_PRESSED, KEY_BRACKETCL_PRESSED, KEY_RETURN_PRESSED, KEY_CTRLL_PRESSED, KEY_A_PRESSED, KEY_S_PRESSED,
// 0x20 - 0x27
KEY_D_PRESSED, KEY_F_PRESSED, KEY_G_PRESSED, KEY_H_PRESSED, KEY_J_PRESSED, KEY_K_PRESSED, KEY_L_PRESSED, KEY_SEMICOLON_PRESSED,
// 0x28 - 0x2f
KEY_QUOTE_PRESSED, KEY_BACKTICK_PRESSED, KEY_SHIFTL_PRESSED, KEY_BACKSLASH_PRESSED, KEY_Z_PRESSED, KEY_X_PRESSED, KEY_C_PRESSED, KEY_V_PRESSED,
// 0x30 - 0x37
KEY_B_PRESSED, KEY_N_PRESSED, KEY_M_PRESSED, KEY_COMMA_PRESSED, KEY_DOT_PRESSED, KEY_SLASH_PRESSED, KEY_SHIFTR_PRESSED, KEY_NUM_MULT_PRESSED,
// 0x38 - 0x3f
KEY_ALT_PRESSED, KEY_SPACE_PRESSED, KEY_CAPS_PRESSED, KEY_F1_PRESSED, KEY_F2_PRESSED, KEY_F3_PRESSED, KEY_F4_PRESSED, KEY_F5_PRESSED,
// 0x40 - 0x47
KEY_F6_PRESSED, KEY_F7_PRESSED, KEY_F8_PRESSED, KEY_F9_PRESSED, KEY_F10_PRESSED, KEY_NUM_NUMLK_PRESSED, KEY_SCRLK_PRESSED, KEY_NUM_7_PRESSED,
// 0x48 - 0x4f
KEY_NUM_8_PRESSED, KEY_NUM_9_PRESSED, KEY_NUM_SUB_PRESSED, KEY_NUM_4_PRESSED, KEY_NUM_5_PRESSED, KEY_NUM_6_PRESSED, KEY_NUM_ADD_PRESSED, KEY_NUM_1_PRESSED,
// 0x50 - 0x57
KEY_NUM_2_PRESSED, KEY_NUM_3_PRESSED, KEY_NUM_0_PRESSED, KEY_NUM_DOT_PRESSED, 0x00, 0x00, 0x00, KEY_F11_PRESSED,
// 0x58 - 0x5f
KEY_F12_PRESSED, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// 0x60 - 0x6f
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// 0x70 - 0x7f
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
// 0x80 - 0x87
0x00, KEY_ESC_RELEASED, KEY_1_RELEASED, KEY_2_RELEASED, KEY_3_RELEASED, KEY_4_RELEASED, KEY_5_RELEASED, KEY_6_RELEASED,
// 0x88 - 0x8f
KEY_7_RELEASED, KEY_8_RELEASED, KEY_9_RELEASED, KEY_0_RELEASED, KEY_MINUS_RELEASED, KEY_EQUAL_RELEASED, KEY_BACKSPACE_RELEASED, KEY_TAB_RELEASED,
// 0x90 - 0x97
KEY_Q_RELEASED, KEY_W_RELEASED, KEY_E_RELEASED, KEY_R_RELEASED, KEY_T_RELEASED, KEY_Y_RELEASED, KEY_U_RELEASED, KEY_I_RELEASED,
// 0x98 - 0x9f
KEY_O_RELEASED, KEY_P_RELEASED, KEY_BRACKETOP_RELEASED, KEY_BRACKETCL_RELEASED, KEY_RETURN_RELEASED, KEY_CTRLL_RELEASED, KEY_A_RELEASED, KEY_S_RELEASED,
// 0xa0 - 0xa7
KEY_D_RELEASED, KEY_F_RELEASED, KEY_G_RELEASED, KEY_H_RELEASED, KEY_J_RELEASED, KEY_K_RELEASED, KEY_L_RELEASED, KEY_SEMICOLON_RELEASED,
// 0xa8 - 0xaf
KEY_QUOTE_RELEASED, KEY_BACKTICK_RELEASED, KEY_SHIFTL_RELEASED, KEY_BACKSLASH_RELEASED, KEY_Z_RELEASED, KEY_X_RELEASED, KEY_C_RELEASED, KEY_V_RELEASED,
// 0xb0 - 0xb7
KEY_B_RELEASED, KEY_N_RELEASED, KEY_M_RELEASED, KEY_COMMA_RELEASED, KEY_DOT_RELEASED, KEY_SLASH_RELEASED, KEY_SHIFTR_RELEASED, KEY_NUM_MULT_RELEASED,
// 0xb8 - 0xbf
KEY_ALT_RELEASED, KEY_SPACE_RELEASED, KEY_CAPS_RELEASED, KEY_F1_RELEASED, KEY_F2_RELEASED, KEY_F3_RELEASED, KEY_F4_RELEASED, KEY_F5_RELEASED,
// 0xc0 - 0xc7
KEY_F6_RELEASED, KEY_F7_RELEASED, KEY_F8_RELEASED, KEY_F9_RELEASED, KEY_F10_RELEASED, KEY_NUM_NUMLK_RELEASED, KEY_SCRLK_RELEASED, KEY_NUM_7_RELEASED,
// 0xc8 - 0xcf
KEY_NUM_8_RELEASED, KEY_NUM_9_RELEASED, KEY_NUM_SUB_RELEASED, KEY_NUM_4_RELEASED, KEY_NUM_5_RELEASED, KEY_NUM_6_RELEASED, KEY_NUM_ADD_RELEASED, KEY_NUM_1_RELEASED,
// 0xd0 - 0xd7
KEY_NUM_2_RELEASED, KEY_NUM_3_RELEASED, KEY_NUM_0_RELEASED, KEY_NUM_DOT_RELEASED, 0x00, 0x00, 0x00, KEY_F11_RELEASED,
// 0xd8 - 0xdf
KEY_F12_RELEASED, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

};

uint8_t* kbuff;
uint8_t* kbuff_head;
uint8_t* kbuff_tail;

// 0x01 => e0
// 0x02 => e1
// 0x04 => shift
// 0x08 => caps
// 0x10 => ctrl
// 0x20 => alt
// 0x40 => altgr
uint8_t flags = 0x00;

static void send_keyboard_command(uint8_t command) {
    uint8_t ret;
    uint8_t exit = 1;
    
    do {
        while(inb(0x64) & 0x02);
        outb(0x60, command);
        ret = inb(0x60);
        if(ret == 0xfa) {
            exit = 0;
        }
    } while(exit != 0);
}

void kbd_init(void) {
    
    //kbuff = (uint8_t*) pmm_alloc();
    kprintf("[DEBUG] [kbd] Request kernel page\n");
    kbuff = (uint8_t*) vmm_alloc_kernel(NULL);
    kprintf("[DEBUG] [kbd] Got kernel page %x\n", kbuff);
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

    // activate scanning
    send_keyboard_command(0xf4);
    
    // clear output buffer
    while(inb(0x64) & 0x01) {
        inb(0x60);
    }
}

uint8_t kbd_translate(uint8_t scan) {
    // I'm assuming scancode 1 here because that is the default mode in my testing
    // and I wasn't able to change it yet :-/
    //
    // TODO translate scancode to keycode

    // check if special byte was sent
    if(scan == 0xe0) {
        // TODO set e0 flag
    } else if(scan == 0xe1) {
        // TODO set e1 flag
    } else {
        if(flags & 0x01) {

        } else if(flags & 0x02) {

        } else {
            return scan1[scan];
        }
    }
    

    return 0x00;
}

void kbd_isr(void) {
    uint8_t keycode = kbd_translate(inb(0x60));
    #ifdef DEBUG
    kprintf("Received keycode: %x\n", keycode);
    #endif
    if(keycode != 0x00) {
        if(kbuff_tail < kbuff + PAGE_SIZE) {
            #ifdef DEBUG
            kprintf("max: %x\n", kbuff + PAGE_SIZE);
            kprintf("cur: %x\n", kbuff_tail);
            #endif
            *kbuff_tail = keycode;
            kbuff_tail++;
        } else {
            #ifdef DEBUG
            kprintf("Not enough keybuffer!\n");
            #endif
        }
    }
}

char keycode_to_ascii(uint8_t key) {
    switch(key) {
        case KEY_A_RELEASED: return 'a';
        case KEY_B_RELEASED: return 'b';
        case KEY_C_RELEASED: return 'c';
        case KEY_D_RELEASED: return 'd';
        case KEY_E_RELEASED: return 'e';
        case KEY_F_RELEASED: return 'f';
        case KEY_G_RELEASED: return 'g';
        case KEY_H_RELEASED: return 'h';
        case KEY_I_RELEASED: return 'i';
        case KEY_J_RELEASED: return 'j';
        case KEY_K_RELEASED: return 'k';
        case KEY_L_RELEASED: return 'l';
        case KEY_M_RELEASED: return 'm';
        case KEY_N_RELEASED: return 'n';
        case KEY_O_RELEASED: return 'o';
        case KEY_P_RELEASED: return 'p';
        case KEY_Q_RELEASED: return 'q';
        case KEY_R_RELEASED: return 'r';
        case KEY_S_RELEASED: return 's';
        case KEY_T_RELEASED: return 't';
        case KEY_U_RELEASED: return 'u';
        case KEY_V_RELEASED: return 'v';
        case KEY_W_RELEASED: return 'w';
        case KEY_X_RELEASED: return 'x';
        case KEY_Y_RELEASED: return 'y';
        case KEY_Z_RELEASED: return 'z';
        case KEY_0_RELEASED: return '0';
        case KEY_1_RELEASED: return '1';
        case KEY_2_RELEASED: return '2';
        case KEY_3_RELEASED: return '3';
        case KEY_4_RELEASED: return '4';
        case KEY_5_RELEASED: return '5';
        case KEY_6_RELEASED: return '6';
        case KEY_7_RELEASED: return '7';
        case KEY_8_RELEASED: return '8';
        case KEY_9_RELEASED: return '9';
        case KEY_SPACE_RELEASED: return ' ';
        case KEY_RETURN_RELEASED: return '\n';
    };
    return 0x00;
}

char getc() {
    while(kbuff_head == kbuff_tail);
    uint8_t key = *kbuff_head;
    if(key != 0x00) {
        kbuff_head++;
    }
    while(keycode_to_ascii(key) == 0x00) {
        key = *kbuff_head;
        if(key != 0x00) {
            kbuff_head++;
        }
    }
    char c = keycode_to_ascii(key);
    #ifdef DEBUG
    kprintf("Ascii %c returned\n", c);
    #endif
    return c;
}

uint8_t get_key() {
    while(kbuff_head == kbuff_tail);
    uint8_t key = *kbuff_head;
    if(key != 0x00) {
        kbuff_head++;
    }
    return key;
}
