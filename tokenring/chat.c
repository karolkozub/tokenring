#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <ctype.h>
#include "chat.h"
#include "window.h"
#include "console.h"

#define ASCII_ESC 27
#define ASCII_ENTER 13
#define ASCII_BACKSPACE 8

struct chat
{
    rectangle rect;
    ring_node* rn;
    tr_window* input;
    tr_window* display;

    char buf_input[256];
    int  buf_count;
};

static void chat_draw(chat* cht)
{
    int i;

    console_clear();

    /* rogi */
    console_putch(cht->rect.x, cht->rect.y, 201);
    console_putch(cht->rect.x+cht->rect.width-1, cht->rect.y, 187);
    console_putch(cht->rect.x, cht->rect.y+cht->rect.height-1, 200);
    console_putch(cht->rect.x+cht->rect.width-1, cht->rect.y+cht->rect.height-1, 188);

    for(i = cht->rect.x+1; i < cht->rect.x+cht->rect.width-1; ++i)
    {
	console_putch(i, cht->rect.y, 205);
	console_putch(i, cht->rect.y+cht->rect.height-5, 196);
	console_putch(i, cht->rect.y+cht->rect.height-1, 205);
    }

    for(i = cht->rect.y+1; i < cht->rect.y+cht->rect.height-1; ++i)
    {
	console_putch(cht->rect.x, i, 186);
	console_putch(cht->rect.x+cht->rect.width-1, i, 186);
    }

    /* poprawki */
    console_putch(cht->rect.x, cht->rect.y+cht->rect.height-5, 199);
    console_putch(cht->rect.x+cht->rect.width-1, cht->rect.y+cht->rect.height-5, 182);

    tr_window_clear(cht->display);
    tr_window_clear(cht->input);
}

chat* chat_create(ring_node* rn, rectangle rect)
{
    chat* cht = (chat*)malloc(sizeof(chat));
    rectangle rect_display, rect_input;

    cht->rect = rect;
    cht->rn = rn;

    rect_display.x = rect.x + 1;
    rect_display.y = rect.y + 1;
    rect_display.width = rect.width - 2;
    rect_display.height = rect.height - 6;

    cht->display = tr_window_create(rect_display);
    
    rect_input.x = rect.x + 1;
    rect_input.y = rect.y + rect.height - 4;
    rect_input.width = rect.width - 2;
    rect_input.height = 3;

    cht->input = tr_window_create(rect_input);

    cht->buf_count = 0;

    chat_draw(cht);

    tr_window_puts(cht->display, "RSChat 0.01");

    return cht;
}

static void chat_handle_input(chat* cht, char* str)
{
    static char buf[80];
    int dest;
    int state;

    if(str[0] == '/')
    {
	if(!strncmp(str+1, "init", 4))
	{
	    tr_window_puts(cht->display, "Initializing token ring");
	    ring_node_send_init(cht->rn);
	}
	else if(!strncmp(str+1, "stats", 5))
	{
	    tr_window_puts(cht->display, "Ring node stats:");
	    state = ring_node_get_state(cht->rn);
	    sprintf(buf, "  state: %s", (state == STATE_PRE_INIT ? "PRE_INIT":
					 (state == STATE_INIT ? "INIT":
					  (state == STATE_WAITING ? "WAITING":
					   (state == STATE_HAS_TOKEN ? "HAS_TOKEN":
					    "INVALID")))));
	    tr_window_puts(cht->display, buf);
	    sprintf(buf, "  id   : %d", ring_node_get_id(cht->rn));
	    tr_window_puts(cht->display, buf);
	    sprintf(buf, "  size : %d", ring_node_get_size(cht->rn));
	    tr_window_puts(cht->display, buf);
	    sprintf(buf, "  token: %d", ring_node_get_token_count(cht->rn));
	    tr_window_puts(cht->display, buf);
	    sprintf(buf, "  sent : %d", ring_node_get_sent_count(cht->rn));
	    tr_window_puts(cht->display, buf);
	    sprintf(buf, "  recvd: %d", ring_node_get_recvd_count(cht->rn));
	    tr_window_puts(cht->display, buf);

	}
	else if(!strncmp(str+1, "clear", 5))
	{
	    tr_window_clear(cht->display);
	}
	else if(!strncmp(str+1, "msg", 3))
	{
	    str += 4;
	    if(sscanf(str, "%d %*s", &dest))
	    {
		while(isspace(*str))++str;
		while(isdigit(*str))++str;
		while(isspace(*str))++str;
		
		sprintf(buf, "priv to %d:", dest);
		tr_window_print(cht->display, buf);
		tr_window_puts(cht->display, str);
		ring_node_send(cht->rn, str, strlen(str), dest);
	    }
	    else
	    {
		tr_window_puts(cht->display, "Usage:");
		tr_window_puts(cht->display, "  /msg <dest> <message>");
	    }
	}
	else if(!strncmp(str+1, "help", 4))
	{
	    tr_window_puts(cht->display, "Valid commands:");
	    tr_window_puts(cht->display, "  /init  - initialises token ring");
	    tr_window_puts(cht->display, "  /stats - displays statistics");
	    tr_window_puts(cht->display, "  /clear - clear display");
	    tr_window_puts(cht->display, "  /msg <id> <message>  - send <message> to <id>");
	    tr_window_puts(cht->display, "  /help  - display this help message");
	}
    }
    else
    {
	if(ring_node_get_state(cht->rn) < STATE_WAITING)
	{
	    tr_window_puts(cht->display, "The token ring is not yet initialized.");
	    tr_window_puts(cht->display, "Type '/init' to initialize it.");
	    
	    return;
	}

	tr_window_print(cht->display, "me: ");
	tr_window_puts(cht->display, str);
	ring_node_send(cht->rn, str, strlen(str), ID_ALL);
    }
}

void chat_handle_recv(chat* cht, package* pkg)
{
    static char buf[32];
    int i;

    sprintf(buf, "%d%s: ", pkg->src, (pkg->dest==ID_ALL?"":"(priv)"));

    tr_window_print(cht->display, buf);

    for(i = 0; i < pkg->data_size; ++i)
	tr_window_putch(cht->display, pkg->data[i]);

    tr_window_putch(cht->display, '\n');
}

static void chat_handle_char(chat* cht, char c)
{
    int i;

    switch(c)
    {
    case ASCII_ESC:
	console_clear();
	exit(0);
    case ASCII_BACKSPACE:
	tr_window_delch(cht->input);
	if(cht->buf_count)
	    --cht->buf_count;
	break;
    case ASCII_ENTER:
	if(cht->buf_count == 0)
	    return;

	cht->buf_input[cht->buf_count] = 0;

	chat_handle_input(cht, cht->buf_input);

	cht->buf_count = 0;
	tr_window_clear(cht->input);
	break;
    default:
	if(!isprint(c))
	    return;
	tr_window_putch(cht->input, c);
	cht->buf_input[cht->buf_count++] = c;
    }
}

static void chat_update_token_count(chat* cht)
{
    static char buf[10];
    int offset,x;

    sprintf(buf, "%d", ring_node_get_token_count(cht->rn));

    x = cht->rect.x + cht->rect.width - strlen(buf) - 2;
    
    console_putch(x-1, 0, '[');
    console_putch(x+strlen(buf), 0, ']');
    for(offset = 0; offset < strlen(buf); ++offset)
	console_putch(x+offset, 0, buf[offset]);
}

void chat_update(chat* cht)
{
    char ch;
    package* pkg;
    int i;

    ring_node_update(cht->rn);

    if(pkg = ring_node_recv(cht->rn))
    {
	chat_handle_recv(cht, pkg);
	
	package_delete(pkg);
    }

    if(kbhit())
	chat_handle_char(cht, getch());

    chat_update_token_count(cht);
}

void chat_delete(chat* cht)
{
    tr_window_delete(cht->display);
    tr_window_delete(cht->input);
    free(cht);
}
