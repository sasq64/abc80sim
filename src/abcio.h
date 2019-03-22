#ifndef ABCIO_H
#define ABCIO_H

#include "compiler.h"
#include "hostfile.h"

extern void io_init(void);

enum model
{
    MODEL_ABC80 = 0,
    MODEL_ABC802 = 1,
};

enum model_extra
{
    MODEL_ABC80_M40 = 2,
    MODEL_ABC802_M40 = 3,
};

extern enum model model;
extern unsigned int kilobytes;
extern bool old_basic;
extern bool startup_width40;

extern void abc80_mem_mode40(bool);
extern void abc80_mem_setmap(unsigned int);
extern void abc802_set_mem(bool);

extern void disk_reset(void);
extern void disk_out(int sel, int port, int value);
extern int disk_in(int sel, int port);

/* This is the "fake" ABCbus-connected RTC */
extern int rtc_in(int sel, int port);

/* ABC806 RTC */
extern uint8_t abc806_rtc_in(uint8_t port);
extern void abc806_rtc_out(uint8_t port, uint8_t value);

extern void abc800_ctc_out(uint8_t, uint8_t);
extern uint8_t abc800_ctc_in(uint8_t);
extern void abc800_ctc_init(void);

extern void printer_reset(void);
extern void printer_out(int sel, int port, int value);
extern int printer_in(int sel, int port);
extern void dart_pr_out(uint8_t port, uint8_t v);
extern uint8_t dart_pr_in(uint8_t port);

extern void keyboard_down(int sym);
extern unsigned int keyboard_up(void);
extern bool faketype;

extern void abc802_vsync(void);

extern void dump_memory(bool ramonly);

extern void abc80_piob_out(uint8_t port, uint8_t v);
extern uint8_t abc80_piob_in(void);
extern void abc80_cas_init(void);

extern void abc800_sio_cas_out(uint8_t port, uint8_t v);
extern uint8_t abc800_sio_cas_in(uint8_t port);
extern void abc800_cas_init(void);

/*
 * Z80 has fixed priorities based on the device daisy chain, but the vectors
 * can be different, so we identify interrupts by their priority level.
 */
enum abc80_irq
{
    IRQ80_PIOA,
    IRQ80_PIOB
};
enum abc800_irq
{
    IRQ800_DARTA,
    IRQ800_DARTB,
    IRQ800_SIOA,
    IRQ800_SIOB,
    IRQ800_CTC0,
    IRQ800_CTC1,
    IRQ800_CTC2,
    IRQ800_CTC3
};

/* Directory and filenames */
extern const char *fileop_path, *disk_path, *screen_path, *memdump_path,
    *cas_path;
extern struct file_list cas_files;

/* Program name for error messages */
extern const char* program_name;

#endif
