/*
 * File operations related to the host filesystem
 */

#include "hostfile.h"
#include "compiler.h"

#ifdef HAVE_SYS_MMAN_H
#    include <sys/mman.h>
#endif

static inline enum host_file_mode mode_type(enum host_file_mode mode)
{
    return mode & HF_TYPE_MASK;
}

#define PRIV_MODE (S_IRUSR | S_IWUSR)
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)
#define DIR_MODE (FILE_MODE | S_IXUSR | S_IXGRP | S_IXOTH)

#ifdef HAVE__MKDIR
#    define make_dir(x) _mkdir(x)
#else
#    define make_dir(x) mkdir((x), DIR_MODE)
#endif

#ifndef O_BINARY
#    define O_BINARY 0
#endif
#ifndef O_TEXT
#    define O_TEXT 0
#endif
#if defined(__WIN32__) && defined(_O_U16TEXT)
#    define UNICODE_O_FLAGS _O_U16TEXT
#else
#    define UNICODE_O_FLAGS O_TEXT
#endif

#ifndef O_NOFOLLOW
#    define O_NOFOLLOW 0
#endif
#ifndef O_SHORT_LIVED
#    define O_SHORT_LIVED 0
#endif
#ifndef O_DIRECTORY
#    define O_DIRECTORY 0
#endif

/* List of all host files */
static struct host_file* list;

/* Common routine to finish the job once we have a name and fd */
static struct host_file* finish_host_file(struct host_file* hf);

static inline int mode_openflags(enum host_file_mode mode)
{
    switch (mode_type(mode)) {
    case HF_BINARY:
        return O_BINARY;
    case HF_TEXT:
        return O_TEXT;
    case HF_UNICODE:
        return UNICODE_O_FLAGS;
    case HF_DIRECTORY:
        return O_DIRECTORY;
    default:
        return 0;
    }
}

#ifndef HAVE__SETMODE
#    define _setmode(x, y) ((void)(x), (void)(y))
#endif

static inline bool filename_is_absolute(const char* name)
{
    if (*name == '/')
        return true;

#ifdef __WIN32__
    if (*name == '\\' || strchr(name, ':'))
        return true;
#endif

    return false;
}

int stat_file(const char* dir, const char* filename, struct stat* st)
{
    size_t dl;
    char* path;
    int rv, err;

    if (!dir)
        dir = "";

    dl = strlen(dir);
    asprintf(&path, "%s%s%s", dir,
             (dl && !is_path_separator(dir[dl - 1])) ? "/" : "",
             filename ? filename : ".");
    if (!path)
        return -1;

    rv = stat(path, st);
    err = errno;
    free(path);
    errno = err;

    return rv;
}

/*
 * Common routine for opening a host file formed from a directory
 * and a file name
 */
struct host_file* open_host_file(enum host_file_mode mode, const char* dir,
                                 const char* filename, int openflags)
{
    size_t dl, fl;
    char* p;
    struct host_file* hf;

    if (!dir)
        dir = "";

    if (!filename) {
        filename = ".";
        if (mode_type(mode) != HF_DIRECTORY)
            mode = HF_FAIL;
    }

    if (mode & HF_FAIL) {
        errno = ENOENT;
        return NULL;
    }

    dl = strlen(dir);
    fl = strlen(filename);

    hf = calloc(sizeof *hf + dl + fl + 1, 1);
    if (!hf)
        return NULL;

    hf->fd = -1;
    hf->mode = mode;
    hf->openflags = openflags | mode_openflags(mode);
    hf->nuke = !!(openflags & O_EXCL);

    p = hf->filename;
    if (dl > 0 && !filename_is_absolute(filename)) {
        p = mempcpy(p, dir, dl);
        if (!is_path_separator(p[-1]))
            *p++ = '/';
    }
    p = mempcpy(p, filename, fl + 1);
    hf->namelen = p - hf->filename;

    if (mode_type(mode) == HF_DIRECTORY) {
        hf->d = opendir(hf->filename);
    } else {
        mode_t filemode = (mode & HF_PRIVATE) ? PRIV_MODE : FILE_MODE;
        for (;;) {
            hf->fd = open(hf->filename, hf->openflags, filemode);
            if (hf->fd >= 0 || !(mode & HF_RETRY))
                break;

            hf->openflags = (hf->openflags & ~O_ACCMODE) | O_RDONLY;
            hf->openflags &= ~(O_CREAT | O_EXCL | O_APPEND);
            mode &= ~HF_RETRY;
        }
    }

    return finish_host_file(hf);
}

/*
 * Create a numbered dump file for writing (only)
 */
struct host_file* dump_file(enum host_file_mode mode, const char* dir,
                            const char* pattern)
{
    int err;
    unsigned int n;
    struct host_file* hf;
    char* filename;
    const int openflags = O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW;

    if (!dir)
        dir = "";

    /* If it is a directory name, try to create it if it doesn't exist */
    if (dir[0])
        make_dir(dir);

    for (n = 1; n <= 9999; n++) {
        asprintf(&filename, pattern, n);
        if (!filename)
            return NULL;
        hf = open_host_file(mode, dir, filename, openflags);
        err = errno;
        free(filename);
        errno = err;

        if (hf || err != EEXIST)
            return hf;
    }

    return hf;
}

#ifdef HAVE_MKSTEMP

struct host_file* temp_file(enum host_file_mode mode, const char* prefix)
{
    struct host_file* hf = NULL;
    size_t pfxlen;

    mode |= HF_PRIVATE;

    if (!prefix)
        prefix = "";

    pfxlen = strlen(prefix);

    hf = calloc(sizeof *hf + pfxlen + 6, 1);
    if (!hf)
        return NULL;

    hf->mode = mode;
    hf->openflags = O_RDWR | O_CREAT | O_EXCL | O_NOFOLLOW | O_SHORT_LIVED;
    hf->nuke = true;
    hf->namelen = pfxlen + 6;
    memcpy(hf->filename, prefix, pfxlen);
    memcpy(hf->filename + pfxlen, "XXXXXX", 7);

    hf->fd = mkstemp(hf->filename);
    if (hf->fd < 0) {
        free(hf);
        return NULL;
    }
    _setmode(hf->fd, mode_openflags(mode));

    return finish_host_file(hf);
}

#else

/* Hack it with tmpnam() */

#    ifndef TMP_MAX
#        define TMP_MAX 65536
#    endif

struct host_file* temp_file(enum host_file_mode mode)
{
    char* filename = NULL;
    int err;
    int attempts = TMP_MAX;
    size_t namelen;
    const int openflags =
        O_RDWR | O_CREAT | O_EXCL | O_NOFOLLOW | O_SHORT_LIVED;

    mode |= HF_PRIVATE;

    do {
        filename = tempnam(NULL, TEMPFILE_PREFIX);
        if (!filename)
            return NULL;

        hf = open_host_file(mode, NULL, filename, openflags);
        err = errno;
        free(filename);
    } while (!hf && err == EEXIST && --attempts);

    errno = err;
    return hf;
}

#endif

/* Update hf->filesize, return -1 on failure */
static int update_filesize(struct host_file* hf)
{
    struct stat st;

    if (fstat(hf->fd, &st))
        return -1;

    hf->filesize = st.st_size;
    return 0;
}

/* Common routine to finish the job once we have a name and fd */
static struct host_file* finish_host_file(struct host_file* hf)
{
    const char* opt;

    if (!hf)
        return NULL;

    if (hf->mode == HF_DIRECTORY) {
        if (!hf->d)
            goto err;
    } else {
        if (hf->fd < 0)
            goto err;

        if (update_filesize(hf))
            goto err;

        switch (hf->openflags & O_ACCMODE) {
        case O_RDONLY:
            opt = "r";
            break;
        case O_WRONLY:
            opt = (hf->openflags & O_APPEND) ? "a" : "w";
            break;
        default:
            opt = (hf->openflags & O_APPEND)
                      ? "a+"
                      : (hf->openflags & (O_CREAT | O_TRUNC)) ? "w+" : "r+";
            break;
        }

        hf->f = fdopen(hf->fd, opt);
        if (!hf->f)
            goto err;
    }

    hf->next = list;
    hf->prevp = &list;
    if (list)
        list->prevp = &hf->next;
    list = hf;

    return hf;

err:
    close_file(&hf);
    return NULL;
}

/*
 * Page size, on systems that support such a thing
 */
static size_t page_mask;

static inline size_t page_size(void)
{
#ifdef __WIN32__
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return si.dwPageSize;
#elif defined(HAVE_SYSCONF) && defined(_SC_PAGESIZE)
    return sysconf(_SC_PAGESIZE);
#elif defined(HAVE_SYSCONF) && defined(_SC_PAGE_SIZE)
    return sysconf(_SC_PAGESIZE);
#elif defined(HAVE_GETPAGESIZE)
    return getpagesize();
#elif defined(PAGE_SIZE)
    return PAGE_SIZE;
#else
    return 1; /* Bogus, but maybe good enough */
#endif
}

#ifdef HAVE_FTRUNCATE
#    define set_file_size(fd, size) ftruncate(fd, size)
#elif defined(HAVE__CHSIZE_S)
#    define set_file_size(fd, size) _chsize_s(fd, size)
#elif defined(HAVE__CHSIZE)
#    define set_file_size(fd, size) _chsize(fd, size)
#else
static int set_file_size(int fd, size_t size)
{
    (void)fd;
    (void)size;
    return -1;
}
#endif

/*
 * Wrappers around mmap, munmap and msync.  The Windows API for this
 * requires some additional storage.
 */
#ifdef HAVE_MMAP

static void* do_map_file(struct host_file* hf)
{
    int prot;
    size_t flen;
    size_t mlen = hf->mlen;
    uint8_t* map;

    prot = (file_rdok(hf) ? PROT_READ : 0) | (file_wrok(hf) ? PROT_WRITE : 0);
    flen = (hf->flen + page_mask) & ~page_mask;

    /*
     * Use mlen here, even if the file is too small.  Otherwise we may not
     * reserve enough address space.
     */
    map = mmap(NULL, mlen, prot, MAP_SHARED, fileno(hf->f), 0);
    if (map == MAP_FAILED)
        return NULL;

    /*
     * If the file is smaller than we wanted (due to being readonly,
     * lack of space or so on), then try to mmap anonymous memory
     * over the rest of the mapping to avoid SIGBUS.
     */
    if (flen < mlen)
        mmap(map + flen, mlen - flen, prot,
             MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);

    return hf->map = map;
}

static void do_unmap_file(struct host_file* hf)
{
    if (!hf->map)
        return;

    munmap(hf->map, hf->mlen);
    hf->map = NULL;
}

static void do_msync_file(struct host_file* hf)
{
#    ifdef HAVE_MSYNC
    if (!hf->map || !file_wrok(hf))
        return;

    msync(hf->map, hf->mlen, MS_SYNC);
#    else
    (void)hf;
#    endif
}

#elif defined(__WIN32__)

static void* do_map_file(struct host_file* hf)
{
    HANDLE hfile = (HANDLE)_get_osfhandle(fileno(hf->f));
    HANDLE mapping;
    void* map;
    DWORD prot;

    mapping = CreateFileMapping(
        hfile, NULL, file_wrok(hf) ? PAGE_READWRITE : PAGE_READONLY,
        (DWORD)((uint64_t)hf->mlen >> 32), (DWORD)(hf->mlen), NULL);
    if (!mapping)
        return NULL;

    prot = (file_rdok(hf) ? FILE_MAP_READ : 0) |
           (file_wrok(hf) ? FILE_MAP_WRITE : 0);
    map = MapViewOfFile(mapping, prot, 0, 0, hf->mlen);

    if (!map) {
        CloseHandle(mapping);
        return NULL;
    }

    hf->maphandle = mapping;
    return (hf->map = map);
}

static void do_unmap_file(struct host_file* hf)
{
    if (!hf->map)
        return;

    UnmapViewOfFile(hf->map);
    hf->map = NULL;

    CloseHandle(hf->maphandle);
}

static void do_msync_file(struct host_file* hf)
{
    if (!hf->map || !file_wrok(hf))
        return;

    FlushViewOfFile(hf->map, hf->mlen);
}

#else /* No memory mapping technique known */

static void* do_map_file(struct host_file* hf)
{
    (void)hf;
    return NULL;
}

static void do_unmap_file(struct host_file* hf)
{
    (void)hf;
}

static void do_msync_file(struct host_file* hf)
{
    (void)hf;
}

#endif

/*
 * Map a file into memory, if possible; otherwise create a memory buffer
 * containing the full file contents that gets written back on flush_file()
 * or close_file().
 */
#define MAX_MAP_FILE ((off_t)128 * 1024 * 1024)

void* map_file(struct host_file* hf, size_t mlen)
{
    size_t page_mask;
    off_t mleno;

    if (!hf || !hf->f || mode_type(hf->mode) != HF_BINARY)
        return NULL; /* Not a mappable file */

    if (hf->map)
        return hf->map; /* Already mapped */

    if (update_filesize(hf))
        return NULL;

    mleno = mlen;
    if (!mleno)
        mleno = hf->filesize;
    mleno = mlen = (mleno > MAX_MAP_FILE) ? MAX_MAP_FILE : (size_t)mleno;

    if (hf->filesize < mleno && file_wrok(hf)) {
        set_file_size(hf->fd, mleno); /* Try to extend file */
        if (update_filesize(hf))
            return NULL;
    }

    hf->flen = (hf->filesize < mleno) ? (size_t)hf->filesize : mlen;

    /* Round up to a size in pages */
    page_mask = page_size() - 1;
    hf->mlen = (mlen + page_mask) & ~page_mask;

    return do_map_file(hf);
}

void flush_file(struct host_file* hf)
{
    if (!hf || !hf->f || !file_wrok(hf))
        return; /* Nothing to sync */

    fflush(hf->f);
    do_msync_file(hf);
}

/* This function returns errno on failure, the errno variable is preserved */
int close_file(struct host_file** filep)
{
    struct host_file* file;
    int old_errno = errno;
    int err = 0;

    if (!filep || !(file = *filep))
        return 0;

    if (file->prevp) {
        *file->prevp = file->next; /* Remove from linked list */
        if (file->next)
            file->next->prevp = file->prevp;
    }

    if (file->d) {
        if (closedir(file->d))
            err = err ? err : errno;
    } else {
        flush_file(file);
        do_unmap_file(file);

        if (file->f) {
            if (fclose(file->f))
                err = err ? err : errno;
            else
                file->fd = -1; /* fclose() closes the file descriptor too */
        }

        if (file->fd >= 0) {
            if (close(file->fd))
                err = err ? err : errno;
        }

        if (file->nuke && file->fd >= 0 && file->filename[0]) {
            if (remove(file->filename))
                err = err ? err : errno;
        }
    }

    free(file);
    *filep = NULL;
    errno = old_errno;

    return err;
}

static void hostfile_cleanup(void)
{
    struct host_file *hf, *next;
    for (hf = list; hf; hf = next) {
        next = hf->next;
        close_file(&hf);
    }
}

void hostfile_init(void)
{
    atexit(hostfile_cleanup);
    page_mask = page_size() - 1;
}

/*
 * Strip the path from a (host) filename
 */
const char* host_strip_path(const char* path)
{
    const char* p;

    p = strrchr(path, '\0');
    while (--p >= path) {
        if (is_path_separator(*p))
            break;
    }

    return p + 1;
}
