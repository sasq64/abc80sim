#include "clock.h"
#include "abcio.h"
#include "compiler.h"
#include "nstime.h"
#include "screen.h"
#include "z80.h"
#include "z80irq.h"

static double ns_per_tstate = 1000.0 / 3.0; /* Nanoseconds per tstate (clock cycle) */
static double tstate_per_ns = 3.0 / 1000.0; /* Inverse of the above = freq in GHz */

static void abc80_clock_tick(void);
static void abc800_clock_tick(void);
static struct abctimer* ctc_timer[4];

static bool limit_speed;

/*
 * Initialize the time for next event
 */
struct abctimer
{
    uint64_t period; /* Period in ns */
    uint64_t last;   /* Last checkpoint in ns */
    uint64_t ltst;   /* TSTATE value for last checkpoint */
    void (*func)(void);
};

#define MAX_TIMERS 2
static struct abctimer timers[MAX_TIMERS];
static int ntimers;

#define MS(x) ((x)*INT64_C(1000000))

static struct abctimer* create_timer(uint64_t period, void (*func)(void))
{
    struct abctimer* t;

    if (ntimers >= MAX_TIMERS)
        abort();

    t = &timers[ntimers++];

    t->period = period;
    t->func = func;
    t->last = 0;
    t->ltst = 0;

    return t;
}

static unsigned int poll_tstate_period;
#define MAX_TSTATE_PERIOD 512

void timer_init(double mhz)
{
    if (mhz <= 0.001 || mhz >= 1.0e+6) {
        limit_speed = false;
    } else {
        limit_speed = true;
        ns_per_tstate = 1000.0 / mhz;
        tstate_per_ns = mhz / 1000.0;
    }
    nstime_init();

    /* Limit polling to once every μs simulated time */
    poll_tstate_period = 1000 * ns_per_tstate;
    if (!limit_speed || poll_tstate_period > MAX_TSTATE_PERIOD)
        poll_tstate_period = MAX_TSTATE_PERIOD;

    switch (model) {
    case MODEL_ABC80:
        /* 20 ms = 50 Hz */
        create_timer(MS(20), abc80_clock_tick);
        break;
    case MODEL_ABC802:
        /* 10.67 ms = 93.75 Hz */
        ctc_timer[3] = create_timer(10666667, abc800_clock_tick);

        /* 20 ms = 50 Hz */
        create_timer(MS(20), abc802_vsync);
        break;
    }
}

/* See if it is time to slow down a bit */
static void consider_napping(uint64_t now, uint64_t next)
{
    uint64_t when;
    int64_t ahead, behind;
    static uint64_t ref_time, ref_tstate;

    if (unlikely(now <= ref_time || TSTATE <= ref_tstate))
        goto weird;

    when = ref_time + (TSTATE - ref_tstate) * ns_per_tstate;
    behind = now - when;
    ahead = when - next;

    /* Sanity range check: 100 ms behind or 100 ms ahead of schedule */
    if (unlikely(behind >= MS(200) || ahead >= MS(100)))
        goto weird;

    /* If we are ahead of the next event, hold off and wait for it */
    if (ahead >= 0)
        mynssleep(next, now);
    return;

weird:
    /* We fell too far behind, got suspended, or the clock jumped... */
    ref_time = now;
    ref_tstate = TSTATE;
}

/* Poll for timers - these the only external event we look for */
volatile bool z80_quit;

bool z80_poll_external(void)
{
    uint64_t now;
    static uint64_t next = 0;
    int i;
    bool sleepy = limit_speed;
    static uint64_t next_check_tstate;

    if (z80_quit)
        return true; /* Terminate CPU loop */

    if (likely(TSTATE < next_check_tstate))
        return false;

    now = nstime();

    if (unlikely(now >= next)) {
        next = UINT64_MAX;

        for (i = 0; i < ntimers; i++) {
            struct abctimer* t = &timers[i];
            if (t->period) {
                uint64_t tnext = t->last + t->period;
                if (unlikely(now >= tnext)) {
                    t->last += t->period;
                    if (unlikely(now >= tnext)) {
                        /* Missed tick(s), advance to skip missed */
                        t->last = now - (now - t->last) % t->period;
                        tnext = t->last + t->period;
                    }
                    /* TSTATE value corresponding to t->last */
                    t->ltst = TSTATE - (now - t->last) * tstate_per_ns;
                    t->func();
                    sleepy =
                        false; /* Just in case "now" is too far behind now */
                }
                if (next > tnext)
                    next =
                        tnext; /* The next event is closer than you thought */
            }
        }
    }

    next_check_tstate = TSTATE + poll_tstate_period;
    if (limit_speed) {
        uint64_t next_ev = TSTATE + (next - now) * tstate_per_ns;
        if (next_ev < next_check_tstate)
            next_check_tstate = next_ev;
    }

    if (sleepy)
        consider_napping(now, next);

    return false;
}

/*
 * ABC80: Trig a non maskable interrupt in the Z80 on the clock signal.
 */
static void abc80_clock_tick(void)
{
    vsync_screen(); /* Also vertical retrace */
    z80_nmi();
}

/*
 * ABC800: Clock interrupt through the CTC
 */
static uint8_t ctc_ctl[4], ctc_div[4];
static struct z80_irq ctc_irq[4] = {
    IRQ(IRQ800_CTC0, NULL, NULL, NULL),
    IRQ(IRQ800_CTC1, NULL, NULL, NULL),
    IRQ(IRQ800_CTC2, NULL, NULL, NULL),
    IRQ(IRQ800_CTC3, NULL, NULL, NULL),
};

static void abc800_clock_tick(void)
{
    if ((ctc_ctl[3] & 0xc0) == 0x80)
        z80_interrupt(&ctc_irq[3]);
}

/*
 * CTC I/O
 */
void abc800_ctc_out(uint8_t port, uint8_t v)
{
    port &= 3; /* Get channel */

    if (ctc_ctl[port] & 4) {
        ctc_div[port] = v;
        ctc_ctl[port] &= ~4;
        return;
    }

    if ((v & 1) == 0) {
        int i;
        v &= ~7;
        for (i = 0; i <= 3; i++)
            ctc_irq[i].vector = v | (i << 1);
        return;
    }

    if (v & 2)
        v = 1; /* Reset channel */

    ctc_ctl[port] = v;
}

uint8_t abc800_ctc_in(uint8_t port)
{
    uint8_t v, div;
    const struct abctimer* t;

    port &= 3;
    t = ctc_timer[port];
    div = ctc_div[port];

    if (!t)
        return -1;

    if (limit_speed) {
        /* Interpolate based on TSTATEs (virtual time) */
        v = ((int64_t)t->period -
             (int64_t)(((TSTATE - t->ltst) * ns_per_tstate)) * div) /
            t->period;
    } else {
        /* Interpolate based on nanoseconds (real time) */
        v = ((t->period - (nstime() - t->last)) * div) / t->period;
    }

    return v;
}

void abc800_ctc_init(void)
{
    int i;
    for (i = 0; i < 4; i++)
        z80_register_irq(&ctc_irq[i]);
}
