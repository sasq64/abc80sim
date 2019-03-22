#ifndef TRACE_H
#define TRACE_H

#include "compiler.h"

extern FILE* tracef;

enum tracing
{
    TRACE_NONE = 0x00,
    TRACE_CPU = 0x01,
    TRACE_IO = 0x02,
    TRACE_DISK = 0x04,
    TRACE_CAS = 0x08,
    TRACE_PR = 0x10,
    TRACE_ALL = 0x1f
};

extern int traceflags;

static inline bool tracing(enum tracing flags)
{
    return unlikely(traceflags & flags);
}

extern void trace_dump_data(const char* prefix, const void* data,
                            unsigned int l);

static inline void trace_dump(enum tracing flags, const char* prefix,
                              const void* data, size_t l)
{
    if (tracing(flags))
        trace_dump_data(prefix, data, l);
}

#endif /* TRACE_H */
