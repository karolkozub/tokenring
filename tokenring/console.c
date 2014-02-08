#include <dos.h>

#include "console.h"

#define CONSOLE_WIDTH  80
#define CONSOLE_HEIGHT 25
#define BASE_ADDR      0xB800
#define CHAR_OFFSET(x,y)        (2*((y)*CONSOLE_WIDTH+(x)))
#define COLOR_OFFSET(x,y)       (CHAR_OFFSET(x,y)+1)
#define CONSOLE_PUTCHAR(ch,x,y) pokeb(BASE_ADDR, CHAR_OFFSET(x,y),ch)
#define CONSOLE_SETCOLOR(c,x,y) pokeb(BASE_ADDR,COLOR_OFFSET(x,y),c)
#define CONSOLE_GETCHAR(x,y)    peekb(BASE_ADDR, CHAR_OFFSET(x,y))
#define CONSOLE_GETCOLOR(x,y)   peekb(BASE_ADDR,COLOR_OFFSET(x,y))

void console_putch(int x, int y, char ch)
{
    CONSOLE_PUTCHAR(ch, x, y);
}

void console_copych(int x_from, int y_from, int x_to, int y_to)
{
    CONSOLE_PUTCHAR(CONSOLE_GETCHAR(x_from, y_from), x_to, y_to);
}

void console_clear()
{
    clrscr();
}
