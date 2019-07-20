#include <console.h>
#include <stdarg.h>
#include <serial.h>
#include <system.h>

static int curX = 0;
static int curY = 0;
static uint8_t color = 0x07;

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
        //write_serial('\n');
    }
    
    if(curY > 24) {
        kscroll();
    }
    
    if(c == '\n') {
        return;
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

void ksetcolor(uint8_t col) {
    color = col;
}

uint8_t kgetcolor(void) {
    return color;
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
    
    uint8_t flags = 0x00;
    #define CON_FLAG_NOPREFIX 0x01
    #define CON_FLAG_NOPRINT 0x02
    
    va_start(ap, str);
    while(*str) {
        if(*str == '%' || (flags & CON_FLAG_NOPRINT)) {
            if(*str == '%')
                str++;
            switch(*str) {
                case '0':
                    if(*(str+1) == 'b' || *(str+1) == 'x') {
                        flags |= CON_FLAG_NOPREFIX | CON_FLAG_NOPRINT;
                        //kprintc('?');
                        //kprints("TEST\n");
                        //kprintn(flags, 2);
                    } else {
                        kprintc('%');
                        kprintc('0');
                    }
                    break;
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
                    if(!(flags & CON_FLAG_NOPRINT)) {
                        kprintc('b');
                    } else {
                        flags &= ~(CON_FLAG_NOPREFIX | CON_FLAG_NOPRINT);
                        //kprintc('#');
                    }
                    break;
                case 'd':
                    nr = va_arg(ap, unsigned int);
                    kprintn(nr, 10);
                    break;
                case 'x':
                    nr = va_arg(ap, unsigned int);
                    //kprints("\nTEST: ");
                    //kprintn(flags, 2);
                    //kprints("\n");
                    if(!(flags & CON_FLAG_NOPREFIX)) {
                        kprints("0x");
                    } else {
                        flags &= ~(CON_FLAG_NOPREFIX | CON_FLAG_NOPRINT);
                        //kprints("2TEST2\n");
                        //kprintn(flags, 2);
                        //kprintc('#');
                    }
                    kprintn(nr, 16);
                    break;
                case '%':
                    kprintc('%');
                    break;
                case '\0':
                    goto out;
                    break;
                default:
                    if(flags & CON_FLAG_NOPRINT) {
                        //kprints("\n3TEST3 ");
                        //kprintn(flags, 2);
                        //kprints("\n");
                    }
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
