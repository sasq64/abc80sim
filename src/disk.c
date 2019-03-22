/*
 * ABC80 simulated disk
 */

#include "abcio.h"
#include "compiler.h"
#include "hostfile.h"
#include "trace.h"
#include "z80.h"

const char* disk_path = "abcdisk";

#define NOTTHERE 0
#define READONLY 0
#define INTERLEAVE 0

/* This is the interpretation of an "out" command */
enum out_state
{
    disk_need_init,
    disk_k0,
    disk_k1,
    disk_k2,
    disk_k3,
    disk_upload,
    disk_download
};

struct ctl_state
{
    enum out_state state;
    uint8_t k[4];
    unsigned int secperclust;
    unsigned int sectors;
    uint8_t ilmsk, ilfac; /* Interlacing parameters */
    uint8_t new;          /* "New addressing" */
    const char name[3];
    int out_ptr;                /* Pointer within buffer for out data */
    int in_ptr;                 /* Pointer within buffer for in data */
    int status;                 /* Primary status */
    int aux_status;             /* Auxilliary status */
    int notready_ctr;           /* How many times are we not ready? */
    struct host_file* files[8]; /* File for this unit */
    unsigned char buf[4][256];  /* 4 buffers @ 256 bytes */
};

// clang-format off
static struct ctl_state mo_state =
  {
    .secperclust = 1,
    .sectors     = 40*1*16,
#if INTERLEAVE
    .ilmsk       = 15,
    .ilfac       = 7,
#endif
    .name        = "mo"
  };
static struct ctl_state mf_state =
  {
    .secperclust = 4,
    .sectors     = 80*2*16,
    .name        = "mf"
  };
static struct ctl_state sf_state =
  {
    .secperclust = 4,
    .sectors     = (77*2-1)*26,	/* Spår 0, sida 0 används ej */
    .name        = "sf"
  };
static struct ctl_state hd_state =
  {
    .secperclust = 32,
    .new         = 1,		/* Actually irrelevant when secperclust = 32 */
    .sectors     = 238*8*32,
    .name        = "hd"
  };
// clang-format on

static struct ctl_state* const sel_to_state[64] = {
    [36] = &hd_state,
    [44] = &mf_state,
    [45] = &mo_state,
    [46] = &sf_state,
};

static inline unsigned int cur_sector(struct ctl_state* state)
{
    uint8_t k2 = state->k[2], k3 = state->k[3];

    if (state->new)
        return (k2 << 8) + k3;
    else
        return (((k2 << 3) + (k3 >> 5)) * state->secperclust) + (k3 & 31);
}

static inline int file_pos_valid(struct ctl_state* state)
{
    return cur_sector(state) < state->sectors;
}

static inline int file_pos(struct ctl_state* state)
{
    unsigned int ilmsk = state->ilmsk;
    unsigned int sector = cur_sector(state);

    sector = (sector & ~ilmsk) | ((sector * state->ilfac) & ilmsk);

    return sector << 8;
}

static void disk_reset_state(struct ctl_state* state)
{
    int i;

    state->state = disk_k0;
    state->status = state->aux_status = 0;
    state->in_ptr = -1;
    state->out_ptr = 0;
    state->notready_ctr = 4;

    for (i = 0; i < 8; i++)
        flush_file(state->files[i]);
}

static void disk_init(struct ctl_state* state)
{
    char devname[4];
    int i;

    /* If any of these don't exist we simply report device not ready */
    if (disk_path) {
        devname[0] = state->name[0];
        devname[1] = state->name[1];
        devname[3] = '\0';
        for (i = 0; i < 8; i++) {
            devname[2] = i + '0';
            /* Try open RDWR first, then RDONLY, but don't create */
            state->files[i] = open_host_file(HF_BINARY | HF_RETRY, disk_path,
                                             devname, O_RDWR);

            /* Try to memory-map the file */
            map_file(state->files[i], state->sectors << 8);
        }
    }
    disk_reset_state(state);
}

static void do_next_command(struct ctl_state* state)
{
    struct host_file* hf = state->files[state->k[1] & 7];
    uint8_t* buf = state->buf[state->k[1] >> 6]; /* If applicable */

    if (state->k[0] & 0x01) {
        /* READ SECTOR */
        if (hf->map) {
            memcpy(buf, hf->map + file_pos(state), 256);
        } else {
            fseek(hf->f, file_pos(state), SEEK_SET);
            fread(buf, 1, 256, hf->f);
        }
        state->k[0] &= ~0x01; /* Command done */
    }
    if (state->k[0] & 0x02) {
        /* SECTOR TO HOST */
        state->in_ptr = 0;
        state->state = disk_download;
        state->k[0] &= ~0x02; /* Command done */
        return;
    }
    if (state->k[0] & 0x04) {
        /* SECTOR FROM HOST */
        state->state = disk_upload;
        state->out_ptr = 0;
        state->k[0] &= ~0x04; /* Command done */
        return;
    }
    if (state->k[0] & 0x08) {
        /* WRITE SECTOR */
        if (!file_wrok(hf)) {
            state->status = 0x80;     /* Error */
            state->aux_status = 0x40; /* Write protect */
        } else if (hf->map) {
            memcpy(hf->map + file_pos(state), buf, 256);
        } else {
            clearerr(hf->f);
            fseek(hf->f, file_pos(state), SEEK_SET);
            fwrite(state->buf[state->k[1] >> 6], 1, 256, hf->f);
            if (ferror(hf->f)) {
                state->status = 0x08;     /* Error */
                state->aux_status = 0x40; /* Write protect */
            }
        }
        state->k[0] &= ~0x08; /* Command done */
    }
    state->state = disk_k0;
}

void disk_reset(void)
{
    int i;
    struct ctl_state* state;
    for (i = 0; i < 64; i++) {
        state = sel_to_state[i];
        if (state && state->state != disk_need_init)
            disk_reset_state(state);
    }
}

void disk_out(int sel, int port, int value)
{
    struct ctl_state* state = sel_to_state[sel];

    if (!state)
        return; /* Not a disk drive */

    if (state->state == disk_need_init)
        disk_init(state);

    switch (port) {
    case 0:
        switch (state->state) {
        case disk_k0:
        case disk_k1:
        case disk_k2:
            state->status = state->aux_status = 0;
            state->k[state->state - disk_k0] = value;
            state->state++;
            break;
        case disk_k3:
            state->status = state->aux_status = 0;
            state->k[3] = value;
            state->state = disk_k0;

            if (tracing(TRACE_DISK)) {
                fprintf(tracef, "%s%d: command %02X %02X %02X %02X\n",
                        state->name, state->k[1] & 7, state->k[0], state->k[1],
                        state->k[2], state->k[3]);
                fprintf(tracef, "PC = %04X  BC = %04X  DE = %04X  HL = %04X\n",
                        REG_PC, REG_BC, REG_DE, REG_HL);
            }

            /* Bad drive/sector? */
            if (!state->files[state->k[1] & 7]) {
                state->status = 0x08;     /* Error */
                state->aux_status = 0x80; /* Device not ready */
            } else if (!file_pos_valid(state)) {
                state->status = 0x08;     /* Error */
                state->aux_status = 0x10; /* Seek error */
            } else {
                do_next_command(state);
            }
            break;
        case disk_upload:
            state->buf[state->k[1] >> 6][state->out_ptr++] = value;
            if (tracing(TRACE_DISK))
                fprintf(tracef, "%02X", value);
            if (state->out_ptr >= 256) {
                if (tracing(TRACE_DISK))
                    fprintf(tracef,
                            "\nPC = %04X  BC = %04X  DE = %04X  HL = %04X\n",
                            REG_PC, REG_BC, REG_DE, REG_HL);
                do_next_command(state);
            }
            break;
        case disk_download:
            break;
        case disk_need_init:
            abort(); /* Should never happen */
            break;
        }
        break;

    case 2: /* Start command */
    case 4: /* Reset */
        if (tracing(TRACE_DISK)) {
            fprintf(tracef, "OUT %d/%d : ", sel, port);
            fprintf(tracef, "PC = %04X  BC = %04X  DE = %04X  HL = %04X\n",
                    REG_PC, REG_BC, REG_DE, REG_HL);
        }
        disk_reset();
        break;

    default:
        /* Nothing */
        break;
    }
}

int disk_in(int sel, int port)
{
    struct ctl_state* state = sel_to_state[sel];
    uint8_t v = 0xff;

    if (!state)
        return 0xff; /* Not a disk drive */

    if (state->state == disk_need_init)
        disk_init(state);

    switch (port) {
    case 0:
        if (state->in_ptr >= 0) {
            v = state->buf[state->k[1] >> 6][state->in_ptr++];
            if (state->in_ptr >= 256) {
                state->in_ptr = -1;
                do_next_command(state);
            }
        } else {
            v = state->aux_status;
        }
        break;

    case 1: /* Controller status */
        if (state->notready_ctr) {
            state->notready_ctr--;
            v = 0x80;
        } else {
            v = 0x01 | state->status | ((state->state == disk_k0) ? 0x80 : 0);
        }
        break;

    default:
        break;
    }

    if (tracing(TRACE_DISK)) {
        fprintf(tracef, "IN %d/%d: %02X : ", sel, port, v);
        fprintf(tracef, "PC = %04X  BC = %04X  DE = %04X  HL = %04X\n", REG_PC,
                REG_BC, REG_DE, REG_HL);
    }
    return v;
}
