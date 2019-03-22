/*
 * Copyright (C) 1992 Clarendon Hill Software.
 *
 * Permission is granted to any individual or institution to use, copy,
 * or redistribute this software, provided this copyright notice is retained.
 *
 * This software is provided "as is" without any expressed or implied
 * warranty.  If this software brings on any sort of damage -- physical,
 * monetary, emotional, or brain -- too bad.  You've got no one to blame
 * but yourself.
 *
 * The software may be modified for your own purposes, but modified versions
 * must retain this notice.
 */

#ifndef Z80_H
#define Z80_H

#include "compiler.h"
#include "trace.h"

#include <SDL.h>

struct twobyte
{
#if WORDS_LITTLEENDIAN
    uint8_t low, high;
#else
    uint8_t high, low;
#endif
};

/* for implementing registers which can be seen as bytes or words: */
typedef union
{
    struct twobyte byte;
    uint16_t word;
} wordregister;

typedef void (*eoifunc)(uint8_t, void*);
struct eoi
{
    eoifunc func;
    void* arg;
    int trigger; /* Vector to call EOI for, otherwise -1 */
};

struct z80_irq;

struct z80_state_struct
{
    wordregister af;
    wordregister bc;
    wordregister de;
    wordregister hl;
    wordregister ix;
    wordregister iy;
    wordregister sp;
    wordregister pc;

    wordregister af_prime;
    wordregister bc_prime;
    wordregister de_prime;
    wordregister hl_prime;

    uint8_t i;  /* interrupt-page address register */
    uint8_t rc; /* counting part of register R (bits 6-0) */
    uint8_t rf; /* fixed part of register R (bit 7) */

    uint8_t interrupt_mode;
    bool iff1, iff2, ei_shadow, signal_eoi;

    bool nmi_in_progress;      /* to prevent multiple simultaneous NMIs */
    volatile bool nminterrupt; /* used to signal a non maskable interrupt */

    uint64_t tc; /* T-state (clock cycle) counter */
};

#define Z80_ADDRESS_LIMIT (1 << 16)

/*
 * Register accessors:
 */

#define REG_A (z80_state.af.byte.high)
#define REG_F (z80_state.af.byte.low)
#define REG_B (z80_state.bc.byte.high)
#define REG_C (z80_state.bc.byte.low)
#define REG_D (z80_state.de.byte.high)
#define REG_E (z80_state.de.byte.low)
#define REG_H (z80_state.hl.byte.high)
#define REG_L (z80_state.hl.byte.low)
#define REG_IXH (z80_state.ix.byte.high)
#define REG_IXL (z80_state.ix.byte.low)
#define REG_IYH (z80_state.iy.byte.high)
#define REG_IYL (z80_state.iy.byte.low)

#define REG_SP (z80_state.sp.word)
#define REG_PC (z80_state.pc.word)

#define REG_AF (z80_state.af.word)
#define REG_BC (z80_state.bc.word)
#define REG_DE (z80_state.de.word)
#define REG_HL (z80_state.hl.word)

#define REG_AF_PRIME (z80_state.af_prime.word)
#define REG_BC_PRIME (z80_state.bc_prime.word)
#define REG_DE_PRIME (z80_state.de_prime.word)
#define REG_HL_PRIME (z80_state.hl_prime.word)

#define REG_IX (z80_state.ix.word)
#define REG_IY (z80_state.iy.word)

#define REG_I (z80_state.i)
#define REG_R ((z80_state.rc & 0x7f) | (z80_state.rf & 0x80))

#define TSTATE z80_state.tc

/*
 * Flag accessors:
 *
 * Flags are:
 *
 *	7   6   5   4   3   2   1   0
 *	S   Z   -   H   -  P/V  N   C
 *
 *	C	Carry
 *	N	Subtract
 *	P/V	Parity/Overflow
 *	H	Half-carry
 *	Z	Zero
 *	S	Sign
 */

#define CARRY_MASK (0x1)
#define SUBTRACT_MASK (0x2)
#define PARITY_MASK (0x4)
#define OVERFLOW_MASK (0x4)
#define HALF_CARRY_MASK (0x10)
#define ZERO_MASK (0x40)
#define SIGN_MASK (0x80)
#define ALL_FLAGS_MASK                                                         \
    (CARRY_MASK | SUBTRACT_MASK | OVERFLOW_MASK | HALF_CARRY_MASK |            \
     ZERO_MASK | SIGN_MASK)

#define SET_SIGN() (REG_F |= SIGN_MASK)
#define CLEAR_SIGN() (REG_F &= (~SIGN_MASK))
#define SET_ZERO() (REG_F |= ZERO_MASK)
#define CLEAR_ZERO() (REG_F &= (~ZERO_MASK))
#define SET_HALF_CARRY() (REG_F |= HALF_CARRY_MASK)
#define CLEAR_HALF_CARRY() (REG_F &= (~HALF_CARRY_MASK))
#define SET_OVERFLOW() (REG_F |= OVERFLOW_MASK)
#define CLEAR_OVERFLOW() (REG_F &= (~OVERFLOW_MASK))
#define SET_PARITY() (REG_F |= PARITY_MASK)
#define CLEAR_PARITY() (REG_F &= (~PARITY_MASK))
#define SET_SUBTRACT() (REG_F |= SUBTRACT_MASK)
#define CLEAR_SUBTRACT() (REG_F &= (~SUBTRACT_MASK))
#define SET_CARRY() (REG_F |= CARRY_MASK)
#define CLEAR_CARRY() (REG_F &= (~CARRY_MASK))

#define SIGN_FLAG (REG_F & SIGN_MASK)
#define ZERO_FLAG (REG_F & ZERO_MASK)
#define HALF_CARRY_FLAG (REG_F & HALF_CARRY_MASK)
#define OVERFLOW_FLAG (REG_F & OVERFLOW_MASK)
#define PARITY_FLAG (REG_F & PARITY_MASK)
#define SUBTRACT_FLAG (REG_F & SUBTRACT_MASK)
#define CARRY_FLAG (REG_F & CARRY_MASK)
#define SIGN_FLAG (REG_F & SIGN_MASK)

extern struct z80_state_struct z80_state;
/* Signal an NMI */
static inline void z80_nmi(void)
{
    z80_state.nminterrupt = true;
}

extern void z80_reset(void);
extern int z80_run(bool, bool);
extern uint8_t mem_read(uint16_t);
extern uint8_t mem_fetch(uint16_t);
extern uint8_t mem_fetch_m1(uint16_t);
extern void mem_write(uint16_t, uint8_t);
extern uint8_t* mem_rom_address(void);
extern uint8_t* mem_get_addr(uint16_t);
extern uint16_t mem_read_word(uint16_t);
extern uint16_t mem_fetch_word(uint16_t);
extern void mem_write_word(uint16_t, uint16_t);
extern void tracemem(void);
extern void z80_out(int, uint8_t);
extern int z80_in(int);
extern int disassemble(int);
extern int DAsm(uint16_t pc, char* T, int* target);
extern bool z80_poll_external(void);

extern uint8_t ram[]; /* Array for plain RAM */

extern void mem_init(unsigned int flags, const char* memfile);
#define MEMFL_NOBASIC 1
#define MEMFL_NODEV 2

#endif /* Z80_H */
