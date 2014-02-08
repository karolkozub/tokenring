#ifndef RS_PORT_H
#define RS_PORT_H

#include "defs.h"
#include "rs_defs.h"

struct rs_port;
typedef struct rs_port rs_port;

rs_port* rs_port_create(port, baud_rate, word_len, parity, stop_bit);

void rs_port_update(rs_port*);

void rs_port_send_byte(rs_port*, byte);
byte rs_port_recv_byte(rs_port*);
int rs_port_poll_in(rs_port*);

int rs_port_send(rs_port*, byte*, int);
int rs_port_recv(rs_port*, byte*, int);

void rs_port_delete(rs_port*);

#endif