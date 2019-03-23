#ifndef CLOCK_H
#define CLOCK_H

#include <stdbool.h>

extern void timer_init(double mhz);
extern void timer_poll(void);
extern void vsync_screen(void);

extern volatile bool z80_quit;

#endif /* CLOCK_H */
