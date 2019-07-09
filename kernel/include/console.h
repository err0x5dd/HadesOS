#include "stdint.h"

#ifndef CONSOLE_H
#define CONSOLE_H

#define COLOR_FG_BLACK       0x00
#define COLOR_FG_BLUE        0x01
#define COLOR_FG_GREEN       0x02
#define COLOR_FG_CYAN        0x03
#define COLOR_FG_RED         0x04
#define COLOR_FG_PURPLE      0x05
#define COLOR_FG_BROWN       0x06
#define COLOR_FG_GRAY        0x07
#define COLOR_FG_DARKGRAY    0x08
#define COLOR_FG_LIGHTBLUE   0x09
#define COLOR_FG_LIGHTGREEN  0x0A
#define COLOR_FG_LIGHTCYAN   0x0B
#define COLOR_FG_LIGHTRED    0x0C
#define COLOR_FG_LIGHTPURPLE 0x0D
#define COLOR_FG_YELLOW      0x0E
#define COLOR_FG_WHITE       0x0F

#define COLOR_BG_BLACK       0x00
#define COLOR_BG_BLUE        0x10
#define COLOR_BG_GREEN       0x20
#define COLOR_BG_CYAN        0x30
#define COLOR_BG_RED         0x40
#define COLOR_BG_PURPLE      0x50
#define COLOR_BG_BROWN       0x60
#define COLOR_BG_GRAY        0x70
#define COLOR_BG_DARKGRAY    0x80
#define COLOR_BG_LIGHTBLUE   0x90
#define COLOR_BG_LIGHTGREEN  0xA0
#define COLOR_BG_LIGHTCYAN   0xB0
#define COLOR_BG_LIGHTRED    0xC0
#define COLOR_BG_LIGHTPURPLE 0xD0
#define COLOR_BG_YELLOW      0xE0
#define COLOR_BG_WHITE       0xF0

void kclean(void);

void ksetpos(int x, int y);
void ksetcolor(uint8_t color);
uint8_t kgetcolor(void);

void kprintf(const char* str, ...);

#endif

