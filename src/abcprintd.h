#ifndef ABCPRINTD_H
#define ABCPRINTD_H

#include "compiler.h"

extern void abcprint_init(void);
extern void abcprint_recv(const void*, size_t);
extern void abcprint_send(const void*, size_t);
extern bool file_op(unsigned char);
extern int open_serial_port(const char* path, unsigned int speed, bool flow);
extern const char *fileop_path, *lpr_command;
extern FILE* console_file;

#endif
