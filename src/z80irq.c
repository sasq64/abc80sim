#include "z80irq.h"
#include "compiler.h"
#include "z80.h"

volatile unsigned int irq_pending; /* Quick way to poll */
unsigned int irq_mask = ~0U;
static struct z80_irq* irqs[MAX_IRQ];

void z80_register_irq(struct z80_irq* irq)
{
    unsigned int prio = irq->prio;

    assert(prio < MAX_IRQ);
    assert(!irqs[prio]);
    irqs[irq->prio] = irq;
}

/*
 * Z80 interrupt acknowledge cycle. Return the vector from the highest
 * priority pending interrupt, or -1 if spurious.
 */
int z80_intack(void)
{
    unsigned int prio;
    int vector = -1;
    unsigned int irqpend, irqmasked;
    struct z80_irq* irq;

    do {
        /* Find the highest priority (lowest numeric) interrupt pending */
        irqpend = irq_pending;
        irqmasked = irqpend & irq_mask;

        if (unlikely(!irqmasked))
            return vector; /* All interrupts went away... */

        prio = __builtin_ctz(irqmasked);
        if (unlikely(!atomic_test_clear_bit(&irq_pending, prio)))
            continue; /* This particular interrupt went away on us? */

        irq = irqs[prio];

        if (unlikely(irq->intack))
            vector = irq->intack(irq);
        else
            vector = irq->vector;
    } while (unlikely(vector < 0));

    /* Inside the handler for this interrupt */
    irq_mask &= ~(1U << prio);
    irq->handled = true;

    return vector;
}

/*
 * A RETI instruction was invoked, which is interpreted as an EOI.
 * In a real Z80 this is done by snooping the bus.
 * The priority chain is again used, so the EOI is directed to the
 * highest priority interrupt which is currently under service.
 */
void z80_eoi(void)
{
    unsigned int nirqmask, prio;
    struct z80_irq* irq;

    nirqmask = ~irq_mask;
    if (!nirqmask)
        return; /* No interrupts pending... */

    prio = __builtin_ctz(nirqmask);
    irq = irqs[prio];
    irq->handled = false;
    irq_mask |= 1U << prio;
    if (irq->eoi)
        irq->eoi(irq);
}
