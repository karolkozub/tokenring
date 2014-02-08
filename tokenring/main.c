#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "chat.h"
#include "ring_node.h"
#include "rs_port.h"
#include "rs_defs.h"

int get_com(int num)
{
    switch(num)
    {
    case 1:
	return COM1;
    case 2:
	return COM2;
    case 3:
	return COM3;
    case 4:
	return COM4;
    }
    
    return 0;
}

void init_abort(char* str)
{
    perror(str);
    exit(1);
}

void init_ports(rs_port** rs_in, rs_port** rs_out, int argc, char** argv)
{
    int i;
    int in = 0, out = 0, com;

    printf("Starting init_ports\n");

    for(i = 1; i < argc; ++i)
    {
	printf("Argument: %s\n", argv[i]);

	if(in && !*rs_in)
	{
	    printf("in :: wczytywanie wartosci\n");
	    if(!sprintf(argv[i], "%d", &in))
		printf("in :: nie udalo sie wyczytac\n");
	    if(!(com = get_com(in)))
		init_abort("Invalid value for COM_IN");

	    printf("*rs_in -> 0x%x (%d)\n", com, in);
	    *rs_in = rs_port_create(in, BAUD_9600, WLEN_8, STOP_1, NO_PARITY);
	    continue;
	}
	if(out && !*rs_out)
	{
	    printf("out :: wczytywanie wartosci\n");
	    if(!sprintf(argv[i], "%d", &out))
		printf("out :: nie udalo sie wyczytac\n");
	    if(!(com = get_com(out)))
		init_abort("Invalid value for COM_OUT");

	    printf("*rs_out -> 0x%x (%d)\n", com, out);
	    *rs_out = rs_port_create(out, BAUD_9600, WLEN_8, STOP_1, NO_PARITY);
	    continue;
	}
	if(!(strcmp(argv[i], "-i") && strcmp(argv[i], "--com-in")))
	{
	    printf("in :: rozpoznano flage\n");
	    in = 255;
	    continue;
	}
	if(!(strcmp(argv[i], "-o") && strcmp(argv[i], "--com-out")))
	{
	    printf("out :: rozpoznano flage\n");
	    out = 255;
	    continue;
	}
    }

    if(!*rs_in || !*rs_out)
	init_abort("You have to init com ports via\n * -i <com_in> -o <com_out>");
}

int main(int argc, char* argv[])
{
    rs_port* rs_out = 0;
    rs_port* rs_in = 0;
    ring_node* rn;
    rectangle cht_rect;
    chat* cht;

    rs_in  = rs_port_create(COM1, BAUD_9600, WLEN_8, STOP_1, NO_PARITY);
    rs_out = rs_port_create(COM2, BAUD_9600, WLEN_8, STOP_1, NO_PARITY);

    rn = ring_node_create(rs_in, rs_out);

    cht_rect.x      =  0;
    cht_rect.y      =  0;
    cht_rect.width  = 80;
    cht_rect.height = 25;

    cht = chat_create(rn, cht_rect);

    for(;;)
    {
	chat_update(cht);
    }

    chat_delete(cht);
    ring_node_delete(rn);
    rs_port_delete(rs_in);
    rs_port_delete(rs_out);
    
    return 0;
}
