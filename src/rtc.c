#include "abcio.h"
#include "compiler.h"
#include "z80.h"

#include <time.h>

static uint8_t bytes[8]; /* YY YY mm dd HH MM SS FF */
static int ptr;

/*
 * ASCII string - will be converted to BCD
 * __ is a placeholder for the command phase
 */
static uint8_t e05time[17 * 2]; /* __ HH MM dd mm YY ?? SS ?? */
static unsigned int e05bit;
static uint8_t e05cmd;

#ifdef __WIN32__

#    include <windows.h>

/* Windows: use GetLocalTime */

static void sys_latch_time(void)
{
    SYSTEMTIME lt;

    GetLocalTime(&lt);

    bytes[0] = lt.wYear / 100;
    bytes[1] = lt.wYear % 100;
    bytes[2] = lt.wMonth;
    bytes[3] = lt.wDay;
    bytes[4] = lt.wHour;
    bytes[5] = lt.wMinute;
    bytes[6] = lt.wSecond;
    bytes[7] = lt.wMilliseconds / 20;
}

#else

/* Unix-like API assumed */

#    include <sys/time.h>

static void sys_latch_time(void)
{
    struct timeval tv;
    const struct tm* tm;
    time_t t;

    gettimeofday(&tv, NULL);
    t = tv.tv_sec;
    tm = localtime(&t);

    bytes[0] = tm->tm_year / 100 + 19;
    bytes[1] = tm->tm_year % 100;
    bytes[2] = tm->tm_mon + 1;
    bytes[3] = tm->tm_mday;
    bytes[4] = tm->tm_hour;
    bytes[5] = tm->tm_min;
    bytes[6] = tm->tm_sec;
    bytes[7] = tv.tv_usec / 20000;
}

#endif

static void latch_time(void)
{
    sys_latch_time();

    sprintf((char*)e05time, "__%02u%02u%02u%02u%02u??%02u??", bytes[4],
            bytes[5], bytes[3], bytes[2], bytes[1] /* ,??? */,
            bytes[6] /* ,??? */);

    ptr = 0;             /* Reset pointer for bus clock */
    e05cmd = e05bit = 0; /* Reset pointer for E05-16 */
}

/* This is the "fake" ABCbus-connected RTC */
int rtc_in(int sel, int port)
{
    uint8_t b;

    (void)sel;

    switch (port) {
    case 0:
        b = bytes[ptr];
        ptr = (ptr + 1) & 7;
        break;
    case 1:
        latch_time();
        b = 0xd2; /* Presence check */
        break;
    default:
        b = 0xff;
        break;
    }

    return b;
}

/* ABC806 RTC (E05-16) */

static uint8_t e05state;

void abc806_rtc_out(uint8_t port, uint8_t val)
{
    uint8_t oldstate, reg, set, clr;

    if (port & 1)
        return; /* No idea what this does */

    reg = val & 7;
    oldstate = e05state;
    e05state = (val >> 7 << reg) | (e05state & ~(1 << reg));
    set = e05state & ~oldstate;
    clr = ~e05state & oldstate;

    switch (reg) {
    case 0:
        /* "EME* = ? */
        break;
    case 1:
        /* Mode40 - set here? */
        break;
    case 2:
        /* Bit A8 on "7621/HRU11" = ? */
        break;
    case 3:
        /* INI# on "PROT" = ? */
        break;
    case 4:
        /* "TXOFF" = ? */
        break;
    case 5:
        /* RTC chip select */
        if (set)
            latch_time();
        break;
    case 6:
        if (clr && (e05state & 0x20)) {
            e05bit++;
            if (e05bit == 5)
                e05bit = ((e05cmd & 7) << 3) + 8; /* Pure guess */
            else if (e05bit >= 9 * 8)
                e05bit = 8;
        }
        break;
    case 7:
        /* Data out (OC) must be 1 to read */
        if (e05bit < 4) {
            e05cmd =
                (e05cmd & ~(1 << e05bit)) | (((uint8_t)~val >> 7) << e05bit);
        }
        break;
    }
}

uint8_t abc806_rtc_in(uint8_t port)
{
    uint8_t v;
    unsigned int bp, by, bi;

    if (!(port & 1))
        return 0xff; /* No idea */

    /* D3:0 has something to do with "7621/HRU11" = ?; assume 1 for now */
    v = 0xff;

    /* D7 is RTC output */
    bp = e05bit ^ 7;
    by = bp >> 2;
    bi = (bp & 3) + 4;
    if (e05state & 0x20)
        v = (v & 0x7f) | ((e05time[by] << bi) & 0x80 & e05state);

    return v;
}
