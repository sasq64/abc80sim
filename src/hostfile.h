#ifndef HOSTFILE_H
#define HOSTFILE_H

#include "compiler.h"

static inline bool is_stdio(const char* filename)
{
    return !filename || !filename[0] || (filename[0] == '-' && !filename[1]);
}

enum host_file_mode
{
    HF_BINARY = 0,    /* Raw binary */
    HF_TEXT = 1,      /* Text mode compatible with ASCII */
    HF_UNICODE = 2,   /* Platform preferred Unicode encoding */
    HF_DIRECTORY = 3, /* opendir() on a directory */
    HF_TYPE_MASK = 0x0f,

    HF_PRIVATE = 0x10, /* Only user permissions */
    HF_RETRY = 0x20,   /* If O_RDWR retry with O_RDONLY on failure */
    HF_FAIL = 0x40     /* Don't actually try to open, return ENOENT */
};

struct host_file
{
    FILE* f;
    DIR* d;
    struct host_file **prevp, *next;
    off_t filesize;
    size_t namelen;
    int fd;
    int openflags;
    bool nuke;
    enum host_file_mode mode;
    uint8_t* map; /* Memory-mapped contents */
    size_t mlen;  /* Length of memory map */
    size_t flen;  /* Length of true file in memory map */
#ifdef __WIN32__
    HANDLE maphandle; /* Special Windows drain bramage */
#endif
    char filename[1];
};

/* This file should now be kept, not deleted on close */
static inline void keep_file(struct host_file* file)
{
    file->nuke = false;
}

static inline bool is_path_separator(char c)
{
    switch (c) {
    case '/':
#ifdef __WIN32__
    case ':':
    case '\\':
#endif
        return true;
    default:
        return false;
    }
}

#ifndef O_ACCMODE
#    define O_ACCMODE (O_RDONLY | O_WRONLY | O_RDWR) /* Hope this works */
#endif

static inline bool file_rdok(struct host_file* hf)
{
    return (hf->openflags & O_ACCMODE) != O_WRONLY;
}

static inline bool file_wrok(struct host_file* hf)
{
    return (hf->openflags & O_ACCMODE) != O_RDONLY;
}

/* Open a host filesystem file */
extern struct host_file* open_host_file(enum host_file_mode mode,
                                        const char* dir, const char* filename,
                                        int openflags);

/* Map (or read) the file contents into memory */
extern void* map_file(struct host_file* file, size_t len);

/* Create a numbered dump file */
extern struct host_file* dump_file(enum host_file_mode mode, const char* dir,
                                   const char* pattern);

/* Create a temporary file */
extern struct host_file* temp_file(enum host_file_mode mode,
                                   const char* prefix);

/* Write contents back to disk if necessary */
extern void flush_file(struct host_file* file);

/* Close and optionally delete a host file */
extern int close_file(struct host_file** temp);

/* Stat a combined path in the filesystem */
extern int stat_file(const char* dir, const char* filename, struct stat* st);

/* Initialize the hostfile subsystem */
extern void hostfile_init(void);

/* Point to a filename, without any path */
extern const char* host_strip_path(const char* path);

/* Simple linked list of filenames */
struct file_node;
struct file_list
{
    struct file_node *first, *last;
};

extern void filelist_add_file(struct file_list*, const char*);
extern void filelist_add_list(struct file_list*, const char*);
extern void filelist_free(struct file_list*);
extern char* filelist_pop(struct file_list*);

#endif /* HOSTFILE_H */
