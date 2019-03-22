#include "compiler.h"

#include "abcio.h"
#include "clock.h"
#include "screen.h"
#include "trace.h"
#include "z80.h"
#include "z80irq.h"

/* Select code for ABC/4680 bus */
static int8_t abcbus_select = -1;

/* Keyboard IRQ vector */
static volatile unsigned int keyb_data;
static uint8_t keyb_fakedata;

/* These constants are designed to make dart_keyb_in() as simple as possible */
#define KEYB_NEW 0x100
#define KEYB_DOWN 0x800

/* Fake minimal-touch input */
bool faketype;

static int keyb_intack_fake(struct z80_irq* irq);
static struct z80_irq* keyb_irq;

static struct z80_irq keyb_irq_80 = IRQ(IRQ80_PIOA, NULL, NULL, NULL);
static struct z80_irq keyb_irq_fake =
    IRQ(IRQ80_PIOA, keyb_intack_fake, NULL, NULL);
static struct z80_irq keyb_irq_800 = IRQ(IRQ800_DARTB, NULL, NULL, NULL);

static inline uint8_t abc800_mangle_port(uint8_t port)
{
    if ((port & 0xe0) == 0x00)
        return port & 0xe7;
    else if ((port & 0xf0) == 0x20)
        return port & 0xf3;
    else if ((port & 0xf8) == 0x28)
        return port & 0xf9;
    else if ((port & 0xc0) == 0x40)
        return port & 0xe3;
    else
        return port;
}

/*
 * This function is called from the z80 at an OUT instruction.
 * We check if any special port was accessed and
 * dispatch possible actions.
 */
static void abcbus_out(uint8_t port, uint8_t value)
{
    if (port == 1) {
        abcbus_select = value & 0x3f;
        return;
    }

    switch (abcbus_select) {
    case 36: /* HDx: */
    case 44: /* MFx: */
    case 45: /* MOx: */
    case 46: /* SFx: */
        disk_out(abcbus_select, port, value);
        break;

    case 60: /* PRx: */
        printer_out(abcbus_select, port, value);
        break;

    default:
        break;
    }
}

static void abc80_out(uint8_t port, uint8_t value)
{
    port &= 0x17; /* Only these bits decoded in ABC80 */

    switch (port) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        abcbus_out(port, value);
        break;

    case 6: /* sound */
        if (value == 131) {
            putchar(7); /* beep */
            fflush(stdout);
        }
        break;

    case 7: /* Mikrodatorn 64K page switch port */
        abc80_mem_setmap(value & 3);
        break;

    case (57 & 0x17): /* Keyboard control port */
        if (!(value & 1)) {
            keyb_irq->vector = value;
        }
        break;

    case (58 & 0x17):
    case (59 & 0x17):
        abc80_piob_out(port, value);
        break;

    default:
        break;
    }
}

static bool vsync;
void abc802_vsync(void)
{
    vsync_screen();
    vsync = true;
}

static uint8_t dart_keyb_ctl[8];
static bool dart_keyb_vsync;

static void dart_keyb_out(uint8_t port, uint8_t value)
{
    /* Keyboard DART control */
    uint8_t reg;

    if ((port & 1) == 0) {
        return; /* Data out - ignore for now */
    }

    reg = dart_keyb_ctl[0] & 7;
    dart_keyb_ctl[0] &= ~7; /* Restore register 0 */

    dart_keyb_ctl[reg] = value;
    switch (reg) {
    case 0:
        switch ((value >> 3) & 7) {
        case 2:
            dart_keyb_vsync = vsync;
            vsync = false;
            break;
        case 3:
            memset(dart_keyb_ctl, 0, sizeof dart_keyb_ctl);
            break;
        case 4:
            break; /* Allow IRQ to be enabled */
        default:
            break;
        }
        break;
    case 5:
        setmode40(!!(value & 2));
        abc802_set_mem(!!(value & 0x80));
        break;
    default:
        break;
    }

    if ((dart_keyb_ctl[1] & 0x18) == 0) {
        keyb_irq->vector = -1;
    } else if (dart_keyb_ctl[1] & 0x04) {
        /* Status affects vector */
        keyb_irq->vector = (dart_keyb_ctl[2] & ~0x0f) | 0x04;
    } else {
        /* Fixed vector */
        keyb_irq->vector = (dart_keyb_ctl[2] & ~0x01);
    }
}

/* Get the keyboard data, clearing the KEYB_NEW flag */
static unsigned int get_key(void)
{
    unsigned int rv, kbd;

    rv = kbd = keyb_data;
    cmpxchg(&keyb_data, &kbd, kbd & ~KEYB_NEW);

    return rv;
}

static int keyb_intack_fake(struct z80_irq* irq)
{
    unsigned int data = get_key();

    keyb_fakedata = (data & 0x7f) | ((data & KEYB_NEW) ? 0x80 : 0x00);

    return irq->vector;
}

static uint8_t dart_keyb_in(uint8_t port)
{
    uint8_t v, reg;

    switch (port & 1) {
    case 0: /* Data register */
        v = get_key();
        break;

    case 1: /* Control register */
        reg = dart_keyb_ctl[0] & 7;
        dart_keyb_ctl[0] &= ~7; /* Restore register 0 */

        switch (reg) {
        case 0:
            v = (keyb_data >> 8) + (1 << 2) + /* Transmit buffer empty */
                (dart_keyb_vsync << 4) +      /* RI -> vsync */
                (1 << 5);                     /* CTS -> 60 Hz */
            break;
        case 1:
            v = (1 << 0); /* All sent */
            break;
        case 2:
            v = dart_keyb_ctl[2];
            break;
        default:
            v = 0;
            break;
        }
        break;
    }

    return v;
}

static void abc802_out(uint8_t port, uint8_t value)
{
    port = abc800_mangle_port(port);

    switch (port) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
        abcbus_out(port, value);
        break;

    case 32:
    case 33:
        dart_pr_out(port, value);
        break;

    case 34:
    case 35:
        dart_keyb_out(port, value);
        break;

    case 54:
    case 55:
        abc806_rtc_out(port, value);
        break;

    case 56:
    case 57:
        crtc_out(port, value);
        break;

    case 66:
    case 67:
        abc800_sio_cas_out(port, value);
        break;

    case 96:
    case 97:
    case 98:
    case 99:
        abc800_ctc_out(port, value);
        break;

    default:
        break;
    }
}

static void (*do_out)(uint8_t, uint8_t);

void z80_out(int port, uint8_t value)
{
    if (tracing(TRACE_IO)) {
        fprintf(tracef,
                "OUT: port 0x%02x (%3d) sel 0x%02x (%2d) "
                "data 0x%02x (%3d) PC=%04x\n",
                port, port, abcbus_select & 0xff, abcbus_select, value, value,
                REG_PC);
    }

    do_out(port, value);
}

/*
 * This function is called from the z80 at an IN instruction.
 */
static uint8_t abcbus_in(uint8_t port)
{
    if (port == 7) {
        /* Reset all */
        abcbus_select = -1;
        disk_reset();
        printer_reset();
        return 0xff;
    }

    switch (abcbus_select) {
    case 36: /* HDx: */
    case 44: /* MFx: */
    case 45: /* MOx: */
    case 46: /* SFx: */
        return disk_in(abcbus_select, port);
        break;

    case 60: /* PRx: */
        return printer_in(abcbus_select, port);
        break;

    case 55: /* RTC */
        return rtc_in(abcbus_select, port);
        break;

    default:
        return 0xff;
        break;
    }
}

static uint8_t abc80_in(uint8_t port)
{
    uint8_t v = 0xff;

    port &= 0x17;

    switch (port) {
    case 0:
    case 1:
    case 7:
        v = abcbus_in(port);
        break;

    case 3:
        setmode40(1);
        break;

    case 4:
        setmode40(0);
        break;

    case (56 & 0x17):
        if (faketype) {
            v = keyb_fakedata;
            keyb_fakedata &= ~0x80;
        } else {
            unsigned int kbd = keyb_data;
            v = (kbd & 0x7f) | ((kbd & KEYB_DOWN) ? 0x80 : 0);
        }
        break;

    case (58 & 0x17):
        v = abc80_piob_in();
        break;

    default:
        break;
    }

    return v;
}

static uint8_t abc802_in(uint8_t port)
{
    uint8_t v = 0xff;

    port = abc800_mangle_port(port);

    switch (port) {
    case 0:
    case 1:
    case 2:
    case 7:
        v = abcbus_in(port);
        break;

    case 32:
    case 33:
        v = dart_pr_in(port);
        break;

    case 34:
    case 35:
        v = dart_keyb_in(port);
        break;

    case 54:
    case 55:
        v = abc806_rtc_in(port);
        break;

    case 56:
    case 57:
        v = crtc_in(port);
        break;

    case 66:
    case 67:
        v = abc800_sio_cas_in(port);
        break;

    case 96:
    case 97:
    case 98:
    case 99:
        v = abc800_ctc_in(port);
        break;

    default:
        break;
    }

    return v;
}

static uint8_t (*do_in)(uint8_t port);

int z80_in(int port)
{
    uint8_t sel, v;

    sel = abcbus_select;
    v = do_in(port);

    if (tracing(TRACE_IO)) {
        fprintf(tracef,
                " IN: port 0x%02x (%3d) sel 0x%02x (%2d) "
                "data 0x%02x (%3d) PC=%04x\n",
                port, port, sel, (int8_t)sel, v, v, REG_PC);
    }
    return v;
}

/* This is called in the event handler thread context! */
void keyboard_down(int sym)
{
    if (model == MODEL_ABC80) {
        if (sym & ~127)
            return;
    }

    keyb_data = sym | KEYB_NEW | KEYB_DOWN;
    z80_interrupt(keyb_irq);
}

unsigned int keyboard_up(void)
{
    unsigned int rv, kbd;

    rv = kbd = keyb_data;
    cmpxchg(&keyb_data, &kbd, kbd & ~KEYB_DOWN);

    return rv;
}

void io_init(void)
{
    switch (model) {
    case MODEL_ABC80:
        do_out = abc80_out;
        do_in = abc80_in;
        keyb_data = 0;
        abc80_cas_init();
        keyb_irq = faketype ? &keyb_irq_fake : &keyb_irq_80;
        break;
    case MODEL_ABC802:
        do_out = abc802_out;
        do_in = abc802_in;
        keyb_data = 0xff;
        abc800_cas_init();
        abc800_ctc_init();
        keyb_irq = &keyb_irq_800;
        break;
    }
    z80_register_irq(keyb_irq);
}
