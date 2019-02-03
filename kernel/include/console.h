#ifndef CONSOLE_H
#define CONSOLE_H

void kclean(void);

void ksetpos(int x, int y);
void ksetcolor(char color);

void kprintf(const char* str, ...);

#endif

