#include <stdlib.h>

#include "pkg_buffer.h"

struct pkg_buffer
{
    package** buf;
    int buf_size;
    int start;
    int count;
};

pkg_buffer* pkg_buffer_create(int start_size)
{
    pkg_buffer* buf = (pkg_buffer*)malloc(sizeof(pkg_buffer));

    buf->buf_size = start_size > 0 ? start_size : 1;
    buf->buf = (package**)malloc(buf->buf_size * sizeof(package*));

    buf->start = buf->count = 0;

    return buf;
}

static int pkg_buffer_full(pkg_buffer* buf)
{
    return buf->count == buf->buf_size;
}

static void pkg_buffer_resize(pkg_buffer* buf)
{
    int i, new_size = buf->buf_size * 2;

    buf->buf = (package**)realloc(buf->buf, new_size);

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

void pkg_buffer_put(pkg_buffer* buf, package* pkg)
{
    if(pkg_buffer_full(buf))
	pkg_buffer_resize(buf);

    buf->buf[(buf->start + buf->count)%buf->buf_size] = pkg;
    ++buf->count;
}

package* pkg_buffer_get(pkg_buffer* buf)
{
    package* pkg = buf->buf[buf->start];
    
    buf->start = (buf->start+1) % buf->buf_size;
    --buf->count;
    
    return pkg;
}

package* pkg_buffer_peek(pkg_buffer* buf)
{
    return buf->buf[buf->start];
}

int pkg_buffer_empty(pkg_buffer* buf)
{
    return buf->count == 0;
}

int pkg_buffer_count(pkg_buffer* buf)
{
    return buf->count;
}

void pkg_buffer_delete(pkg_buffer* buf)
{
    free(buf->buf);
    free(buf);
}
