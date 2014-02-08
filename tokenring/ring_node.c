#include <assert.h>

#include "defs.h"
#include "ring_node.h"
#include "rs_port.h"
#include "package.h"
#include "pkg_buffer.h"
#include "byte_buffer.h"

/* DEBUG */
#include <stdio.h>

#define ASSERT_IS_UBYTE(x) assert((x) & 0xFF) 

/* Protokol tokenringa:
 * 
 * Zawsze pierwszy bajt to rozmiar calego pakietu, a drugi bajt to typ pakietu:
 * 0x00 - init :: przypisanie id
 * 0x01 - init :: rozmiar sieci
 * 0x02 - token
 * 0x03 - dane
 * 
 * init:
 * Pierwszy obieg ma na celu przypisanie kazdemu komputerowi unikalnego identyfikatora.
 * Pakiet ma postac 0x0200XX, gdzie na bajcie zaznaczonym jako XX podany jest nowy identyfikator. Po odebraniu tego pakietu komputer zapisuje sobie swoj id, a nastepnie przesyla pakiet dalej zwiekszajac XX o 1. Po wykonaniu pelnego okrazenia komputer inicjujacy ten proces powinien odebrac pakiet odczytujac z niego informacje o ilosci komputerow w sieci.
 * Nastepnie komputer ten powinien wyslac nowy pakiet do sieci, informujacy o ilosci komputerow w sieci reszte.
 * Pakiet ten ma postac 0x0201XX, gdzie XX to ilosc komputerow w sieci.
 * 
 * token:
 * Pakiet sluzy tylko i wylacznie do przekazania tokenu do kolejnego komputera w pierscieniu.
 * Pakiet sklada sie z dwoch bajtow 0x0102
 *
 * dane:
 * Pakiet sluzy do przekazywania porcji danych z jednego komputera do drugiego
 * Pakiet ma postac 0xRR03ZZCCDDDDDDD..., przy czym RR oznacza rozmiar pakietu (max 255), ZZ to id zrodla, CC to id celu (id == 0xFF jest specjalnym id oznaczajacym wyslanie wiadomosce do wszystkich komputerow w sieci (dlatego maksymalna ilosc komputerow w sieci jest ogranoczona do 255), a nastepne (0xRR - 3) bajtow zawiera te dane.
 * Przykladowo w pakiecie 0x0D03000100010203040506070809 :
 * zrodlem jest komputer 0x00
 * celem   jest komputer 0x01
 * przesylanie dane to (0x0D - 3 == 10 bajtow): 0x00010203040506070809
 *
 * Zaden komputer nie moze przetrzymywac tokenu jak nie ma nic do wyslania, ani wyslac wiecej niz jednego pakietu z danymi bez oddania tokenu.
 */

static enum token_ring_size
{
    SIZE_UNKNOWN = -1
};

static enum token_ring_consts
{
    PKG_MAX_SIZE = 255
};

struct ring_node
{
    rs_port* rs_in;
    rs_port* rs_out;

    /* przechowuje odebrane bajty; gdy uzbiera sie pelny pakiet przenoszony jest do buf_in_pkg */
    byte_buffer* buf_in_raw;
    /* przechowuje pelne pakiety; po przetworzeniu pakiety z danymi dla tego wezla laduja w buf_in_recv */
    pkg_buffer* buf_in_pkg;
    /* przechowuje pakiety oczekujace na odebranie */
    pkg_buffer* buf_in_waiting;

    /* przechowuje bajty do wyslania */
    byte_buffer* buf_out_raw;
    /* przechowuje pakiety do wyslania */
    pkg_buffer* buf_out_pkg;
    /* przechowuje pakiety oczekujace na wyslanie (token) */
    pkg_buffer* buf_out_waiting;
   
    ring_node_state state;
    int id;
    int ring_size;
    unsigned int token_count;
    unsigned int sent;
    unsigned int recvd;
};

/* Wykorzystywane jest to, ze pakiet INIT_ID po dotarciu do hosta 
   bedzie zawieral wielkosc sieci */
static void handle_host_init_id(ring_node* rn, package* pkg)
{
    assert(rn->state == STATE_INIT);

    ring_node_generate_token(rn);
    rn->ring_size  = pkg->id;
    
    package_delete(pkg);
    pkg_buffer_put(rn->buf_out_pkg, package_create_init_size(rn->ring_size));
}

static void handle_normal_init_id(ring_node* rn, package* pkg)
{
    assert(rn->state == STATE_PRE_INIT)

    rn->state = STATE_INIT;
    rn->id    = pkg->id;

    assert(rn->id > ID_HOST && rn->id < ID_ALL);
    
    pkg->id += 1;
    pkg_buffer_put(rn->buf_out_pkg, pkg);
}

static void handle_host_init_size(ring_node* rn, package* pkg)
{
    /* ignoruj */

    package_delete(pkg);
}

static void handle_normal_init_size(ring_node* rn, package* pkg)
{
    assert(rn->state == STATE_INIT);

    rn->state     = STATE_WAITING;
    rn->ring_size = pkg->ring_size;
    
    pkg_buffer_put(rn->buf_out_pkg, pkg);
}

static void handle_data(ring_node* rn, package* pkg)
{
    assert(rn->state >= STATE_WAITING);

    if(pkg->src == rn->id)
    {
	package_delete(pkg);

	return;
    }

    /* Jesli to do nas to tylko odbieramy*/
    if(pkg->dest == rn->id)
    {
	pkg_buffer_put(rn->buf_in_waiting, pkg);

	return;
    }

    /* Jesli to do wszystkich to zapisaujemy kopie a potem wysylamy*/
    if(pkg->dest == (byte)ID_ALL)
	pkg_buffer_put(rn->buf_in_waiting, package_copy(pkg));

    /* wpp tylko wysylamy*/
    pkg_buffer_put(rn->buf_out_pkg, pkg);
}

static void handle_token(ring_node* rn, package* pkg)
{
    ring_node_generate_token(rn);

    package_delete(pkg);
}

static void ring_node_update_send_waiting(ring_node* rn)
{
    if(rn->state != STATE_HAS_TOKEN)
	return;

    rn->state = STATE_WAITING;

    if(!pkg_buffer_empty(rn->buf_out_waiting))
	pkg_buffer_put(rn->buf_out_pkg, pkg_buffer_get(rn->buf_out_waiting));

    pkg_buffer_put(rn->buf_out_pkg, package_create_token());
}

static void ring_node_update_send_pkg(ring_node* rn)
{
    package* pkg;

    if(pkg_buffer_empty(rn->buf_out_pkg))
	return;

    pkg = pkg_buffer_get(rn->buf_out_pkg);

    package_write_to_buf(pkg, rn->buf_out_raw);
    package_delete(pkg);
}

static void ring_node_update_send_raw(ring_node* rn)
{
    while(!byte_buffer_empty(rn->buf_out_raw))
    {
	rs_port_send_byte(rn->rs_out, byte_buffer_get(rn->buf_out_raw));
	++rn->sent;
    }
}

static void ring_node_update_recv(ring_node* rn)
{
    int ch;
    package* pkg;

    /* odbieranie "surowych danych" z portu */
    while(rs_port_poll_in(rn->rs_in))
    {
	++rn->recvd;
	byte_buffer_put(rn->buf_in_raw, rs_port_recv_byte(rn->rs_in));
    }
    /* sprobuj utworzyc pakiet na podstawie danych */
    pkg = package_read_from_buf(rn->buf_in_raw);
    
    if(pkg)
	pkg_buffer_put(rn->buf_in_pkg, pkg);
}

static void ring_node_process_packages(ring_node* rn)
{
    package* pkg;

    while(!pkg_buffer_empty(rn->buf_in_pkg))
    {
	pkg = pkg_buffer_get(rn->buf_in_pkg);

	switch(pkg->type)
	{
	case PKG_INIT_ID:
	    if(rn->id == ID_HOST)
		handle_host_init_id(rn, pkg);
	    else
		handle_normal_init_id(rn, pkg);
	    break;
	case PKG_INIT_SIZE:
	    if(rn->id == ID_HOST)
		handle_host_init_size(rn, pkg);
	    else
		handle_normal_init_size(rn, pkg);
	    break;
	case PKG_DATA:
	    handle_data(rn, pkg);
	    break;
	case PKG_TOKEN:
	    handle_token(rn, pkg);
	    break;
	default:
	    assert(0);
	}
    }
}

ring_node* ring_node_create(rs_port* rs_in, rs_port* rs_out)
{
    ring_node* rn = (ring_node*)malloc(sizeof(ring_node));

    rn->rs_in  = rs_in;
    rn->rs_out = rs_out;

    rn->buf_in_raw      = byte_buffer_create(1);
    rn->buf_out_raw     = byte_buffer_create(1);
    rn->buf_in_pkg      = pkg_buffer_create(1);
    rn->buf_out_pkg     = pkg_buffer_create(1);
    rn->buf_in_waiting  = pkg_buffer_create(1);
    rn->buf_out_waiting = pkg_buffer_create(1);

    rn->state     = STATE_PRE_INIT;
    rn->id        = ID_UNKNOWN;
    rn->ring_size = SIZE_UNKNOWN;

    rn->token_count = 0;
    rn->sent        = 0;
    rn->recvd       = 0;

    return rn;
}

void ring_node_delete(ring_node* rn)
{
    byte_buffer_delete(rn->buf_in_raw);
    byte_buffer_delete(rn->buf_out_raw);
    pkg_buffer_delete(rn->buf_in_pkg);
    pkg_buffer_delete(rn->buf_out_pkg);
    pkg_buffer_delete(rn->buf_in_waiting);
    pkg_buffer_delete(rn->buf_out_waiting);

    free(rn);
}

void ring_node_send_init(ring_node* rn)
{
    assert(rn->state == STATE_PRE_INIT);

    rn->state = STATE_INIT;
    rn->id    = ID_HOST;

    pkg_buffer_put(rn->buf_out_pkg, package_create_init_id(rn->id + 1));
}

void ring_node_generate_token(ring_node* rn)
{
    rn->state      = STATE_HAS_TOKEN;
    ++(rn->token_count);
}

void ring_node_update(ring_node* rn)
{
    rs_port_update(rn->rs_in);
    rs_port_update(rn->rs_out);

    /* odbierz */
    ring_node_update_recv(rn);
    /* przetworz */
    ring_node_process_packages(rn);
    /* wyslij */
    ring_node_update_send_waiting(rn);
    ring_node_update_send_pkg(rn);
    ring_node_update_send_raw(rn);
}

void ring_node_send(ring_node* rn, byte* tab, int size, int dest)
{
    static const int pkg_max_data_size = PKG_MAX_SIZE - 3;
    int pkg_data_size;

    while(size > 0)
    {
	pkg_data_size = min(pkg_max_data_size, size);

	pkg_buffer_put(rn->buf_out_waiting, package_create_data(rn->id, dest, tab, pkg_data_size));

	size -= pkg_data_size;
	tab  += pkg_data_size;
    }
}

package* ring_node_recv(ring_node* rn)
{
    if(pkg_buffer_empty(rn->buf_in_waiting))
	return 0;

    return pkg_buffer_get(rn->buf_in_waiting);
}

ring_node_state ring_node_get_state(ring_node* rn)
{
    return rn->state;
}

int ring_node_get_id(ring_node* rn)
{
    return rn->id;
}

int ring_node_get_size(ring_node* rn)
{
    return rn->ring_size;
}

int ring_node_get_token_count(ring_node* rn)
{
    return rn->token_count;
}

int ring_node_get_sent_count(ring_node* rn)
{
    return rn->sent;
}

int ring_node_get_recvd_count(ring_node* rn)
{
    return rn->recvd;
}
