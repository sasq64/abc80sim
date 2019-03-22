#include "trace.h"
#include "compiler.h"

void trace_dump_data(const char* prefix, const void* data, unsigned int l)
{
    unsigned int i, j;
    const uint8_t* p = data;

    for (i = 0; i < l; i += 16) {
        fprintf(tracef, "%s: %04x : ", prefix, i);

        for (j = 0; j < 16; j++) {
            if (i + j < l)
                fprintf(tracef, " %02x", p[j]);
            else
                fputs("   ", tracef);

            if (j == 8)
                fputs(" -", tracef);
        }

        fputs(" [", tracef);

        for (j = 0; j < 16; j++) {
            char c;

            if (i + j < l)
                c = p[j];
            else
                c = ' ';

            if (c < ' ' || c > '~')
                c = '.';

            fputc(c, tracef);
        }

        fputs("]\n", tracef);
        p += 16;
    }
}
