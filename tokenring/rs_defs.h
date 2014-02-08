#ifndef RS_DEFS_H
#define RS_DEFS_H

#define HI_BYTE(x) (((x) >> 8) & 0xFF)
#define LO_BYTE(x) ((x) & 0xFF)

typedef enum
{
    COM1 = 0x3F8,
    COM2 = 0x2F8,
    COM3 = 0x3E8,
    COM4 = 0x2E8
} port;

typedef enum
{
    BAUD_11520 = 0x001,
    BAUD_56000 = 0x002,
    BAUD_38400 = 0x003,
    BAUD_19200 = 0x006,
    BAUD_9600  = 0x00C,
    BAUD_4800  = 0x018,
    BAUD_2400  = 0x030,
    BAUD_1200  = 0x060,
    BAUD_600   = 0x0C0,
    BAUD_300   = 0x180,
    BAUD_150   = 0x300,
    BAUD_110   = 0x417,
    BAUD_50    = 0x900
} baud_rate;

typedef enum
{
    WLEN_5 = 0,
    WLEN_6 = 1,
    WLEN_7 = 2,
    WLEN_8 = 3
} word_len;

typedef enum
{
    STOP_1 = 0,
    STOP_2 = 4
} stop_bit;

typedef enum
{
    NO_PARITY   = 0x00,
    ODD_PARITY  = 0x08,
    EVEN_PARITY = 0x18
} parity;

typedef enum
{
    NO_DIV_LATCH = 0x00,
    DIV_LATCH    = 0x80
} div_latch;

typedef enum
{
    RECV_READY  = 0x01,
    OVERRUN_ERR = 0x02,
    PARITY_ERR  = 0x04,
    FRAME_ERR   = 0x08,
    BREAK_INT   = 0x10,
    SEND_READY  = 0x20,
    RECV_ERR    = 0x80
} line_status;

typedef enum
{
    REG_IO        = 0,
    REG_DIV_LO    = 0,
    REG_INT_ACT   = 1,
    REG_DIV_HI    = 1,
    REG_INT_IDENT = 2,
    REG_LCR       = 3,
    REG_MCR       = 4,
    REG_LSR       = 5,
    REG_MSR       = 6
} reg_offset;

#endif