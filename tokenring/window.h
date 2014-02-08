#ifndef WINDOW_H
#define WINDOW_H

#include "defs.h"

struct tr_window;
typedef struct tr_window tr_window;

tr_window* tr_window_create(rectangle);

void tr_window_putch(tr_window*, char);
void tr_window_puts(tr_window*, char*);
/* Nie daje nowej linii po */
void tr_window_print(tr_window*, char*);

void tr_window_delch(tr_window*);
void tr_window_clear(tr_window*);

/* void window_cputch(window*, char, color fg, color bg); */

/*
window_tputch(window*, char)
window_tcputch(window*, char)
 */

void tr_window_delete(tr_window*);

#endif
