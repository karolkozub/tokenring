#ifndef RS_CONSOLE_H
#define RS_CONSOLE_H

void console_putch(int x, int y, char);
/* void console_cputch(int x, int y, char, color); */

void console_copych(int x_from, int y_from, int x_to, int y_to);
/* ccopych */

void console_clear();

#endif
