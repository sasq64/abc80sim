#include "abcio.h"
#include "abcprintd.h"
#include "z80irq.h"

#define BUF_SIZE 512

static unsigned char output_buf[BUF_SIZE];
static int output_head, output_tail;

/* Called to send data abcprint -> abc */
void abcprint_send(const void* buf, size_t count)
{
    const unsigned char* bp = buf;

    while (count--) {
        int nt = (output_tail + 1) % BUF_SIZE;

        if (nt == output_head)
            return; /* Output buffer full - data lost */

        output_buf[output_tail] = *bp++;
        output_tail = nt;
    }
}

static int abcprint_read(void)
{
    int c;

    if (output_head == output_tail) {
        return -1;
    } else {
        c = output_buf[output_head];
        output_head = (output_head + 1) % BUF_SIZE;
        return c;
    }
}

static bool abcprint_poll(void)
{
    return output_head != output_tail;
}

static struct z80_irq dart_pr_irq;

void printer_reset(void)
{
    static bool init = false;

    if (!init) {
        init = true;
        abcprint_init();
        if (model != MODEL_ABC80)
            z80_register_irq(&dart_pr_irq);
    }
}

void printer_out(int sel, int port, int value)
{
    unsigned char v = value;

    (void)sel;

    switch (port) {
    case 0:
        abcprint_recv(&v, 1); /* Data received abc -> abcprint */
        break;

    case 4:
        abcprint_init();
        break;

    default:
        break;
    }
}

int printer_in(int sel, int port)
{
    int v;

    (void)sel;

    switch (port) {
    case 0:
        v = abcprint_read();
        break;

    case 1:
        v = abcprint_poll() ? 0x40 : 0;
        break;

    default:
        v = -1;
        break;
    }

    return v;
}

/* Hardware-like interface via the ABC800 PR: port */
static uint8_t dart_pr_ctl[8];

static struct z80_irq dart_pr_irq = IRQ(IRQ800_DARTA, NULL, NULL, NULL);

void dart_pr_out(uint8_t port, uint8_t v)
{
    uint8_t r;

    switch (port & 1) {
    case 0: /* Data port */
        if (dart_pr_ctl[5] & 0x08)
            abcprint_recv(&v, 1);
        break;

    case 1: /* Control port */
        r = dart_pr_ctl[0] & 7;
        dart_pr_ctl[0] &= ~7;
        dart_pr_ctl[r] = v;
        /* Should to things like supporting interrupts here */
        break;
    }
}

uint8_t dart_pr_in(uint8_t port)
{
    uint8_t r, v = 0;

    switch (port & 1) {
    case 0: /* Data port */
        if (dart_pr_ctl[3] & 1)
            v = abcprint_read();
        break;

    case 1:
        r = dart_pr_ctl[0] & 7;
        dart_pr_ctl[0] &= ~7;

        switch (r) {
        case 0: /* RR0 primary status */
            /*
             * 7 - 0 - No break
             * 6 - 0 - No transmit underrun
             * 5 - 1 - CTS# asserted
             * 4 - x - RI# asserted if 80 columns on boot
             * 3 - 1 - DCD# asserted
             * 2 - 1 - Transmit buffer empty
             * 1 - 0 - Interrupt not pending
             * 0 - x - Receive character available
             */
            v = 0x2c | (!startup_width40 << 4) |
                (dart_pr_ctl[3] & abcprint_poll());
            break;

        case 1: /* RR1 Rx special modes */
            /*
             * 5 - 0 - No receiver overrun
             * 4 - 0 - No parity error
             * 0 - 1 - All sent
             */
            v = 0x01;
            break;

        default:
            break;
        }
    }

    return v;
}
