#ifndef NSTIME_H
#define NSTIME_H

#include "compiler.h"

extern uint64_t nstime(void);
extern void nstime_init(void);
extern void mynssleep(uint64_t until, uint64_t since);

#endif /* NSTIME_H */
