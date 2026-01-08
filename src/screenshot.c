/*
 * Take a PNG screenshot of an SDL surface
 */

#include <SDL_image.h>

#include "screenshot.h"
#include "abcio.h"
#include "compiler.h"
#include "hostfile.h"


const char* screen_path;

int screenshot(SDL_Surface* surf)
{
    struct host_file *hv = dump_file(HF_BINARY, screen_path, "scrn%04u.png");
    if (!hv || !hv->f) {
        fprintf(stderr, "Failed to find a free screenshot filename in %s\n",
                screen_path);
        return 0;
    }

    char filename[1025];
    strncpy(filename, hv->filename, sizeof(filename));

    //SDL_LockSurface(surf);
    int res = IMG_SavePNG(surf, filename) > -1;
    //SDL_UnlockSurface(surf);

    keep_file(hv);
    close_file(&hv);

    return res;
}
