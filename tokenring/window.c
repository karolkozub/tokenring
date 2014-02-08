#include <stdlib.h>

#include "window.h"
#include "console.h"

#define FIRST_COL(rect)  ((rect).x)
#define LAST_COL(rect)   ((rect).x + (rect).width - 1)
#define FIRST_LINE(rect) ((rect).y)
#define LAST_LINE(rect)  ((rect).y + (rect).height -1)

struct tr_window
{
    rectangle rect;
    point cursor;
};

tr_window* tr_window_create(rectangle rect)
{
    tr_window* wnd = (tr_window*)malloc(sizeof(tr_window));

    wnd->rect = rect;
    wnd->cursor.x = rect.x;
    wnd->cursor.y = rect.y;

    return wnd;
}

/* Przesuwa cala zawartosc o jeden wiersz do gory tworzac na koncu czysta linie */
static void tr_window_scroll(tr_window* wnd)
{
    int line, x;
    
    for(line = FIRST_LINE(wnd->rect); line < LAST_LINE(wnd->rect); ++line)
    {
	for(x = FIRST_COL(wnd->rect); x <= LAST_COL(wnd->rect); ++x)
	    console_copych(x, line+1, x, line);
    }
    
    for(x = FIRST_COL(wnd->rect); x <= LAST_COL(wnd->rect); ++x)
	console_putch(x, LAST_LINE(wnd->rect), ' ');	
}

/* Ustawia kursor na poprawnej pozycji (wewnatrz prostakatu okna) 
   Uzywane przed wstawieniem znaku
 */
static void tr_window_validate_cursor(tr_window* wnd)
{
    /* wyjscie poza koniec linii */
    if(wnd->cursor.x > LAST_COL(wnd->rect))
    {
	wnd->cursor.x = FIRST_COL(wnd->rect);
	++wnd->cursor.y;
    }

    /* wyjscie poza ostatnia linie */
    if(wnd->cursor.y > LAST_LINE(wnd->rect))
    {
	tr_window_scroll(wnd);
	--wnd->cursor.y;
    }
}

void tr_window_putch(tr_window* wnd, char c)
{
    tr_window_validate_cursor(wnd);
    if(c == '\n')
    {
	++wnd->cursor.y;
	wnd->cursor.x = wnd->rect.x;
    }
    else
    {
	console_putch(wnd->cursor.x++, wnd->cursor.y, c);
    }
}

void tr_window_print(tr_window* wnd, char* str)
{
    while(*str)
	tr_window_putch(wnd, *(str++));
}

void tr_window_puts(tr_window* wnd, char* str)
{
    tr_window_print(wnd, str);
    tr_window_putch(wnd, '\n');
}

void tr_window_delch(tr_window* wnd)
{
    if(wnd->cursor.x == FIRST_COL(wnd->rect))
    {
	if(wnd->cursor.y == FIRST_LINE(wnd->rect))
	    return;

	wnd->cursor.x = LAST_COL(wnd->rect);
	--wnd->cursor.y;
    }
    else
    {
	--wnd->cursor.x;
    }

    console_putch(wnd->cursor.x, wnd->cursor.y, ' ');
}

void tr_window_clear(tr_window* wnd)
{
    int x, y;
    for(y = FIRST_LINE(wnd->rect); y <= LAST_LINE(wnd->rect); ++y)
	for(x = FIRST_COL(wnd->rect); x <= LAST_COL(wnd->rect); ++x)
	    console_putch(x, y, ' ');

    wnd->cursor.x = wnd->rect.x;
    wnd->cursor.y = wnd->rect.y;
}

void tr_window_delete(tr_window* wnd)
{
    
}
