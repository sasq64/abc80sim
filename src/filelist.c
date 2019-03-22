#include "compiler.h"
#include "hostfile.h"

struct file_node
{
    struct file_node* next;
    char* name;
};

static int filelist_add(struct file_list* list, char* name)
{
    struct file_node* node;

    node = malloc(sizeof *node);
    if (!node)
        return -1;

    node->name = name;
    node->next = NULL;
    if (list->last)
        list->last->next = node;
    else
        list->first = node;

    list->last = node;
    return 0;
}

void filelist_add_file(struct file_list* list, const char* filename)
{
    char* name = strdup(filename);

    if (!name)
        return;

    if (filelist_add(list, name))
        free(name);
}

void filelist_add_list(struct file_list* list, const char* listfile)
{
    FILE* f;
    char *listpath, *p;
    char linebuf[PATH_MAX];
    const char* dirsep;

    listpath = strdup(listfile);
    if (!listpath)
        return;

    p = (char*)host_strip_path(listpath);
    *p = '\0'; /* Remove the actual filename */
    dirsep = "";
    if (p > listpath && !is_path_separator(p[-1]))
        dirsep = "/";

    f = fopen(listfile, "rt");
    if (!f) {
        free(listpath);
        return;
    }

    while (fgets(linebuf, sizeof linebuf, f)) {
        p = strchr(linebuf, '\n');
        if (p)
            *p = '\0';

        asprintf(&p, "%s%s%s", listpath, dirsep, linebuf);
        if (!p)
            continue;

        if (filelist_add(list, p))
            free(p);
    }

    fclose(f);
    free(listpath);
}

void filelist_free(struct file_list* list)
{
    struct file_node *node, *nx;

    for (node = list->first; node; node = nx) {
        nx = node->next;
        free(node->name);
        free(node);
    }
    list->first = list->last = NULL;
}

char* filelist_pop(struct file_list* list)
{
    char* filename = NULL;
    struct file_node* node = list->first;

    if (node) {
        filename = node->name;
        list->first = node->next;
        if (list->last == node)
            list->last = NULL;
        free(node);
    }

    return filename;
}
