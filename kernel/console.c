#include "include/console.h"
#include "include/stdarg.h"
#include "include/serial.h"
#include "include/system.h"

static int curX = 0;
static int curY = 0;
static char color = 0x07;

static char* video = (char*) 0xb8000;

//###############################################################################

static void kscroll(void) {
    int i;
    for(i = 0; i < 2 * 24 * 80; i++) {
        video[i] = video[i + (2 * 80)];
    }
    
    for(; i < 2 * 25 * 80; i++) {
        video[i] = 0x00;
    }
    
    curY--;
}

static void kprintc(char c) {
    if(curX >= 80 || c == '\n') {
        curY++;
        curX = 0;
        write_serial('\r');
        write_serial('\n');
    }
    
    if(c == '\n') {
        return;
    }
    
    if(curY > 24) {
        kscroll();
    }
    
    video[2 * (curY * 80 + curX)] = c;
    video[2 * (curY * 80 + curX) + 1] = color;

    write_serial(c);

    curX++;
}

static void kprints(const char* str) {
    while(*str) {
        kprintc(*str++);
    }
}

static void kprintn(unsigned int nr, int base) {
    const char* digits = "0123456789abcdefghijklmnopqrstuvwxyz";
    char buf[65];
    char* str;
    
    if(base <= 36) {
        str = buf + 64;
        *str = '\0';
        do {
            *--str = digits[ nr % base];
            nr /= base;
        } while(nr);
        kprints(str);
    }
}

//###############################################################################

int kgetPosX(void) {
    return curX;
}

int kgetPosY(void) {
    return curY;
}

void ksetpos(int x, int y) {
    curX = x;
    curY = y;
    uint16_t pos = y * 80 + x;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t) (pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t) ((pos >> 8) & 0xFF));
}

void ksetcolor(char col) {
    color = col;
}

void kclean(void) {
    int i;
    for(i = 0; i < 2 * 25 * 80; i++) {
        video[i] = 0x00;
    }
    
    ksetpos(0, 0);
}

void kprintf(const char* str, ...) {
    va_list ap;
    const char* s;
    unsigned int nr;
    unsigned int c;
    
    va_start(ap, str);
    while(*str) {
        if(*str == '%') {
            str++;
            switch(*str) {
                case 's':
                    s = va_arg(ap, char*);
                    kprints(s);
                    break;
                case 'c':
                    c = va_arg(ap, unsigned int);
                    kprintc(c);
                    break;
                case 'b':
                    nr = va_arg(ap, unsigned int);
                    kprintn(nr, 2);
                    kprintc('b');
                    break;
                case 'd':
                    nr = va_arg(ap, unsigned int);
                    kprintn(nr, 10);
                    break;
                case 'x':
                    nr = va_arg(ap, unsigned int);
                    kprints("0x");
                    kprintn(nr, 16);
                    break;
                case '%':
                    kprintc('%');
                    break;
                case '\0':
                    goto out;
                    break;
                default:
                    kprintc('%');
                    kprintc(*str);
                    break;
            }
        } else {
            kprintc(*str);
        }
        
        str++;
    }
    
    out:
        va_end(ap);
}
