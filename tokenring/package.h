#ifndef PACKAGE_H
#define PACKAGE_H

#include <stdio.h>
#include "defs.h"
#include "byte_buffer.h"

enum pkg_type
{
    PKG_INIT_ID   = 0,
    PKG_INIT_SIZE = 1,
    PKG_TOKEN     = 2,
    PKG_DATA      = 3
};

typedef struct
{
    byte type;

    byte src;
    byte dest;
    byte* data;
    byte data_size;

    byte id;

    byte ring_size;
} package;

package* package_create_init_id(byte id);
package* package_create_init_size(byte ring_size);
package* package_create_token(void);
package* package_create_data(byte src, byte dest, byte* data, byte data_size);
/* zwraca 0 jesli w buforze nie znajduje sie pelna wiadomosc */
package* package_read_from_buf(byte_buffer*);
void package_write_to_buf(package*, byte_buffer*);

package* package_copy(package*);

void package_delete(package*);


#endif
