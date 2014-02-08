#ifndef PKG_BUFFER_H
#define PKG_BUFFER_H

#include "package.h"

struct pkg_buffer;
typedef struct pkg_buffer pkg_buffer;

pkg_buffer* pkg_buffer_create(int start_size);

void pkg_buffer_put(pkg_buffer*, package*);
package* pkg_buffer_get(pkg_buffer*);
package* pkg_buffer_peek(pkg_buffer*);

void pkg_buffer_push_back(pkg_buffer*, package*);
package* pkg_buffer_pop_back(pkg_buffer*);
package* pkg_buffer_peek_back(pkg_buffer*);

int pkg_buffer_empty(pkg_buffer*);
int pkg_buffer_count(pkg_buffer*);

void pkg_buffer_delete(pkg_buffer*);

#endif

