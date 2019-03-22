#include "abcfile.h"
#include "abcprintd.h"
#include "hostfile.h"
#include "trace.h"

const char* fileop_path = "abcdir";

#define BUF_SIZE 512

static enum {
    st_op, /* Receiving command */
    st_open,
    st_read,
    st_print,  /* Before data */
    st_print2, /* After data */
    st_seek,
    st_rename,
    st_delete,
    st_pread,
    st_pwrite,  /* Before data */
    st_pwrite2, /* After data */
    st_blksize
} state = st_op;

struct file
{
    struct host_file* hf;
    bool binary;
};
static struct file filemap[65536]; /* Massive waste of space, yes... */

static unsigned int blksize; /* System block size */

static inline bool file_open(uint16_t ix)
{
    return !!filemap[ix].hf;
}

static inline bool file_binary(uint16_t ix)
{
    return filemap[ix].binary;
}

static inline enum host_file_mode file_mode(uint16_t ix)
{
    return filemap[ix].hf->mode;
}

static unsigned int byte_count = 4;
static unsigned char cmd[4];
static unsigned char* bytep = cmd;
static unsigned char data[65536 + 2];

static void trace_data(const void* data, size_t len, const char* pfx)
{
    size_t i;
    const uint8_t* dp = data;

    if (!tracing(TRACE_PR))
        return;

    fprintf(tracef, "PR:  %-5s: ", pfx);

    for (i = 0; i < 16; i++) {
        if (i >= len)
            fprintf(tracef, "  ");
        else
            fprintf(tracef, "%02x", dp[i]);

        putc(i == 8 ? '-' : ' ', tracef);
    }

    fprintf(tracef, "%c  [", (len > 16) ? '+' : ' ');

    for (i = 0; i < 16; i++) {
        char c;

        c = (i >= len) ? ' ' : dp[i];
        if (c < 32 || c > 126)
            c = '.';

        putc(c, tracef);
    }
    putc(']', tracef);
    if (len > 16)
        fprintf(tracef, "+ (%lu bytes)", (unsigned long)len);
    putc('\n', tracef);
}

static void pr_send(const void* buf, size_t len)
{
    trace_data(buf, len, "SEND");
    abcprint_send(buf, len);
}

static void send_reply(int status)
{
    unsigned char reply[4];

    reply[0] = 0xff;
    reply[1] = cmd[0];
    reply[2] = cmd[1];
    reply[3] = status;

    pr_send(reply, 4);
}

/* Returns the status code, use send_reply(do_close(ix)) if reply desired */
static int do_close(uint16_t ix)
{
    if (file_open(ix)) {
        close_file(&filemap[ix].hf);
        return 0;
    } else {
        return 128 + 45; /* "Fel logiskt filnummer" */
    }
}

static void do_closeall(bool reply)
{
    int ix;

    for (ix = 0; ix <= 65535; ix++)
        if (file_open(ix))
            do_close(ix);

    if (reply)
        send_reply(0);
}

static void do_open(uint16_t ix, char* name)
{
    int err;
    char path_buf[64];
    int openflags;
    enum host_file_mode mode;
    struct host_file* hf;

    if (!fileop_path) {
        send_reply(128 + 42); /* Skivan ej klar */
        return;
    }

    do_close(ix);

    unmangle_filename(path_buf, name);

    if (!path_buf[0]) {
        /* Empty filename (readdir) */

        mode = ((cmd[0] & 3) == 0) ? HF_DIRECTORY : HF_FAIL;
        openflags = 0;
    } else {
        /* Actual filename */

        mode = HF_BINARY;
        mode |= (cmd[0] & 2) ? 0 : HF_RETRY;
        openflags = (cmd[0] & 2) ? (O_RDWR | O_TRUNC | O_CREAT) : O_RDWR;
    }

    hf = open_host_file(mode, fileop_path, path_buf, openflags);
    filemap[ix].hf = hf;
    filemap[ix].binary = cmd[0] & 1;

    switch (errno) {
#if 0 /* Enable this? */
  case EACCES:
      err = 128+39;
      break;
  case EROFS:
    err = 128+43;
    break;
  case EIO:
  case ENOTDIR:
    err = 128+48;
    break;
#endif
    default:
        err = 128 + 21;
        break;
    }

    send_reply(hf ? 0 : err);
}

static void do_read_block(uint16_t ix, uint16_t len)
{
    struct host_file* hf = filemap[ix].hf;
    int err;
    int dlen;

    if (!file_open(ix)) {
        send_reply(128 + 45);
        return;
    }
    if (file_mode(ix) == HF_DIRECTORY) {
        send_reply(128 + 37); /* Felaktigt recordformat */
        return;
    }

    clearerr(hf->f);
    dlen = fread(data + 2, 1, len, hf->f);
    if (dlen == 0) {
        if (ferror(hf->f)) {
            switch (errno) {
            case EBADF:
                err = 128 + 44; /* Logisk fil ej öppen */
                break;
            case EIO:
                err = 128 + 35; /* Checksummafel vid läsning */
                break;
            default:
                err = 128 + 48; /* Fel i biblioteket */
                break;
            }
        } else {
            /* EOF */
            err = 128 + 34; /* Slut på filen */
        }
        send_reply(err);
        return;
    }

    send_reply(0);

    data[0] = len;
    data[1] = len >> 8;
    pr_send(data, len + 2);
}

/* Common routine for all commands which need seek */
static int seeker(uint16_t ix, uint64_t pos)
{
    struct host_file* hf = filemap[ix].hf;
    int err;

    if (!file_open(ix)) {
        err = 128 + 45; /* Fel logiskt filnummer */
    } else if (file_mode(ix) == HF_DIRECTORY) {
        err = 128 + 37; /* Felaktigt recordformat */
    } else if (fseek(hf->f, pos, SEEK_SET) == -1) {
        err = 128 + 38; /* Recordnummer utanför filen */
    } else {
        err = 0;
    }

    return err;
}

static void do_seek(uint16_t ix, uint64_t pos)
{
    send_reply(seeker(ix, pos));
}

static void do_pread(uint16_t ix, uint16_t blk)
{
    int err;

    err = seeker(ix, blksize * (long)blk);
    if (err)
        send_reply(err);
    else
        do_read_block(ix, blksize);
}

static void do_input(uint16_t ix)
{
    struct host_file* hf = filemap[ix].hf;
    int err;
    char data1[255 + 2]; /* Max number of bytes to return + 2 */
    char *p, *q, c;
    int dlen;
    struct dirent* de;
    struct stat st;

    if (!file_open(ix)) {
        send_reply(128 + 45);
        return;
    }

    if (file_mode(ix) != HF_DIRECTORY) {
        clearerr(hf->f);
        if (!fgets((char*)data, sizeof data, hf->f)) {
            if (ferror(hf->f)) {
                switch (errno) {
                case EBADF:
                    err = 128 + 44; /* Logisk fil ej öppen */
                    break;
                case EIO:
                    err = 128 + 35; /* Checksummafel vid läsning */
                    break;
                default:
                    err = 128 + 48; /* Fel i biblioteket */
                    break;
                }
            } else {
                /* EOF */
                err = 128 + 34; /* Slut på filen */
            }
        } else {
            /* Strip CR and change LF -> CR LF */
            if (!file_binary(ix)) {
                for (p = (char*)data, q = data1 + 2; (c = *p); p++) {
                    if (q == &data1[sizeof data1])
                        break;
                    switch (c) {
                    case '\r':
                        break;
                    case '\n':
                        *q++ = '\r';
                        /* fall through */
                    default:
                        *q++ = c;
                        break;
                    }
                }
                dlen = q - (data1 + 2);
            } else {
                dlen = strnlen(data1 + 2, sizeof data1 - 2);
            }
            err = 0;
        }
    } else if (hf->d) {
        while ((de = readdir(hf->d))) {
            if (de->d_name[0] != '.' &&
                (dlen = mangle_for_readdir(data1 + 2, de->d_name))) {
                if (!stat_file(fileop_path, de->d_name, &st) &&
                    S_ISREG(st.st_mode))
                    break;
            }
        }
        if (de) {
            unsigned long blocks, pad;
            blocks = (st.st_size + blksize - 1) / blksize;
            pad = blksize * blocks - st.st_size;
            /* pad = unused bytes in the last block */
            dlen += sprintf(data1 + 2 + dlen, ",%lu,%lu\r\n", blocks, pad);
            err = 0;
        } else {
            err = 128 + 34;
        }
    } else {
        err = 128 + 44;
    }

    send_reply(err);
    if (!err) {
        data1[0] = dlen;
        data1[1] = dlen >> 8;
        pr_send(data1, dlen + 2);
    }
}

static void do_print(uint16_t ix, uint16_t len, bool eolcvt)
{
    struct host_file* hf = filemap[ix].hf;
    int err;

#ifdef __WIN32__
    /* ABC sends CR LF as line endings, which matches Windows */
    eolcvt = false;
#endif

    if (!file_open(ix)) {
        err = 128 + 45;
    } else if (file_mode(ix) == HF_DIRECTORY) {
        err = 128 + 39; /* Directories are readonly */
    } else {
        clearerr(hf->f);
        if (len) {
            if (eolcvt && !file_binary(ix)) {
                int i;
                for (i = 0; i < len - 1; i++) {
                    char c = data[i];
                    if (c == '\r')
                        continue; /* CR LF -> LF */
                    else
                        putc(c, hf->f);
                }
                putc(data[len - 1], hf->f); /* Last char is always verbatim */
            } else {
                fwrite(data, 1, len, hf->f);
            }
            fflush(hf->f);
        }

        if (!ferror(hf->f)) {
            err = 0;
        } else {
            switch (errno) {
            case EACCES:
                err = 128 + 39; /* Filen skrivskyddad */
                break;
            case ENOSPC:
            case EFBIG:
                err = 128 + 41; /* Skivan full */
                break;
            case EROFS:
                err = 128 + 43; /* Skivan skrivskyddad */
                break;
            case EBADF:
                err = 128 + 44; /* Logisk fil ej öppen */
                break;
            case EIO:
                err = 128 + 36; /* Checksummafel vid skrivning */
                break;
            default:
                err = 128 + 48; /* Fel i biblioteket */
                break;
            }
        }
    }
    send_reply(err);
}

static void do_pwrite(uint16_t ix, uint16_t blk)
{
    int err;

    err = seeker(ix, blksize * (long)blk);
    if (err)
        send_reply(err);
    else
        do_print(ix, blksize, false);
}

static void do_rename(const char* files)
{
    char old_name[64], new_name[64];
    int err;

    unmangle_filename(old_name, files);
    unmangle_filename(new_name, files + 11);

    if (!rename(old_name, new_name)) {
        err = 0;
    } else {
        switch (errno) {
        case EACCES:
            err = 128 + 39; /* Filen skrivskyddad */
            break;
        case EROFS:
            err = 128 + 43; /* Skivan skrivskyddad */
            break;
        case ENOENT:
            err = 128 + 21; /* Hittar ej filen */
            break;
        case ENOSPC:
            err = 128 + 41; /* Skivan full */
            break;
        case EISDIR:
            err = 128 + 40; /* Filen raderingsskyddad */
            break;
        case EIO:
            err = 128 + 36; /* Checksummafel vid skrivning */
            break;
        default:
            err = 128 + 48; /* Fel i biblioteket */
            break;
        }
    }

    send_reply(err);
}

static void do_delete(const char* file)
{
    char path_buf[64];
    int err;

    unmangle_filename(path_buf, file);

    if (!remove(path_buf)) {
        err = 0;
    } else {
        switch (errno) {
        case EROFS:
            err = 128 + 43; /* Skivan skrivskyddad */
            break;
        case ENOENT:
            err = 128 + 21; /* Hittar ej filen */
            break;
        case ENOSPC:
            err = 128 + 41; /* Skivan full */
            break;
        case EISDIR:
        case EPERM:
        case EACCES:
            err = 128 + 40; /* Filen raderingsskyddad */
            break;
        case EIO:
            err = 128 + 36; /* Checksummafel vid skrivning */
            break;
        default:
            err = 128 + 48; /* Fel i biblioteket */
            break;
        }
    }

    send_reply(err);
}

typedef union argbuf
{
    uint8_t b[32];
    char c[32];
    uint64_t q;
} argbuf;

static inline uint64_t get_qword(const argbuf* v)
{
#ifdef WORDS_LITTLEENDIAN
    return v->q;
#else
    return v->b[0] + ((uint64_t)v->b[1] << 8) + ((uint64_t)v->b[2] << 16) +
           ((uint64_t)v->b[3] << 24) + ((uint64_t)v->b[4] << 32) +
           ((uint64_t)v->b[5] << 40) + ((uint64_t)v->b[6] << 48) +
           ((uint64_t)v->b[7] << 56);
#endif
}

bool file_op(unsigned char c)
{
    static argbuf argbuf;
    static unsigned int datalen = 0;
    uint16_t ix;
    uint64_t arg;

    *bytep++ = c;
    if (--byte_count)
        return true; /* More to do... */

    /* Otherwise, we have a full deck of *something* */
    ix = (cmd[3] << 8) + cmd[2];
    arg = get_qword(&argbuf);
    bytep = argbuf.b;

    switch (state) {
    case st_op:
        switch (cmd[0]) {
        case 0xA0: /* OPEN TEXT */
        case 0xA1: /* OPEN BINARY */
        case 0xA2: /* PREPARE TEXT */
        case 0xA3: /* PREPARE BINARY */
            byte_count = 11;
            state = st_open;
            break;

        case 0xA4: /* INPUT */
            do_input(ix);
            break;

        case 0xA5: /* READ BLOCK */
            byte_count = 2;
            state = st_read;
            break;

        case 0xA6: /* PRINT */
        case 0xB9: /* PUT */
            byte_count = 2;
            state = st_print;
            break;

        case 0xA7: /* CLOSE */
            send_reply(do_close(ix));
            break;

        case 0xA8: /* CLOSEALL */
            do_closeall(true);
            break;

        case 0xA9: /* INIT */
            blksize = 253;
            do_closeall(false);
            break;

        case 0xAE: /* SET BLOCK SIZE */
        case 0xAF: /* INITSZ */
            byte_count = 2;
            state = st_blksize;
            break;

        case 0xAA: /* RENAME */
            byte_count = 22;
            state = st_rename;
            break;

        case 0xAB: /* DELETE */
            byte_count = 11;
            state = st_delete;
            break;

        case 0xAC: /* PREAD */
            byte_count = 2;
            state = st_pread;
            break;

        case 0xAD: /* PWRITE */
            byte_count = 2;
            state = st_pwrite;
            break;

        case 0xB0: /* SEEK0 == REWIND */
            do_seek(ix, 0);
            break;

        case 0xB1: /* SEEK1 */
        case 0xB2: /* SEEK2 */
        case 0xB3: /* SEEK3 */
        case 0xB4: /* SEEK4 */
        case 0xB5: /* SEEK5 */
        case 0xB6: /* SEEK6 */
        case 0xB7: /* SEEK7 */
        case 0xB8: /* SEEK8 */
            byte_count = cmd[0] - 0xb0;
            state = st_seek;
            break;

        default:
            /* Unknown command */
            send_reply(128 + 11);
            break;
        }
        if (tracing(TRACE_PR)) {
            static const char* const cmdnames[0x20] = {
                "OPEN A", "OPEN B", "PREP A", "PREP B", "INPUT",  "GET",
                "PRINT",  "CLOSE",  "CALL",   "CALLNR", "RENAME", "DELETE",
                "PREAD",  "PWRITE", "BLKSIZ", "INITSZ", "REWIND", "SEEK1",
                "SEEK2",  "SEEK3",  "SEEK4",  "SEEK5",  "SEEK6",  "SEEK7",
                "SEEK8",  "PUT",    NULL,     NULL,     NULL,     NULL,
                NULL,     NULL};
            int cnum = cmd[0] - 0xa0;
            const char* cmdname = (cnum >= 0x20) ? NULL : cmdnames[cnum];

            fprintf(tracef, "PR:  CMD  : %-6s %02x %02x %04x <need %u bytes>\n",
                    cmdname ? cmdname : "???", cmd[0], cmd[1], ix, byte_count);
        }
        break;

    case st_open:
        trace_data(argbuf.b, 11, "OPEN");
        do_open(ix, argbuf.c);
        break;

    case st_read:
        trace_data(argbuf.b, 2, "READ");
        do_read_block(ix, arg);
        break;

    case st_print:
        trace_data(argbuf.b, 2, "WRTE");
        bytep = data;
        byte_count = arg;
        state = st_print2;
        break;

    case st_print2:
        trace_data(data, datalen, "DATA");
        do_print(ix, datalen, cmd[0] == 0xA6);
        break;

    case st_pwrite:
        trace_data(argbuf.b, 2, "PWRT");
        bytep = data;
        byte_count = 253;
        state = st_pwrite2;
        break;

    case st_pwrite2:
        trace_data(data, datalen, "DATA");
        do_pwrite(ix, arg);
        break;

    case st_pread:
        trace_data(argbuf.b, 2, "PRED");
        do_pread(ix, arg);
        break;

    case st_seek:
        trace_data(argbuf.b, cmd[0] - 0xb0, "SEEK");
        do_seek(ix, arg);
        break;

    case st_rename:
        trace_data(data, 11, "REN1");
        trace_data(data + 11, 11, "REN2");
        do_rename(argbuf.c);
        break;

    case st_delete:
        trace_data(data, 11, "DEL ");
        do_delete(argbuf.c);
        break;

    case st_blksize:
        trace_data(data, 2, "SIZE");
        blksize = arg;
        if (cmd[0] == 0xAF)
            do_closeall(false);
        break;
    }

    datalen = byte_count;
    if (bytep == argbuf.b)
        memset(&argbuf, 0, sizeof argbuf);

    if (byte_count) {
        return true;
    } else {
        /* back to command mode */
        bytep = cmd;
        byte_count = 4;
        state = st_op;
        return false;
    }
}
