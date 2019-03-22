/*
 * Take a PNG screenshot of an SDL surface
 */

#include "screenshot.h"
#include "abcio.h"
#include "compiler.h"
#include "hostfile.h"

#include <png.h>
#include <zlib.h>

const char* screen_path;

static inline void* pixel_row(const SDL_Surface* surf, size_t y)
{
    return (uint8_t*)surf->pixels + (y * surf->pitch);
}

struct sort_pixel
{
    uint32_t pix; /* Pixel value */
    uint32_t pos; /* Position index */
};

static int sort_by_pixel(const void* vp1, const void* vp2)
{
    const uint32_t pix1 = ((const struct sort_pixel*)vp1)->pix;
    const uint32_t pix2 = ((const struct sort_pixel*)vp2)->pix;

    /* A simple subtract here risks overflowing an int */
    return -(pix1 < pix2) | (pix1 > pix2);
}

static inline bool get_pixels(SDL_Surface* surf, struct sort_pixel* ppp)
{
    uint8_t* pvp;
    int x, y;
    uint32_t pos = 0;
    const size_t bytes = surf->format->BytesPerPixel;
    Uint32 andbits = -1;

    switch (bytes) {
    case 1:
        for (y = 0; y < surf->h; y++) {
            pvp = pixel_row(surf, y);
            for (x = 0; x < surf->w; x++) {
                andbits &= ppp->pix = *(uint8_t*)pvp;
                pvp += bytes;
                ppp->pos = pos++;
                ppp++;
            }
        }
        break;
    case 2:
        for (y = 0; y < surf->h; y++) {
            pvp = pixel_row(surf, y);
            for (x = 0; x < surf->w; x++) {
                andbits &= ppp->pix = *(uint16_t*)pvp;
                pvp += bytes;
                ppp->pos = pos++;
                ppp++;
            }
        }
        break;
    case 3:
        for (y = 0; y < surf->h; y++) {
            pvp = pixel_row(surf, y);
            for (x = 0; x < surf->w; x++) {
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
                andbits &= ppp->pix = pvp[0] | (pvp[1] << 8) | (pvp[2] << 16);
#else
                andbits &= ppp->pix = pvp[2] | (pvp[1] << 8) | (pvp[0] << 16);
#endif
                pvp += bytes;
                ppp->pos = pos++;
                ppp++;
            }
        }
        break;

    case 4:
        for (y = 0; y < surf->h; y++) {
            pvp = pixel_row(surf, y);
            for (x = 0; x < surf->w; x++) {
                andbits &= ppp->pix = *(uint32_t*)pvp;
                pvp += bytes;
                ppp->pos = pos++;
                ppp++;
            }
        }
        break;

    default:
        abort();
        break;
    }

    /* Return true if we have a nontrivial alpha channel */
    return !surf->format->palette &&
           (andbits & surf->format->Amask) != surf->format->Amask;
}

#define MAX_PALETTE 256
/* Returns the number of indicies, or -1 on failure */
static int make_indexed(SDL_Surface* surf, uint8_t** data, png_color** palettep,
                        uint8_t** apalettep)
{
    struct sort_pixel* pixp = NULL; /* Pixel pointers */
    struct sort_pixel* ppp;         /* Pixel pointer pointer */
    size_t np;                      /* Total number of pixels */
    size_t i;
    int last_index;            /* Last allocated index */
    uint32_t last_pixel;       /* Last equivalent pixel */
    SDL_PixelFormat* fmt;      /* Cached for speed */
    bool surface_copy = false; /* We copied the surface */
    uint8_t* iimg = NULL;      /* Actual indexed image */
    png_color* palette = NULL; /* Primary palette */
    uint8_t* apalette;         /* Alpha palette */
    bool hasalpha = false;

    /* This is kind of an idiotic algorithm, but it works and is kind of fun */

    /* 1. Allocate arrays and initialize the position array */
    np = surf->w * surf->h; /* Total pixels */

    iimg = malloc(np);
    if (!iimg)
        goto err;
    pixp = malloc(np * sizeof *pixp);
    if (!pixp)
        goto err;
    palette = calloc(MAX_PALETTE, (sizeof *palette + sizeof *apalette));
    if (!palette)
        goto err;

    SDL_LockSurface(surf);
    fmt = surf->format;
    hasalpha = get_pixels(surf, pixp);
    SDL_UnlockSurface(surf);

    /* 2. Sort by pixel value */
    qsort(pixp, np, sizeof *pixp, sort_by_pixel);

    /* 3. Create palette and index values */
    ppp = pixp;
    last_pixel = ~ppp->pix; /* Make sure we don't match on the first */
    last_index = -1;
    apalette = (uint8_t*)&palette[MAX_PALETTE];
    for (i = 0; i < np; i++) {
        if (ppp->pix != last_pixel) {
            last_pixel = ppp->pix;
            last_index++;
            if (last_index >= MAX_PALETTE)
                goto err;
            SDL_GetRGBA(last_pixel, fmt, &palette[last_index].red,
                        &palette[last_index].green, &palette[last_index].blue,
                        &apalette[last_index]);

            hasalpha |= apalette[last_index] != 0xff;
        }
        iimg[ppp->pos] = last_index;
        ppp++;
    }

    /* Done! */
    last_index++; /* Convert to a count */
common_exit:
    *data = iimg;
    *palettep = palette;
    *apalettep = hasalpha ? apalette : NULL;
    if (pixp)
        free(pixp);
    if (surface_copy && surf)
        SDL_FreeSurface(surf);
    return last_index;

err:
    if (palette) {
        free(palette);
        palette = NULL;
    }
    if (iimg) {
        free(iimg);
        iimg = NULL;
    }
    last_index = hasalpha ? -2 : -1;
    goto common_exit;
}

static void my_png_error(png_structp png, png_const_charp errmsg)
{
    (void)errmsg;
    longjmp(png_jmpbuf(png), 1);
}

static void my_png_warning(png_structp png, png_const_charp warnmsg)
{
    (void)png;
    (void)warnmsg;
}

/* This is the pixel format that PNG uses in 8-bit RGB and RGBA modes */
static SDL_PixelFormat pngfmts[2] = {
    {
        /* RGB format */

        NULL,       /* palette */
        24,         /* bits per pixel */
        3,          /* bytes per pixel */
        0, 0, 0, 8, /* precision loss (8 = all alpha lost) */
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
        0,          /* Rshift */
        8,          /* Gshift */
        16,         /* Bshift */
        0,          /* Ashift */
        0x000000ff, /* Rmask */
        0x0000ff00, /* Gmask */
        0x00ff0000, /* Bmask */
#else
        16,         /* Rshift */
        8,          /* Gshift */
        0,          /* Bshift */
        0,          /* Ashift */
        0x00ff0000, /* Rmask */
        0x0000ff00, /* Gmask */
        0x000000ff, /* Bmask */
#endif
        0x00000000, /* Amask */
        0,          /* No actual color key */
        255         /* Completely opaque */
    },
    {
        /* RGBA format */

        NULL,       /* palette */
        32,         /* bits per pixel */
        4,          /* bytes per pixel */
        0, 0, 0, 0, /* precision loss (none) */
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
        0,          /* Rshift */
        8,          /* Gshift */
        16,         /* Bshift */
        24,         /* Ashift */
        0x000000ff, /* Rmask */
        0x0000ff00, /* Gmask */
        0x00ff0000, /* Bmask */
        0xff000000, /* Amask */
#else
        24,         /* Rshift */
        16,         /* Gshift */
        8,          /* Bshift */
        0,          /* Ashift */
        0xff000000, /* Rmask */
        0x00ff0000, /* Gmask */
        0x0000ff00, /* Bmask */
        0x000000ff, /* Amask */
#endif
        0,  /* No actual color key */
        255 /* Completely opaque */
    }};

/*
 * This is a bit of a hack to work around potentially dangerous
 * setjmp() side effects.  The structure contains anything that
 * we may have to deallocate or clean up.
 */
struct allocable
{
    uint8_t* img;         /* Image data */
    SDL_Surface* rgbsurf; /* RGB converted surface */
    png_bytepp rowptrs;   /* Array of row pointers */
    png_color* palette;   /* Palette data */
    uint8_t* apalette;    /* Alpha palette data (does not need freeing) */
    png_structp png;      /* PNG write structure */
    png_infop png_info;   /* PNG info structure */
    struct host_file* hf; /* Host file structure */
};

static int do_screenshot(SDL_Surface* surf, struct allocable* a)
{
    uint8_t* row;
    size_t bytes_per_row;
    int y;
    int npalette, depth;
    time_t now;
    png_time png_now;
    png_bytepp rowptr;
    int color_type;

    /* Get current time for timestamp */
    time(&now);
    png_convert_from_time_t(&png_now, now);

    /* Allocate row pointers */
    a->rowptrs = malloc(surf->h * sizeof *a->rowptrs);
    if (!a->rowptrs)
        return -1;

    /* First, try an indexed image */
    npalette = make_indexed(surf, &a->img, &a->palette, &a->apalette);
    if (npalette > 0) {
        color_type = PNG_COLOR_TYPE_PALETTE;
        if (npalette <= 2)
            depth = 1;
        else if (npalette <= 4)
            depth = 2;
        else if (npalette <= 16)
            depth = 4;
        else
            depth = 8;

        bytes_per_row = surf->w;
        row = a->img;
    } else {
        bool is_rgba = npalette == -2;
        SDL_PixelFormat fmt = pngfmts[is_rgba];
        color_type = is_rgba ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB;

        a->rgbsurf = SDL_ConvertSurface(surf, &fmt, SDL_SWSURFACE);
        if (!a->rgbsurf)
            return -1;

        SDL_LockSurface(a->rgbsurf);
        depth = 8;
        bytes_per_row = a->rgbsurf->pitch;
        row = a->rgbsurf->pixels;
    }

    /* Generate row pointers */
    rowptr = a->rowptrs;
    for (y = 0; y < surf->h; y++) {
        *rowptr++ = row;
        row += bytes_per_row;
    }

    /* Create a PNG write and info structures */
    a->png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, my_png_error,
                                     my_png_warning);
    if (!a->png)
        return -1;

    a->png_info = png_create_info_struct(a->png);
    if (!a->png_info)
        return -1;

    if (setjmp(png_jmpbuf(a->png)))
        return -1;

    /* Open screenshot file */
    a->hf = dump_file(HF_BINARY, screen_path, "scrn%04u.png");
    if (!a->hf)
        return -1;
    png_init_io(a->png, a->hf->f);

    /* IHDR configuration */
    png_set_IHDR(a->png, a->png_info, surf->w, surf->h, depth, color_type,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
                 PNG_FILTER_TYPE_DEFAULT);

    png_set_tIME(a->png, a->png_info, &png_now);

    png_set_compression_level(a->png, Z_BEST_COMPRESSION);
    png_set_compression_strategy(a->png, Z_FILTERED);

    if (npalette > 0) {
        png_set_PLTE(a->png, a->png_info, a->palette, npalette);
        if (a->apalette) {
            int napalette = npalette;
            while (napalette > 0) {
                if (a->apalette[napalette - 1] < 0xff)
                    break;
                napalette--;
            }
            if (napalette)
                png_set_tRNS(a->png, a->png_info, a->apalette, napalette, NULL);
        }
    }

    png_set_rows(a->png, a->png_info, a->rowptrs);

    png_write_png(a->png, a->png_info,
                  (depth < 8) ? PNG_TRANSFORM_PACKING : PNG_TRANSFORM_IDENTITY,
                  NULL);

    keep_file(a->hf); /* We did it! */
    return 0;
}

int screenshot(SDL_Surface* surf)
{
    struct allocable a;
    int rv, err;

    memset(&a, 0, sizeof a);
    rv = do_screenshot(surf, &a);
    err = errno;

    if (a.rgbsurf) {
        SDL_UnlockSurface(a.rgbsurf);
        SDL_FreeSurface(a.rgbsurf);
    }
    if (a.png)
        png_destroy_write_struct(&a.png, &a.png_info);
    if (a.palette)
        free(a.palette);
    if (a.img)
        free(a.img);
    if (a.rowptrs)
        free(a.rowptrs);

    close_file(&a.hf);

    errno = err;
    return rv;
}
