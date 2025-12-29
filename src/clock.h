#ifndef CLOCK_H
#define CLOCK_H

#include <stdbool.h>

#define CLOCKSPEED_1x 3.0 /* Mhz */
#define CLOCKSPEED_10x 30.0 /* MHz */
#define CLOCKSPEED_UNLIMITED 1000.0 /* MHz */


/** Initialize timer module */
extern void timer_init(double mhz);
/** Cleanup timer module */
extern void timer_destroy(void);
/** Change clock speed */
extern void timer_set_speed(double mhz);

extern void timer_poll(void);
extern void vsync_screen(void);

extern volatile bool z80_quit;

#endif /* CLOCK_H */
