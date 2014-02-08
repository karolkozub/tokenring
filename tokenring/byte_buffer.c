#include <stdlib.h>

#ifdef DEBUG
# include <stdio.h>
#endif

#include "byte_buffer.h"

struct byte_buffer
{
    byte* buf;
    int buf_size;
    int start;
    int count;
};

byte_buffer* byte_buffer_create(int start_size)
{
    byte_buffer* buf = (byte_buffer*)malloc(sizeof(byte_buffer));

    buf->buf_size = start_size > 0 ? start_size : 1;
    buf->buf = (byte*)malloc(buf->buf_size * sizeof(byte));

    buf->start = buf->count = 0;

    return buf;
}

static int byte_buffer_full(byte_buffer* buf)
{
    return buf->count == buf->buf_size;
}

static void byte_buffer_resize(byte_buffer* buf)
{
    int i, new_size = buf->buf_size * 2;

    buf->buf = (byte*)realloc(buf->buf, new_size);

    /* Jesli elementy "przylegaja do konca",
       to trzeba je przesunac na nowy koniec. */
    if((buf->start+buf->count) > buf->buf_size)
    {
	for(i = buf->start; i < buf->buf_size; ++i)
	    buf->buf[i + buf->buf_size] = buf->buf[i];

	buf->start += buf->buf_size;
    }

    buf->buf_size = new_size;
}

void byte_buffer_put(byte_buffer* buf, byte b)
{
    if(byte_buffer_full(buf))
	byte_buffer_resize(buf);

    buf->buf[(buf->start + buf->count)%buf->buf_size] = b;
    ++buf->count;
}

byte byte_buffer_get(byte_buffer* buf)
{
    byte b = buf->buf[buf->start];
    
    buf->start = (buf->start+1) % buf->buf_size;
    --buf->count;
    
    return b;
}

byte byte_buffer_peek(byte_buffer* buf)
{
    return buf->buf[buf->start];
}

int byte_buffer_empty(byte_buffer* buf)
{
    return buf->count == 0;
}

int byte_buffer_count(byte_buffer* buf)
{
    return buf->count;
}

void byte_buffer_delete(byte_buffer* buf)
{
    free(buf->buf);
    free(buf);
}
