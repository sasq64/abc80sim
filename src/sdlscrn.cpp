/*
 * sdlscrn.c
 *
 * ABC80/802 screen emulation (40x24/80x24)
 */

extern "C"
{

#include "abcio.h"
#include "clock.h"
//#include "nstime.h"
#include "screen.h"
#include "screenshot.h"
#include "trace.h"
#include "z80.h"
}
#include <inttypes.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define min(x, y) ((x) < (y) ? (x) : (y))
#define max(x, y) ((x) > (y) ? (x) : (y))

#define TS_WIDTH 80
#define TS_HEIGHT 24

#define FONT_XSIZE 6
#define FONT_YSIZE 10

#define FONT_XDUP 2 /* For 80-column mode */
#define FONT_YDUP 3

#define PX_WIDTH (TS_WIDTH * FONT_XSIZE * FONT_XDUP)
#define PX_HEIGHT (TS_HEIGHT * FONT_YSIZE * FONT_YDUP)

extern const unsigned char abc_font[256][FONT_YSIZE];

static void trigger_refresh(void);

#define NCOLORS 8

static struct argb
{
    uint8_t a, r, g, b;
} rgbcolors[NCOLORS] = {
    {0x00, 0x00, 0x00, 0x00}, /* black */
    {0x00, 0xff, 0x00, 0x00}, /* red */
    {0x00, 0x00, 0xff, 0x00}, /* green */
    {0x00, 0xff, 0xff, 0x00}, /* yellow */
    {0x00, 0x00, 0x00, 0xff}, /* blue */
    {0x00, 0xff, 0x00, 0xff}, /* purple */
    {0x00, 0x00, 0xff, 0xff}, /* cyan */
    {0x00, 0xff, 0xff, 0xff}, /* white */
};

/* Mutex for interaction with the CPU thread */
static SDL_mutex* screen_mutex;

#define VRAM_SIZE 2048
#define VRAM_MASK (VRAM_SIZE - 1)

union crtc
{
    uint8_t regs[18];
    struct
    {
        uint8_t htotal;     /* Horizontal total characters */
        uint8_t hdisp;      /* Horizontal displayed characters */
        uint8_t hsyncpos;   /* Horizontal sync position (char units) */
        uint8_t hsyncwidth; /* Horizontal sync width (char units) */
        uint8_t vscantotal; /* Vertical total (char units) */
        uint8_t vadjust;    /* Vertical adjust scan lines */
        uint8_t vdisplay;   /* Displayed character rows */
        uint8_t vsyncpos;   /* Vertical sync position */
        uint8_t interlace;  /* Interlace mode */
        uint8_t maxscan;    /* Maximum scan line address */
        uint8_t curstart;   /* Cursor start line */
        uint8_t curend;     /* Cursor end line */
        uint8_t starth;     /* High half of start address */
        uint8_t startl;     /* Low half of start address */
        uint8_t curh;       /* High half of cursor address */
        uint8_t curl;       /* Low half of cursor address */
    } r;
};

/*
 * Total video state. We keep three copies: one for the CPU to access,
 * one to hand over protected by screen_mutex, and one for the screen
 * generation.
 */
struct video_state
{
    union crtc crtc;
    uint16_t startaddr; /* Position of the first character */
    uint16_t curaddr;   /* Memory position of the CRTC cursor */
    bool mode40;
    bool blink_on;
    uint8_t vram[VRAM_SIZE];
};
static struct video_state cpu, xfr, vdu;
uint8_t* const video_ram = cpu.vram;

struct xy
{
    uint8_t x, y;
};
static struct xy addr_to_xy_tbl[2][2048];

/* A local abstraction of a drawing surface */
struct sdl_data
{
    SDL_Window* window; /* SDL_Window object */
    SDL_Renderer* renderer; /* SDL_Renderer object */
    SDL_Texture* texture;  /* SDL_Texture object */
    Uint32 colors[NCOLORS];
    int lock_count;   /* Lock nesting count */
    uint64_t updated; /* Time stamp of last update */
};

static struct sdl_data rscreen; /* The "physical" screen surface */

/*
 * Give the x,y coordinates for a given location in shadow video RAM
 */
static inline struct xy addr_to_xy(const uint8_t* p)
{
    uint16_t addr = p - vdu.vram;
    addr = (addr - vdu.startaddr) & VRAM_MASK;
    return addr_to_xy_tbl[vdu.mode40][addr];
}

/*
 * Compute the screen offset for a specific x,y coordinates
 */
template <int MODEL> static inline unsigned int screenoffs(uint8_t y, uint8_t x)
{
    size_t offs = -1;

    switch (MODEL) {
    case MODEL_ABC80_M40:
        offs = 1024 + (((y >> 3) * 5) << 3) + ((y & 7) << 7) + x;
        break;
    case MODEL_ABC80:
        offs = (((y >> 3) * 5) << 4) + ((y & 7) << 8) + x;
        break;

    case MODEL_ABC802:
        offs = (y * 80) + x;
        break;

    case MODEL_ABC802_M40:
        offs = (y * 80) + (x << 1);
        break;
    }

    return offs;
}

static inline unsigned int screenoffs(uint8_t y, uint8_t x, bool m40)
{
    size_t offs = -1;

    switch (model) {
    case MODEL_ABC80:
        if (m40)
            offs = 1024 + (((y >> 3) * 5) << 3) + ((y & 7) << 7) + x;
        else
            offs = (((y >> 3) * 5) << 4) + ((y & 7) << 8) + x;
        break;

    case MODEL_ABC802:
        offs = (y * 80) + (x << m40);
        break;
    }

    return offs;
}


/*
 * Return a specific character
 */
template <int MODEL> static inline uint8_t screendata(uint8_t y, uint8_t x)
{
    return vdu.vram[(screenoffs<MODEL>(y, x) + vdu.startaddr) &
                    VRAM_MASK];
}

/*
 * Prevent/allow screen refresh
 */
static void lock_screen(struct sdl_data* s)
{
    if (!s->lock_count++)
        SDL_LockSurface(SDL_GetWindowSurface(s->window));
}

static void unlock_screen(struct sdl_data* s)
{
    if (s->lock_count > 0)
        SDL_UnlockSurface(SDL_GetWindowSurface(s->window));
    else if (unlikely(s->lock_count < 0))
        abort(); /* SHOULD NEVER HAPPEN */

    s->lock_count--;
}

/*
 * Update the on-screen structure to match the screendata[]
 * for character (tx,ty), but don't refresh the rectangle just
 * yet...
 */

template <int MODEL>
static void put_screen(struct sdl_data* s, unsigned int tx, unsigned int ty,
                       bool blink)
{
    const unsigned char* fontp;
    unsigned int voffs;
    unsigned char v, vv;
    uint32_t *pixelp, *pixelpp, fgp, bgp;
    unsigned int x, xx, y, yy, gx;
    uint32_t curmask;
    unsigned char gmode, fg, bg;
    unsigned char cc, invmask;
    unsigned int xdup = FONT_XDUP << vdu.mode40;

    if (tx >= (unsigned int)(TS_WIDTH >> vdu.mode40) ||
        ty >= (unsigned int)TS_HEIGHT)
        return;

    bg = 0; /* XXX: handle NWBG */
    fg = 7;

    gmode = 0;
    for (gx = 0; gx < tx; gx++) {
        cc = screendata<MODEL>(ty, gx);
        if ((cc & 0x68) == 0) {
            gmode = (cc & 0x10) << 3;
            fg = (cc & 0x07);
        }
    }

    voffs = screenoffs<MODEL>(ty, tx) + vdu.startaddr;
    cc = vdu.vram[voffs & VRAM_MASK];
    fontp = abc_font[(cc & 0x7f) + gmode];
    invmask = (blink || model != MODEL_ABC80) ? 0x80 : 0;
    invmask = (cc & invmask) ? 7 : 0;
    bg ^= invmask;
    fg ^= invmask;

    bgp = s->colors[bg];
    fgp = s->colors[fg];

    pixelp = ((uint32_t*)SDL_GetWindowSurface(s->window)->pixels) +
             ty * PX_WIDTH * FONT_YSIZE * FONT_YDUP +
             ((tx * FONT_XSIZE * FONT_XDUP) << vdu.mode40);

    curmask = 0;
    if (unlikely(voffs == vdu.curaddr)) {
        if (blink | (vdu.crtc.r.curstart & 0x40)) {
            curmask = (~0U << (vdu.crtc.r.curstart & 0x1f));
            curmask &= (2U << (vdu.crtc.r.curend & 0x1f)) - 1;
        }
    }

    for (y = 0; y < FONT_YSIZE; y++) {
        vv = *fontp++;
        if (curmask & 1)
            vv = 0x3f;
        curmask >>= 1;
        for (yy = 0; yy < FONT_YDUP; yy++) {
            v = vv;
            pixelpp = pixelp;
            for (x = 0; x < FONT_XSIZE; x++) {
                for (xx = 0; xx < xdup; xx++) {
                    *pixelpp++ = (v & 0x80) ? fgp : bgp;
                }
                v <<= 1;
            }
            pixelp += PX_WIDTH;
        }
    }
}

static void update_screen(struct sdl_data* s)
{
    if (s->lock_count > 0)
        return;

    SDL_Surface *surf = SDL_GetWindowSurface(s->window);
    SDL_UpdateTexture(s->texture, NULL, surf->pixels, surf->pitch);
    SDL_RenderClear(s->renderer);
    SDL_RenderCopy(s->renderer, s->texture, NULL, NULL);
    SDL_RenderPresent(s->renderer);
}

/*
 * Refresh the entire screen or recreate the screen on another surface.
 * If "force_blink" is true, always draw blinking elements visible.
 */
static void refresh_screen(struct sdl_data* s, bool force_blink)
{
    unsigned int x, y;
    unsigned int width;
    bool blink;

    SDL_mutexP(screen_mutex);
    vdu = xfr;
    SDL_mutexV(screen_mutex);

    width = TS_WIDTH >> vdu.mode40;
    blink = force_blink | vdu.blink_on;

    lock_screen(s);

    if (model == MODEL_ABC80) {
        if (vdu.mode40) {
            for (y = 0; y < TS_HEIGHT; y++)
                for (x = 0; x < width; x++)
                    put_screen<MODEL_ABC80_M40>(s, x, y, blink);
        } else {
            for (y = 0; y < TS_HEIGHT; y++)
                for (x = 0; x < width; x++)
                    put_screen<MODEL_ABC80>(s, x, y, blink);
        }
    } else if (model == MODEL_ABC802) {
        if (vdu.mode40) {
            for (y = 0; y < TS_HEIGHT; y++)
                for (x = 0; x < width; x++)
                    put_screen<MODEL_ABC802_M40>(s, x, y, blink);
        } else {
            for (y = 0; y < TS_HEIGHT; y++)
                for (x = 0; x < width; x++)
                    put_screen<MODEL_ABC802>(s, x, y, blink);
        }
    }
    unlock_screen(s);
    update_screen(s);
}

/* Called in CPU thread context */
void setmode40(bool m40)
{
    cpu.mode40 = m40;
    if (model == MODEL_ABC80)
        abc80_mem_mode40(m40);
}

/*
 * Wrap an SDL_Surface in our local stuff
 */
static struct sdl_data* init_sdl_data(struct sdl_data* s)
{
    int i;
    SDL_Surface *surf = SDL_GetWindowSurface(s->window);

    if (unlikely(!s || !surf))
        return NULL;

    /* Convert colors to preferred machine representation */
    for (i = 0; i < NCOLORS; i++) {
        s->colors[i] = SDL_MapRGB(surf->format,
                                  rgbcolors[i].r,
                                  rgbcolors[i].g,
                                  rgbcolors[i].b);
    }

    /* Surface is unlocked */
    s->lock_count = 0;

    return s;
}

/*
 * Screenshot setup
 */
static void abc_screenshot(void)
{
    struct sdl_data s;

    SDL_Surface *surf;
    surf = SDL_CreateRGBSurface(SDL_SWSURFACE, PX_WIDTH, PX_HEIGHT, 32,
                                  0x00ff0000, 0x0000ff00, 0x000000ff, 0);
    if (!init_sdl_data(&s))
        return;
    refresh_screen(&s, true); /* Always snapshot with blink on */

    screenshot(surf);
    SDL_FreeSurface(surf);
}

/*
 * Initialize SDL and the data structures
 */
void screen_init(bool width40, bool color)
{
    int window = 1; /* True = run in a window */
    int i, x, y;

    if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO))
        return;

    atexit(SDL_Quit);

    SDL_CreateWindowAndRenderer(PX_WIDTH, PX_HEIGHT,
                                SDL_WINDOW_SHOWN,
                                &rscreen.window, &rscreen.renderer);

    if (rscreen.window == NULL || rscreen.renderer == NULL) {
        fprintf(stderr, "Failed to initialize SDL objects.");
        exit(EXIT_FAILURE);
    }

    SDL_SetRenderDrawColor(rscreen.renderer, 0, 0, 0, 255);
    SDL_RenderClear(rscreen.renderer);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.
    SDL_RenderSetLogicalSize(rscreen.renderer, PX_WIDTH, PX_HEIGHT);

    rscreen.texture = SDL_CreateTexture(rscreen.renderer,
                               SDL_PIXELFORMAT_ARGB8888,
                               SDL_TEXTUREACCESS_STREAMING,
                               PX_WIDTH, PX_HEIGHT);

    /* No mouse cursor in full screen mode */
    if (!window)
        SDL_ShowCursor(SDL_DISABLE);

    /* If not color, then overwrite colors 1-6 with white */
    if (!color) {
        for (i = 1; i < NCOLORS - 1; i++)
            rgbcolors[i] = rgbcolors[NCOLORS - 1];
    }

    /* Initialize CRTC values to something sensible (also used by ABC80) */
    memset(&cpu, 0, sizeof cpu);
    cpu.crtc.r.htotal = 80;
    cpu.crtc.r.hdisp = 80;
    cpu.crtc.r.vscantotal = 24;
    cpu.crtc.r.vdisplay = 24;
    cpu.crtc.r.curstart = 0x1f; /* No CRTC cursor */
    setmode40(width40);
    vdu = xfr = cpu;

    /* Initialize reverse mapping table */
    memset(addr_to_xy_tbl, -1, sizeof addr_to_xy_tbl);
    for (i = 0; i < 2; i++) {
        for (y = 0; y < TS_HEIGHT; y++) {
            for (x = 0; x < (TS_WIDTH >> i); x++) {
                size_t p = screenoffs(y, x, i);
                addr_to_xy_tbl[i][p].x = x;
                addr_to_xy_tbl[i][p].y = y;
            }
        }
    }

    /* Create interlock mutex */
    screen_mutex = SDL_CreateMutex();

    if (!init_sdl_data(&rscreen))
        return;

    /* Draw initial screen */
    refresh_screen(&rscreen, false);
}

/*
 * Cleanup
 */
void screen_reset(void)
{
    /* Handled by atexit */
}

/*
 * Event-handling loop; main loop of the event/screen thread.
 */
enum dump_memory_type
{
    DUMP_NONE,
    DUMP_MEM,
    DUMP_RAM
};

static volatile enum dump_memory_type dump_memory_now;

void event_loop(void)
{
    SDL_Event event;
    static int keyboard_scan = -1, /* No key currently down */
               pend_keyboard_scan = -1;
    enum kshift
    {
        KSH_SHIFT = 1,
        KSH_CTRL = 2,
        KSH_ALT = 4
    };
    int kshift;

    while (SDL_WaitEvent(&event)) {
        switch (event.type) {
        case SDL_KEYDOWN:
            kshift =
                ((event.key.keysym.mod & (KMOD_LALT | KMOD_RALT)) ? KSH_ALT
                                                                  : 0) |
                ((event.key.keysym.mod & (KMOD_LCTRL | KMOD_RCTRL)) ? KSH_CTRL
                                                                    : 0) |
                ((event.key.keysym.mod & (KMOD_LSHIFT | KMOD_RSHIFT))
                     ? KSH_SHIFT
                     : 0);

            if (kshift & KSH_ALT) {
                /* Alt+key are special functions */

                switch (event.key.keysym.sym) {
                case SDLK_END:
                case SDLK_q:
                    return; /* Return to main() and exit simulator */

                case SDLK_s:
                    abc_screenshot();
                    break;

                case SDLK_r:
                    z80_reset();
                    break;

                case SDLK_n:
                    z80_nmi();
                    break;

                case SDLK_m:
                    dump_memory_now = DUMP_MEM;
                    break;

                case SDLK_u:
                    dump_memory_now = DUMP_RAM;
                    break;

                case SDLK_f:
                    faketype = !faketype;
                    break;

                default:
                    break;
                }
            } else {
                int mysym = -1;

                switch (event.key.keysym.sym) {
                case SDLK_BACKSPACE:
                case SDLK_LEFT:
                    mysym = 8;
                    break;

                case SDLK_RIGHT:
                    mysym = 9;
                    break;

                case SDLK_TAB:
                    mysym = '\t';
                    break;

                case SDLK_F1:
                case SDLK_F2:
                case SDLK_F3:
                case SDLK_F4:
                case SDLK_F5:
                case SDLK_F6:
                case SDLK_F7:
                case SDLK_F8:
                    mysym = (event.key.keysym.sym - SDLK_F1 + 192) +
                            ((int)kshift << 3);
                    break;

                case SDLK_ESCAPE:
                    mysym = 127;
                    break;

                case SDLK_SPACE: /* Ctrl+Space -> NUL */
                    if (kshift & KSH_CTRL)
                        mysym = 0;
                    break;

                case SDLK_RETURN:
                case SDLK_KP_ENTER:
                    mysym = '\r';
                    break;

                default: break;
                }
                if (mysym >= 0) {
                    /* Remember which key so we can tell when it is released */
                    keyboard_scan = event.key.keysym.scancode;
                    keyboard_down(mysym);
                } else
                    pend_keyboard_scan = event.key.keysym.scancode;
            }
            break;
        case SDL_KEYUP:
            if (event.key.keysym.scancode == keyboard_scan)
                keyboard_up();
            break;
        case SDL_TEXTINPUT: {
            int mysym = -1;
            switch (SDL_strlen(event.text.text)) {
            case 1:
                assert((event.text.text[0] & 0x80) == 0);
                mysym = event.text.text[0];
                break;
            case 2:
                // Get a unicode 2 byte sequence.
                assert((event.text.text[0] & 0xE0) == 0xC0);
                assert((event.text.text[1] & 0xC0) == 0x80);
                mysym = ((event.text.text[0] & 0x1C) << 8) & 0xff00;
                mysym |= ((event.text.text[0] & 0x03) << 6) & 0x00C0;
                mysym |= ((event.text.text[1] & 0x3F));
                break;
            default:
                break; // Not a supported character.
            }

            switch ((wchar_t)mysym) {
            case L'¤':
                mysym = '$';
                break;
            case L'É':
                mysym = '@';
                break;
            case L'Å':
                mysym = ']';
                break;
            case L'Ä':
                mysym = '[';
                break;
            case L'Ö':
                mysym = '\\';
                break;
            case L'Ü':
                mysym = '^';
                break;
            case L'é':
                mysym = '`';
                break;
            case L'å':
                mysym = '}';
                break;
            case L'ä':
                mysym = '{';
                break;
            case L'ö':
                mysym = '|';
                break;
            case L'ü':
                mysym = '~';
                break;
            case L'<':
            case L'>':
                if (kshift & KSH_CTRL)
                    mysym = 127;
                break;
            case L'§':
            case L'½':
                mysym = 127;
                break;
            default:
                break;
            }

            if (!(mysym & ~0x1f)) {
                /* Shift+Ctrl -> invert bit 4 */
                if (kshift == (KSH_CTRL | KSH_SHIFT))
                    mysym ^= 0x10;
            }
            if (mysym >= 0) {
                /* Remember which key so we can tell when it is released */
                keyboard_scan = pend_keyboard_scan;
                keyboard_down(mysym);
            }
        } break;

        case SDL_USEREVENT:
            /* Time to update the screen */
            refresh_screen(&rscreen, false);
            break;
        case SDL_QUIT:
            return; /* Return to main(), terminate */
        default:
            break;
        }
    }
}

/*
 * Called from the timer that corresponds to the simulated vsync
 * in the CPU thread context
 */
void vsync_screen(void)
{
    const int blink_rate = 400 / 20; /* 400 ms/20 ms = 2.5 Hz */
    static int blink_ctr;
    enum dump_memory_type dm;

    if (!blink_ctr--) {
        blink_ctr = blink_rate;
        cpu.blink_on = !cpu.blink_on;
    }

    trigger_refresh();

    if (unlikely(dump_memory_now)) {
        dm = xchg(&dump_memory_now, DUMP_NONE);
        if (dm)
            dump_memory(dm == DUMP_RAM);
    }

    if (traceflags)
        fflush(tracef); /* So we don't buffer indefinitely */
}

/* Used from the CPU thread context to cause a screen redraw */
static void trigger_refresh(void)
{
    SDL_Event trigger_redraw;

    SDL_mutexP(screen_mutex);
    xfr = cpu;
    SDL_mutexV(screen_mutex);

    memset(&trigger_redraw, 0, sizeof trigger_redraw);
    trigger_redraw.type = SDL_USEREVENT;
    SDL_PushEvent(&trigger_redraw);
}

/* Called in the CPU thread context */
static uint8_t crtc_addr;

void crtc_out(uint8_t port, uint8_t data)
{
    if (!(port & 1)) {
        crtc_addr = data;
        return;
    }

    if (crtc_addr >= sizeof cpu.crtc.regs)
        return;

    SDL_mutexP(screen_mutex);

    cpu.crtc.regs[crtc_addr] = data;
    cpu.startaddr = ((cpu.crtc.r.starth & 0x3f) << 8) + cpu.crtc.r.startl;
    cpu.curaddr = ((cpu.crtc.r.curh & 0x3f) << 8) + cpu.crtc.r.curl;

    SDL_mutexV(screen_mutex);
}

uint8_t crtc_in(uint8_t port)
{
    if (!(port & 1))
        return crtc_addr;

    if (crtc_addr >= sizeof cpu.crtc.regs)
        return 0xff;

    return cpu.crtc.regs[crtc_addr];
}
