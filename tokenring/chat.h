#ifndef CHAT_H
#define CHAT_H

#include "ring_node.h"

struct chat;
typedef struct chat chat;

chat* chat_create(ring_node*, rectangle);
void chat_update(chat*);
void chat_delete(chat*);

#endif
