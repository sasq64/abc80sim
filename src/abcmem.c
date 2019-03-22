#include "abcfile.h"
#include "abcio.h"
#include "compiler.h"
#include "hostfile.h"
#include "rom.h"
#include "screen.h"
#include "z80.h"

#define MEMORY_SIZE Z80_ADDRESS_LIMIT

uint8_t ram[MEMORY_SIZE];

typedef void (*write_func)(uint8_t* p, uint8_t v);
struct mem_page
{
    uint8_t* data;
    write_func write;
};

static void write_rom(uint8_t* p, uint8_t v);
#define write_ram NULL /* Optimized fast path */
#define write_screen write_ram

#define PAGE_SHIFT 10
#define PAGE_SIZE (1U << PAGE_SHIFT)
#define PAGE_MASK (PAGE_SIZE - 1)
#define PAGE_COUNT (Z80_ADDRESS_LIMIT / PAGE_SIZE)

/* Up to 8 memory maps */
#define MEM_MAPS 8
static struct mem_page memmaps[MEM_MAPS][PAGE_COUNT];

/* Latch the last M1 address fetched, like ABC800 does */
static uint16_t last_m1_address;

/*
 * Currently active memory map(s)
 *
 * ABC800 can have two memory maps: the second kicks in when executing code
 * in the range 0x7800-0x7fff
 */
static const struct mem_page* current_map[2];

static inline const struct mem_page* get_page(uint16_t addr)
{
    size_t map = (last_m1_address & 0xf800) == 0x7800;
    return &current_map[map][addr >> PAGE_SHIFT];
}

/*
 * Memory tracing support
 */
struct mem_trace
{
    uint16_t addr, data;
    uint8_t size;
    bool written;
};

#define MAX_TRACES 16
static struct mem_trace mem_traces[MAX_TRACES + 1];
static struct mem_trace* mem_trace_tail = mem_traces;

static inline void mem_trace_record(uint16_t addr, uint16_t data, uint8_t size,
                                    bool written)
{
    if (!tracing(TRACE_CPU))
        return;

    if (mem_trace_tail <= &mem_traces[MAX_TRACES]) {
        mem_trace_tail->addr = addr;
        mem_trace_tail->data = data;
        mem_trace_tail->size = size;
        mem_trace_tail->written = written;
        mem_trace_tail++;
    }
}

/* Write memory trace data to screen */
void tracemem(void)
{
    const struct mem_trace* mtp;
    bool overflow = false;
    uint16_t last_addr = 0;
    int last_written = -1;

    if (!tracing(TRACE_CPU))
        return;

    if (mem_trace_tail >= &mem_traces[MAX_TRACES]) {
        mem_trace_tail--;
        overflow = true;
    }

    for (mtp = mem_traces; mtp < mem_trace_tail; mtp++) {
        fputc(' ', tracef);
        if (mtp->addr != last_addr || mtp->written != last_written)
            fprintf(tracef, "(%04X)%c", mtp->addr, mtp->written ? '=' : ':');
        fprintf(tracef, "%0*X", mtp->size * 2, mtp->data);
        last_addr = mtp->addr + mtp->size;
        last_written = mtp->written;
    }

    if (overflow)
        fputs(" ...", tracef);

    mem_trace_tail = mem_traces;
}

static inline uint8_t do_mem_read(uint16_t address)
{
    return get_page(address)->data[address & PAGE_MASK];
}

uint8_t mem_read(uint16_t address)
{
    uint8_t value = do_mem_read(address);

    mem_trace_record(address, value, 1, false);
    return value;
}

uint8_t mem_fetch(uint16_t address)
{
    /* Don't trace instruction fetches */
    return do_mem_read(address);
}

/* This is called when fetching the first opcode byte, corresponding to M1# */
uint8_t mem_fetch_m1(uint16_t address)
{
    /* Don't trace instruction fetches */
    last_m1_address = address;
    return do_mem_read(address);
}

/*
 * Words are stored with the low-order byte in the lower address.
 */
static inline uint16_t do_mem_read_word(uint16_t address)
{
    uint8_t b0, b1;

    b0 = do_mem_read(address);
    b1 = do_mem_read(address + 1);

    return (b1 << 8) + b0;
}

uint16_t mem_read_word(uint16_t address)
{
    uint16_t value = do_mem_read_word(address);
    mem_trace_record(address, value, 2, false);
    return value;
}

uint16_t mem_fetch_word(uint16_t address)
{
    /* Don't trace instruction fetches */
    return do_mem_read_word(address);
}

/*
 * Simple write operations
 */
static void write_rom(uint8_t* p, uint8_t v)
{
    /* Do nothing */
    (void)p;
    (void)v;
}
static void do_mem_write(uint16_t address, uint8_t value)
{
    const struct mem_page* page;
    uint8_t* p;

    page = get_page(address);
    p = &page->data[address & PAGE_MASK];
    if (likely(!page->write))
        *p = value;
    else
        page->write(p, value);
}

void mem_write(uint16_t address, uint8_t value)
{
    mem_trace_record(address, value, 1, true);
    do_mem_write(address, value);
}

void mem_write_word(uint16_t address, uint16_t value)
{
    mem_trace_record(address, value, 2, true);
    do_mem_write(address, value);
    do_mem_write(address + 1, value >> 8);
}

/*
 * The ABC80 memory map can be altered either by flipping the
 * video mode or by doing out 7 (if enabled.)
 */
static unsigned int abc80_map;

void abc80_mem_mode40(bool mode40)
{
    abc80_map = (abc80_map & ~1) | mode40;
    current_map[0] = current_map[1] = memmaps[abc80_map];
}
void abc80_mem_setmap(unsigned int map)
{
    if (kilobytes < 64)
        return; /* Only 64K models can remap memory */

    abc80_map = ((map & 3) << 1) | (abc80_map & ~6);
    current_map[0] = current_map[1] = memmaps[abc80_map];
}

/*
 * Open or close the MEM: area on ABC802
 */
void abc802_set_mem(bool opened)
{
    current_map[0] = memmaps[opened ? 2 : 0];
    current_map[1] = memmaps[opened ? 2 : 1];
}

#define K(x) ((x)*1024)
#define ALL_MAPS ((1U << MEM_MAPS) - 1)

static void map_memory(unsigned int maps, size_t where, size_t size, void* what,
                       write_func wfunc)
{
    size_t m;

    assert(((where | size) & PAGE_MASK) == 0);
    assert((maps & ~ALL_MAPS) == 0);

    for (m = 0; maps; m++, maps >>= 1) {
        struct mem_page* mp;
        uint8_t* datap;
        size_t npg;

        if (!(maps & 1))
            continue;

        mp = &memmaps[m][where >> PAGE_SHIFT];
        datap = what;

        npg = size >> PAGE_SHIFT;

        while (npg--) {
            mp->data = datap;
            mp->write = wfunc;
            datap += PAGE_SIZE;
            mp++;
        }
    }
}

/*
 * Load a binary file into low (< 30K) RAM, in the format used by
 * the ABC802 MEM: device.
 */
static void load_memfile(const char* memfile)
{
    struct host_file* hf;
    uint8_t* rp;
    unsigned int blk;
    struct abcdata abc;
    const unsigned int max_blocks = (K(30) >> 8) - 1;

    if (!memfile)
        return;

    rp = ram;

    hf = open_host_file(HF_BINARY, NULL, memfile, O_RDONLY);
    if (!hf)
        goto exit;
    if (!map_file(hf, 0))
        goto exit;

    init_abcdata(&abc, hf->map, hf->flen);
    blk = 0;
    while (blk < max_blocks) {
        bool done;
        rp[0] = 0x53;
        rp[1] = 0;
        rp[2] = blk++;
        done = get_abc_block(rp + 3, &abc);
        rp += 256;
        if (done)
            break;
    }

exit:
    memset(rp, 0, 3); /* Avoid possible stray magic */
    close_file(&hf);
}

/*
 * Set up memory maps.  Note: dump_memory() currently relies on
 * map 7 being all RAM, regardless of if there is an actual
 * map 7 or not.  If this isn't reliable, change this to have a
 * map set up specifically for Alt-u dumps.
 */
void mem_init(unsigned int flags, const char* memfile)
{
    /* Start by initializing all memory maps to all RAM */
    map_memory(ALL_MAPS, 0, K(64), ram, write_ram);

    switch (model) {
    case MODEL_ABC80:
        /* 4 maps * 2 (40/80) */

        if ((kilobytes < 1 || kilobytes > 32) && kilobytes != 64) {
            fprintf(stderr, "%s: invalid ABC80 memory size %uK, using 64K\n",
                    program_name, kilobytes);
            kilobytes = 64;
        }

        /* Map 0: default (for < 64K, the only available map) */
        if (!(flags & MEMFL_NOBASIC)) {
            map_memory(0x01, 0, K(16), old_basic ? abc80bas80o : abc80bas80n,
                       write_rom);
            map_memory(0x02, 0, K(16), old_basic ? abc80bas40o : abc80bas40n,
                       write_rom);
        }
        if (!(flags & MEMFL_NODEV)) {
            /* Hack: allow device ROMs to be written to */
            map_memory(0x03, K(16), K(16), abc80_devs, write_ram);
        }
        map_memory(0x01, K(29), K(1), &video_ram[K(0)], write_screen);
        map_memory(0x03, K(31), K(1), &video_ram[K(1)], write_screen);

        if (kilobytes < 32) {
            /*
             * Simulate non-existing memory by filling it with FF
             * and changing it to readonly.  ABC80 RAM grows from
             * top of memory downward toward 32K.
             */
            memset(ram + K(32), 0xff, K(32 - kilobytes));
            map_memory(0x03, K(32), K(32 - kilobytes), &ram[K(32)], write_rom);
        }

        /* Map 1: RAM over ROM areas */
        map_memory(0x04, K(30), K(2), &video_ram[K(0)], write_screen);
        map_memory(0x08, K(31), K(1), &video_ram[K(1)], write_screen);

        /* Map 2: video RAM at the end */
        map_memory(0x10, K(62), K(2), &video_ram[K(0)], write_screen);
        map_memory(0x20, K(63), K(1), &video_ram[K(1)], write_screen);

        /* Map 3: all RAM */

        abc80_mem_setmap(0); /* Default to map 0 */
        break;

    case MODEL_ABC802:
        /* Map 0: normal execution */

        if (!(flags & MEMFL_NOBASIC))
            map_memory(0x01, 0, K(24), abc802rom, write_rom);

        if (!(flags & MEMFL_NODEV))
            map_memory(0x01, K(24), K(8), &abc802rom[K(24)], write_rom);

        map_memory(0x01, K(30), K(2), video_ram, write_screen);

        /* Map 1: execution in option ROM - RAM other than the ROM itself */
        map_memory(0x02, K(30), K(2), &abc802rom[K(30)], write_rom);

        /* Map 2: MEM area open in its entirety */

        abc802_set_mem(false); /* On start, MEM area closed */
        break;
    }

    load_memfile(memfile);
}

/*
 * Dump memory to a file
 */
const char* memdump_path;

void dump_memory(bool ramonly)
{
    const struct mem_page* map = ramonly ? memmaps[7] : current_map[0];
    struct host_file* hf;
    size_t i;

    hf = dump_file(HF_BINARY, memdump_path,
                   ramonly ? "ram%04u.bin" : "mem%04u.bin");
    if (!hf)
        return;

    for (i = 0; i < PAGE_COUNT; i++)
        fwrite(map[i].data, 1, PAGE_SIZE, hf->f);

    if (!ferror(hf->f))
        keep_file(hf); /* It's good */

    close_file(&hf);
}
