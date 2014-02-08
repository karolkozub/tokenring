#include <assert.h>
#include <string.h>
#include "package.h"

package* package_create_init_id(byte id)
{
    package* pkg = (package*)malloc(sizeof(package));
    memset(pkg, 0, sizeof(package));
    
    pkg->type = PKG_INIT_ID;
    pkg->id   = id;

    return pkg;
}

package* package_create_init_size(byte size)
{
    package* pkg = (package*)malloc(sizeof(package));
    memset(pkg, 0, sizeof(package));
    
    pkg->type = PKG_INIT_SIZE;
    pkg->ring_size = size;
    
    return pkg;
}

package* package_create_token(void)
{
    package* pkg = (package*)malloc(sizeof(package));
    memset(pkg, 0, sizeof(package));
    
    pkg->type = PKG_TOKEN;
    
    return pkg;
}

package* package_create_data(byte src, byte dest, byte* data, byte data_size)
{
    package* pkg = (package*)malloc(sizeof(package));
    memset(pkg, 0, sizeof(package));
    
    pkg->type = PKG_DATA;
    pkg->src  = src;
    pkg->dest = dest;
    pkg->data = (byte*)calloc(data_size, sizeof(byte));
    memcpy(pkg->data, data, data_size);
    pkg->data_size = data_size;
    
    return pkg;
}

package* package_read_from_buf(byte_buffer* buf)
{
    static const byte header_size = 3;
    byte pkg_size, i;
    package* pkg;

    if(byte_buffer_empty(buf))
	return 0;
    
    pkg_size = byte_buffer_peek(buf);
    
    /* jesli wielkosc pakietu (plus bajt z wielkoscia) jest wieksza niz to co mamy 
       to mamy za malo      */
    if(pkg_size + 1 > byte_buffer_count(buf))
	return 0;

    /* juz wyczytalismy pkg_size za pomoca peek*/
    byte_buffer_get(buf);

    pkg = (package*)malloc(sizeof(package));
    memset(pkg, 0, sizeof(package));
    
    pkg->type = byte_buffer_get(buf);
    
    switch(pkg->type)
    {
    case PKG_INIT_ID:
	assert(pkg_size == 2);
	pkg->id = byte_buffer_get(buf);
	break;
    case PKG_INIT_SIZE:
	assert(pkg_size == 2);
	pkg->ring_size = byte_buffer_get(buf);
	break;
    case PKG_DATA:
	assert(pkg_size > header_size);
	pkg->src = byte_buffer_get(buf);
	pkg->dest = byte_buffer_get(buf);
	pkg->data_size = pkg_size - header_size;
	pkg->data = (byte*)malloc(pkg->data_size);
	for(i = 0; i < pkg->data_size; ++i)
	    pkg->data[i] = byte_buffer_get(buf);
	break;
    case PKG_TOKEN:
	assert(pkg_size == 1);
	break;
    default:
	assert(0);
    }

    return pkg;
}

void package_write_to_buf(package* pkg, byte_buffer* buf)
{
    static const byte header_size = 3;
    int i;

    switch(pkg->type)
    {
    case PKG_INIT_ID:
	byte_buffer_put(buf, 2);
	byte_buffer_put(buf, pkg->type);
	byte_buffer_put(buf, pkg->id);
	break;
    case PKG_INIT_SIZE:
	byte_buffer_put(buf, 2);
	byte_buffer_put(buf, pkg->type);
	byte_buffer_put(buf, pkg->ring_size);
	break;
    case PKG_DATA:
	byte_buffer_put(buf, header_size + pkg->data_size);
	byte_buffer_put(buf, pkg->type);
	byte_buffer_put(buf, pkg->src);
	byte_buffer_put(buf, pkg->dest);
	for(i = 0; i < pkg->data_size; ++i)
	    byte_buffer_put(buf, pkg->data[i]);
	break;
    case PKG_TOKEN:
	byte_buffer_put(buf, 1);
	byte_buffer_put(buf, pkg->type);
	break;
    default:
	assert(0);
    }
}

package* package_copy(package* pkg)
{
    package* copy = (package*)malloc(sizeof(package));
    memcpy(copy, pkg, sizeof(package));

    if(pkg->type == PKG_DATA)
    {
	copy->data = (byte*)calloc(pkg->data_size, sizeof(byte));
	memcpy(copy->data, pkg->data, pkg->data_size);
    }

    return copy;
}

void package_delete(package* pkg)
{
    assert(pkg->data ? (pkg->type) == PKG_DATA : 1);

    free(pkg->data);
    free(pkg);
}
