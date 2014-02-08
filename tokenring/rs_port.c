#include <dos.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
# include <stdio.h>
#endif

#include "rs_port.h"
#include "rs_defs.h"
#include "byte_buffer.h"

struct rs_port
{
    port port;
    byte_buffer* buf_in;
    byte_buffer* buf_out;
};

rs_port* rs_port_create(port port, baud_rate baud_rate, word_len word_len, stop_bit stop, parity parity)
{
    rs_port* rs = (rs_port*)malloc(sizeof(rs_port));
    rs->port = port;
    rs->buf_in = byte_buffer_create(1);
    rs->buf_out = byte_buffer_create(1);

    outportb(port+REG_INT_ACT, 0);

    outportb(port+REG_LCR, DIV_LATCH);
    outportb(port+REG_DIV_LO, LO_BYTE(baud_rate));
    outportb(port+REG_DIV_HI, HI_BYTE(baud_rate));

    outportb(port+REG_LCR, word_len | stop | parity);

    return rs;
}

static int rs_port_send_ready(rs_port* rs)
{
    return inportb(rs->port+REG_LSR) & SEND_READY;
}

static int rs_port_recv_ready(rs_port* rs)
{
    return inportb(rs->port+REG_LSR) & RECV_READY;
}

void rs_port_update(rs_port* rs)
{
    while(!byte_buffer_empty(rs->buf_out) && rs_port_send_ready(rs))
	outportb(rs->port+REG_IO, byte_buffer_get(rs->buf_out));

    while(rs_port_recv_ready(rs))
	byte_buffer_put(rs->buf_in, inportb(rs->port+REG_IO));
}

void rs_port_send_byte(rs_port* rs, byte b)
{
    byte_buffer_put(rs->buf_out, b);
}

byte rs_port_recv_byte(rs_port* rs)
{
    return byte_buffer_get(rs->buf_in);
}

int rs_port_poll_in(rs_port* rs)
{
    return !byte_buffer_empty(rs->buf_in);
}

int rs_port_send(rs_port* rs, byte* tab, int size)
{
    int sent;
    
    for(sent = 0; sent < size; ++sent)
	rs_port_send_byte(rs, tab[sent]);
    
    return sent;
}

int rs_port_recv(rs_port* rs, byte* tab, int size)
{
    int recvd;
    
    for(recvd = 0; recvd < size && !byte_buffer_empty(rs->buf_in); ++recvd)
	tab[recvd] = rs_port_recv_byte(rs);

    return recvd;
}

void rs_port_delete(rs_port* rs)
{
    byte_buffer_delete(rs->buf_out);
    byte_buffer_delete(rs->buf_in);
    free(rs);
}
