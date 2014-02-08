#ifndef RING_NODE_H
#define RING_NODE_H

#include "defs.h"
#include "package.h"
#include "rs_port.h"

struct ring_node;
typedef struct ring_node ring_node;

/* ring_node nie zarzadza portami - tylko z nich korzysta,
   dlatego ich nie tworzy ani nie usuwa */
ring_node* ring_node_create(rs_port* in, rs_port* out);
void ring_node_delete(ring_node*);

void ring_node_send_init(ring_node*);
void ring_node_generate_token(ring_node*);

void ring_node_update(ring_node*);

void ring_node_send(ring_node*, byte* tab, int size, int dest);
/* Zwraca 0 jesli nie ma nic do odebrania. 
   Odebrana wiadomosc trzeba recznie zwolnic (package_delete)*/
package* ring_node_recv(ring_node*);

static enum ring_node_state
{
    STATE_PRE_INIT,
    STATE_INIT,
    STATE_WAITING,
    STATE_HAS_TOKEN
};
typedef enum ring_node_state ring_node_state;

static enum ring_node_id
{
    ID_UNKNOWN   =  -1,
    ID_HOST      =   0,
    ID_ALL       = 255
};
typedef enum ring_node_id ring_node_id;

ring_node_state ring_node_get_state(ring_node* rn);
int ring_node_get_id(ring_node* rn);
int ring_node_get_size(ring_node* rn);
int ring_node_get_token_count(ring_node* rn);
int ring_node_get_sent_count(ring_node* rn);
int ring_node_get_recvd_count(ring_node* rn);

#endif
