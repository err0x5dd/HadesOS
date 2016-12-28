#include "include/console.h"
#include "include/stdarg.h"

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
}

static void kprintc(char c) {
    if(curX >= 80 || c == '\n') {
        curY++;
        curX = 0;
    }
    
    if(c == '\n') {
        return;
    }
    
    if(curY > 24) {
        kscroll();
    }
    
    video[2 * (curY * 80 + curX)] = c;
    video[2 * (curY * 80 + curX) + 1] = color;
    
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

void ksetpos(int x, int y) {
    curX = x;
    curY = y;
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
    
    va_start(ap, str);
    while(*str) {
        if(*str == '%') {
            str++;
            switch(*str) {
                case 's':
                    s = va_arg(ap, char*);
                    kprints(s);
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
