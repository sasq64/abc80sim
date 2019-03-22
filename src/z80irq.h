#ifndef Z80IRQ_H
#define Z80IRQ_H

#include "compiler.h"
#include "z80.h"

typedef int (*irq_func)(struct z80_irq* irq);

struct z80_irq
{
    irq_func intack;
    irq_func eoi;
    void* pvt;      /* Available for user */
    int16_t vector; /* Available for user if intack defined */
    uint8_t prio;   /* Priority level */
    bool handled;   /* In handler (between INTACK and EOI) */
};

#define IRQ(prio, intack, eoi, pvt)                                            \
    {                                                                          \
        (intack), (eoi), (pvt), -1, (prio), false                              \
    }

#define MAX_IRQ 32

extern volatile unsigned int irq_pending;
extern unsigned int irq_mask; /* 0 = inside handler (irq->handled == true) */

static inline bool poll_irq(void)
{
    return unlikely(irq_pending & irq_mask);
}

void z80_register_irq(struct z80_irq* irq);
int z80_intack(void);
void z80_eoi(void);
static inline void z80_interrupt(struct z80_irq* irq)
{
    atomic_set_bit(&irq_pending, irq->prio);
}
static inline void z80_clear_interrupt(struct z80_irq* irq)
{
    atomic_clear_bit(&irq_pending, irq->prio);
}

#endif /* Z80IRQ_H */
