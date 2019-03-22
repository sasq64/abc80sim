/* ----------------------------------------------------------------------- *
 *
 *   Copyright 2004-2018 H. Peter Anvin - All Rights Reserved
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, Inc., 53 Temple Place Ste 330,
 *   Bostom MA 02111-1307, USA; either version 2 of the License, or
 *   (at your option) any later version; incorporated herein by reference.
 *
 * ----------------------------------------------------------------------- */

/*
 * print.c
 *
 * abcprintd backend - also usable for abc80sim
 *
 */

#include "abcprintd.h"
#include "compiler.h"
#include "hostfile.h"

#include <locale.h>
#include <wchar.h>

#ifdef __WIN32__
const char* lpr_command =
    "powershell -command \"GetContent -raw -path '*' | OutPrinter\"";
#else
const char* lpr_command = "lpr '*'";
#endif

static struct host_file* hf;

static void print_finish(void)
{
    const char* p;
    char *cmd, *q;
    size_t cmdlen, namelen;

    if (!hf)
        return;

    fflush(hf->f);

    namelen = hf->namelen;
    cmdlen = 0;
    for (p = lpr_command; *p; p++) {
        cmdlen += (*p == '*') ? namelen : 1;
    }
    cmd = malloc(cmdlen + 1);

    if (cmd) {
        for (p = lpr_command, q = cmd; *p; p++) {
            if (*p == '*') {
                memcpy(q, hf->filename, namelen);
                q += namelen;
            } else {
                *q++ = *p;
            }
        }
        *q = '\0';

        system(cmd);
        free(cmd);
    }
    close_file(&hf);
}

static void output(unsigned char c)
{
    static const char temp_prefix[] = "abcprint_tmp_";

    static const wchar_t abc_to_unicode[256] =
        L"\000\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017"
        L"\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037"
        L" !\"#¤%&\'()*+,-./0123456789:;<=>?"
        L"ÉABCDEFGHIJKLMNOPQRSTUVWXYZÄÖÅÜ_"
        L"éabcdefghijklmnopqrstuvwxyzäöåü\x25a0"
        L"\x20ac\x25a1\x201a\x0192\x201e\x2026\x2020\x2021"
        L"\x02c6\x2030\x0160\x2039\x0152\x2190\x017d\x2192"
        L"\x2191\x2018\x2019\x201c\x201d\x2022\x2013\x2014"
        L"\x02dc\x2122\x0161\x203a\x0153\x2193\x017e\x0178"
        L"\240\241\242\243$\245\246\247\250\251\252\253\254\255\256\257"
        L"\260\261\262\263\264\265\266\267\270\271\272\273\274\275\276\277"
        L"\300\301\302\303[]\306\307\310@\312\313\314\315\316\317"
        L"\320\321\322\323\324\325\\\327\330\331\332\333^\335\336\337"
        L"\340\341\342\343{}\346\347\350`\352\353\354\355\356\357"
        L"\360\361\362\363\364\365|\367\370\371\372\373~\375\376\377";

    if (!hf) {
        if (c < '\b' || (c > '\r' && c < 31))
            hf = temp_file(HF_BINARY, temp_prefix);
        else
            hf = temp_file(HF_UNICODE, temp_prefix);
    }

    if (hf->mode == HF_BINARY)
        putc(c, hf->f);
    else if (c != '\r')
        putwc(abc_to_unicode[c], hf->f);
}

enum input_state
{
    is_normal, /* Normal operation */
    is_ff,     /* 0xFF received */
    is_file,   /* File operation in progress */
    is_console /* Output to console */
};
static enum input_state is;
FILE* console_file;

void abcprint_init(void)
{
    is = is_normal;
}

void abcprint_recv(const void* data, size_t len)
{
    const unsigned char* dp = data;
    unsigned char c;

    while (len--) {
        c = *dp++;

        switch (is) {
        case is_normal:
            if (c == 0xff)
                is = is_ff;
            else
                output(c);
            break;

        case is_ff:
            if (c == 0) {
                /* End of job */
                print_finish();
                is = is_normal;
            } else if (c >= 0xa0 && c <= 0xbf) {
                /* Opcode range reserved for file ops */
                is = file_op(c) ? is_file : is_normal;
            } else if (c == 0xc0) {
                /* Output to console */
                is = is_console;
            } else {
                output(c);
                is = is_normal;
            }
            break;

        case is_file:
            is = file_op(c) ? is_file : is_normal;
            break;

        case is_console:
            if (c == 0) {
                is = is_normal;
                if (console_file)
                    fflush(console_file);
            } else if (console_file) {
                fputc(c, console_file);
            }
            break;
        }
    }
}
