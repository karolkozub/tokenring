#ifndef BYTE_BUFFER_H
#define BYTE_BUFFER_H

#include "defs.h"

struct byte_buffer;
typedef struct byte_buffer byte_buffer;

byte_buffer* byte_buffer_create(int start_size);

void byte_buffer_put(byte_buffer*, byte);
byte byte_buffer_get(byte_buffer*);
byte byte_buffer_peek(byte_buffer*);

int byte_buffer_empty(byte_buffer*);
int byte_buffer_count(byte_buffer*);

void byte_buffer_delete(byte_buffer*);

#endif

