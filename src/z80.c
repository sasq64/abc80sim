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

/*
 * z80.c:  The guts of the Z-80 emulator.
 *
 * The Z-80 emulator should be general and complete enough to be easily
 * adapted to emulate any Z-80 machine, although it's only really been tested
 * with TRS-80 code.  The only thing we cheat a little on is interrupt
 * handling and the refresh register.  All of the flags are supported.
 * All of the documented Z-80 instructions are implemented.
 *
 * There are undobutedly bugs in the emulator.  If you discover any,
 * please do send a report.
 */
#include "z80.h"
#include "z80irq.h"

/*
 * The state of our Z-80 registers is kept in this structure:
 */
struct z80_state_struct z80_state;

static void diffstate(void);

/*
 * T-states (clock cycles) for various instructions.
 * This reflects the base clock count; in particular:
 * - Conditional JR, CALL, DJNZ, RET not taken
 * - Block instructions not repeated
 * - HALT instruction does not loop
 *
 * Prefix opcodes count as 4 cycles for the prefix itself
 */

/* Main opcode group */
// clang-format off
static const uint8_t clk_main[256] = {
    /*         0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f */
    /* 00 */   4, 10,  7,  6,  4,  4,  7,  4,  4, 11,  7,  6,  4,  4,  7,  4,
    /* 10 */   8, 10,  7,  6,  4,  4,  7,  4, 12, 11,  7,  6,  4,  4,  7,  4,
    /* 20 */   7, 10, 16,  6,  4,  4,  7,  4,  7, 11, 16,  6,  4,  4,  7,  4,
    /* 30 */   7, 10, 13,  6, 11, 11, 10,  4,  7, 11, 13,  6,  4,  4,  7,  4,
    /* 40 */   4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,
    /* 50 */   4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,
    /* 60 */   4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,
    /* 70 */   7,  7,  7,  7,  7,  7,  4,  7,  4,  4,  4,  4,  4,  4,  7,  4,
    /* 80 */   4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,
    /* 90 */   4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,
    /* a0 */   4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,
    /* b0 */   4,  4,  4,  4,  4,  4,  7,  4,  4,  4,  4,  4,  4,  4,  7,  4,
    /* c0 */   5, 10, 10, 10, 10, 11,  7, 11,  5, 10, 10,  4, 10, 17,  7, 11,
    /* d0 */   5, 10, 10, 11, 10, 11,  7, 11,  5,  4, 10, 11, 10,  4,  7, 11,
    /* e0 */   5, 10, 10, 19, 10, 11,  7, 11,  5,  4, 10,  4, 10,  4,  7, 11,
    /* f0 */   5, 10, 10,  4, 10, 11,  7, 11,  5,  6, 10,  4, 10,  4,  7, 11,
};

/* EB opcode group - not including 4 cycles for the EB prefix itself */
#define X 4                        /* Believed to be NOPs with this timing */
static const uint8_t clk_ED[256] = {
    /*         0   1   2   3   4   5   6   7   8   9   a   b   c   d   e   f */
    /* 00 */   X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,
    /* 10 */   X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,
    /* 20 */   X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,
    /* 30 */   X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,
    /* 40 */   8,  8, 11, 16,  4, 10,  4,  5,  8,  8, 11, 16,  4, 10,  4,  5,
    /* 50 */   8,  8, 11, 16,  4, 10,  4,  5,  8,  8, 11, 16,  4, 10,  4,  5,
    /* 60 */   8,  8, 11, 16,  4, 10,  4, 14,  8,  8, 11, 16,  4, 10,  4, 14,
    /* 70 */   8,  8, 11, 16,  4, 10,  4,  X,  8,  8, 11, 16,  4, 10,  4,  X,
    /* 80 */   X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,
    /* 90 */   X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,
    /* a0 */  12, 12, 12, 12,  X,  X,  X,  X, 12, 12, 12, 12,  X,  X,  X,  X,
    /* b0 */  12, 12, 12, 12,  X,  X,  X,  X, 12, 12, 12, 12,  X,  X,  X,  X,
    /* c0 */   X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,
    /* d0 */   X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,
    /* e0 */   X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,
    /* f0 */   X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,  X,
};
#undef X
// clang-format on

/*
 * Tables and routines for computing various flag values:
 */

static const uint8_t sign_carry_overflow_table[] = {
    0,
    OVERFLOW_MASK | SIGN_MASK,
    CARRY_MASK,
    SIGN_MASK,
    CARRY_MASK,
    SIGN_MASK,
    CARRY_MASK | OVERFLOW_MASK,
    CARRY_MASK | SIGN_MASK,
};

static const uint8_t half_carry_table[] = {
    0,
    0,
    HALF_CARRY_MASK,
    0,
    HALF_CARRY_MASK,
    0,
    HALF_CARRY_MASK,
    HALF_CARRY_MASK,
};

static const uint8_t subtract_sign_carry_overflow_table[] = {
    0,
    CARRY_MASK | SIGN_MASK,
    CARRY_MASK,
    OVERFLOW_MASK | CARRY_MASK | SIGN_MASK,
    OVERFLOW_MASK,
    SIGN_MASK,
    0,
    CARRY_MASK | SIGN_MASK,
};

static const uint8_t subtract_half_carry_table[] = {
    0, HALF_CARRY_MASK, HALF_CARRY_MASK, HALF_CARRY_MASK, 0, 0,
    0, HALF_CARRY_MASK,
};

static int parity(unsigned value)
{

    // clang-format off
    /* for parity flag, 1 = even parity, 0 = odd parity. */
    static const char parity_table[256] =
    {
        1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
        0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
        0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
        1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
        0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
        1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
        1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
        0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
        0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
        1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
        1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
        0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
        1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
        0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
        0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
        1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1
    };
    // clang-format on

    return (parity_table[value]);
}

static void add_r(uint8_t jump)
{
    z80_state.rc += jump;
}

static void inc_r(void)
{
    add_r(1);
}

static void do_add_flags(int a, int b, int result)
{
    /*
     * Compute the flag values for a + b = result operation
     */
    int index;
    int f;

    /*
     * sign, carry, and overflow depend upon values of bit 7.
     * half-carry depends upon values of bit 3.
     * We mask those bits, munge them into an index, and look
     * up the flag values in the above tables.
     */

    f = REG_F & ~(SIGN_MASK | ZERO_MASK | HALF_CARRY_MASK | OVERFLOW_MASK |
                  SUBTRACT_MASK | CARRY_MASK);

    index = ((a & 0x88) >> 1) | ((b & 0x88) >> 2) | ((result & 0x88) >> 3);
    f |= half_carry_table[index & 7] | sign_carry_overflow_table[index >> 4];

    if ((result & 0xFF) == 0)
        f |= ZERO_MASK;

    REG_F = f;
}

static void do_sub_flags(int a, int b, int result)
{
    int index;
    int f;

    /*
     * sign, carry, and overflow depend upon values of bit 7.
     * half-carry depends upon values of bit 3.
     * We mask those bits, munge them into an index, and look
     * up the flag values in the above tables.
     */

    f = (REG_F | SUBTRACT_MASK) &
        ~(SIGN_MASK | ZERO_MASK | HALF_CARRY_MASK | OVERFLOW_MASK | CARRY_MASK);

    index = ((a & 0x88) >> 1) | ((b & 0x88) >> 2) | ((result & 0x88) >> 3);
    f |= subtract_half_carry_table[index & 7] |
         subtract_sign_carry_overflow_table[index >> 4];

    if ((result & 0xFF) == 0)
        f |= ZERO_MASK;

    REG_F = f;
}

static void do_adc_word_flags(int a, int b, int result)
{
    int index;
    int f;

    /*
     * sign, carry, and overflow depend upon values of bit 15.
     * half-carry depends upon values of bit 11.
     * We mask those bits, munge them into an index, and look
     * up the flag values in the above tables.
     */

    f = REG_F & ~(SIGN_MASK | ZERO_MASK | HALF_CARRY_MASK | OVERFLOW_MASK |
                  SUBTRACT_MASK | CARRY_MASK);

    index =
        ((a & 0x8800) >> 9) | ((b & 0x8800) >> 10) | ((result & 0x8800) >> 11);

    f |= half_carry_table[index & 7] | sign_carry_overflow_table[index >> 4];

    if ((result & 0xFFFF) == 0)
        f |= ZERO_MASK;

    REG_F = f;
}

static void do_add_word_flags(int a, int b, int result)
{
    int index;
    int f;

    /*
     * carry depends upon values of bit 15.
     * half-carry depends upon values of bit 11.
     * We mask those bits, munge them into an index, and look
     * up the flag values in the above tables.
     */

    f = REG_F & ~(HALF_CARRY_MASK | SUBTRACT_MASK | CARRY_MASK);

    index =
        ((a & 0x8800) >> 9) | ((b & 0x8800) >> 10) | ((result & 0x8800) >> 11);

    f |= half_carry_table[index & 7] |
         (sign_carry_overflow_table[index >> 4] & CARRY_MASK);

    REG_F = f;
}

static void do_sbc_word_flags(int a, int b, int result)
{
    int index;
    int f;

    /*
     * sign, carry, and overflow depend upon values of bit 15.
     * half-carry depends upon values of bit 11.
     * We mask those bits, munge them into an index, and look
     * up the flag values in the above tables.
     */

    f = (REG_F | SUBTRACT_MASK) &
        ~(SIGN_MASK | ZERO_MASK | HALF_CARRY_MASK | OVERFLOW_MASK | CARRY_MASK);

    index =
        ((a & 0x8800) >> 9) | ((b & 0x8800) >> 10) | ((result & 0x8800) >> 11);

    f |= subtract_half_carry_table[index & 7] |
         subtract_sign_carry_overflow_table[index >> 4];

    if ((result & 0xFFFF) == 0)
        f |= ZERO_MASK;

    REG_F = f;
}

static void do_flags_dec_byte(int value)
{
    uint8_t clear, set;

    clear = OVERFLOW_MASK | HALF_CARRY_MASK | ZERO_MASK | SIGN_MASK;
    set = SUBTRACT_MASK;

    if (value == 0x7f)
        set |= OVERFLOW_MASK;
    if ((value & 0xF) == 0xF)
        set |= HALF_CARRY_MASK;
    if (value == 0)
        set |= ZERO_MASK;
    if (value & 0x80)
        set |= SIGN_MASK;

    REG_F = (REG_F & ~clear) | set;
}

static void do_flags_inc_byte(int value)
{
    uint8_t clear, set;

    clear =
        SUBTRACT_MASK | OVERFLOW_MASK | HALF_CARRY_MASK | ZERO_MASK | SIGN_MASK;
    set = 0;

    if (value == 0x80)
        set |= OVERFLOW_MASK;
    if ((value & 0xF) == 0)
        set |= HALF_CARRY_MASK;
    if (value == 0)
        set |= ZERO_MASK;
    if (value & 0x80)
        set |= SIGN_MASK;

    REG_F = (REG_F & ~clear) | set;
}

/*
 * Routines for executing or assisting various non-trivial arithmetic
 * instructions:
 */
static void do_and_byte(int value)
{
    int result;
    uint8_t clear, set;

    result = (REG_A &= value);

    clear = CARRY_MASK | SUBTRACT_MASK | PARITY_MASK | ZERO_MASK | SIGN_MASK;
    set = HALF_CARRY_MASK;

    if (parity(result))
        set |= PARITY_MASK;
    if (result == 0)
        set |= ZERO_MASK;
    if (result & 0x80)
        set |= SIGN_MASK;

    REG_F = (REG_F & ~clear) | set;
}

static void do_or_byte(int value)
{
    int result; /* the result of the or operation */
    uint8_t clear, set;

    result = (REG_A |= value);

    clear = CARRY_MASK | SUBTRACT_MASK | PARITY_MASK | HALF_CARRY_MASK |
            ZERO_MASK | SIGN_MASK;
    set = 0;

    if (parity(result))
        set |= PARITY_MASK;
    if (result == 0)
        set |= ZERO_MASK;
    if (result & 0x80)
        set |= SIGN_MASK;

    REG_F = (REG_F & ~clear) | set;
}

static void do_xor_byte(int value)
{
    int result; /* the result of the xor operation */
    uint8_t clear, set;

    result = (REG_A ^= value);

    clear = CARRY_MASK | SUBTRACT_MASK | PARITY_MASK | HALF_CARRY_MASK |
            ZERO_MASK | SIGN_MASK;
    set = 0;

    if (parity(result))
        set |= PARITY_MASK;
    if (result == 0)
        set |= ZERO_MASK;
    if (result & 0x80)
        set |= SIGN_MASK;

    REG_F = (REG_F & ~clear) | set;
}

static void do_add_byte(int value)
{
    int a, result;

    result = (a = REG_A) + value;
    REG_A = result;
    do_add_flags(a, value, result);
}

static void do_adc_byte(int value)
{
    int a, result;

    if (CARRY_FLAG)
        result = (a = REG_A) + value + 1;
    else
        result = (a = REG_A) + value;
    REG_A = result;
    do_add_flags(a, value, result);
}

static void do_sub_byte(int value)
{
    int a, result;

    result = (a = REG_A) - value;
    REG_A = result;
    do_sub_flags(a, value, result);
}

static void do_negate(void)
{
    int a;

    a = REG_A;
    REG_A = -a;
    do_sub_flags(0, a, REG_A);
    if (a == 0)
        REG_F |= CARRY_MASK;
}

static void do_sbc_byte(int value)
{
    int a, result;

    if (CARRY_FLAG)
        result = (a = REG_A) - (value + 1);
    else
        result = (a = REG_A) - value;
    REG_A = result;
    do_sub_flags(a, value, result);
}

static void do_adc_word(int value)
{
    int a, result;

    if (CARRY_FLAG)
        result = (a = REG_HL) + value + 1;
    else
        result = (a = REG_HL) + value;

    REG_HL = result;

    do_adc_word_flags(a, value, result);
}

static void do_sbc_word(int value)
{
    int a, result;

    if (CARRY_FLAG)
        result = (a = REG_HL) - (value + 1);
    else
        result = (a = REG_HL) - value;

    REG_HL = result;

    do_sbc_word_flags(a, value, result);
}

static void do_add_word(wordregister* ix, int value)
{
    int a, result;

    result = (a = ix->word) + value;
    ix->word = result;

    do_add_word_flags(a, value, result);
}

static void do_cp(int value) /* compare this value with A's contents */
{
    int a, result;

    result = (a = REG_A) - value;
    do_sub_flags(a, value, result);
}

/* dir == 1 for CPI, -1 for CPD */
static void do_cpid(int dir)
{
    do_cp(mem_read(REG_HL));
    REG_HL += dir;
    REG_BC--;

    if (REG_BC == 0)
        CLEAR_OVERFLOW();
    else
        SET_OVERFLOW();
}

/* dir == 1 for CPIR, -1 for CPDR */
static void do_cpidr(int dir)
{
    do_cpid(dir);

    if (REG_BC != 0 && !ZERO_FLAG) {
        TSTATE += 5;
        REG_PC -= 2;
    }
}

static void do_test_bit(int value, int bit)
{
    uint8_t clear, set;

    clear = SIGN_MASK | ZERO_MASK | OVERFLOW_MASK | SUBTRACT_MASK;
    set = HALF_CARRY_MASK;

    if ((value & (1 << bit)) == 0)
        set |= ZERO_MASK;

    REG_F = (REG_F & ~clear) | set;
}

static int rl_byte(int value)
{
    /*
     * Compute rotate-left-through-carry
     * operation, setting flags as appropriate.
     */

    uint8_t clear, set;
    int result;

    clear = SIGN_MASK | ZERO_MASK | HALF_CARRY_MASK | PARITY_MASK |
            SUBTRACT_MASK | CARRY_MASK;
    set = 0;

    if (CARRY_FLAG) {
        result = ((value << 1) & 0xFF) | 1;
    } else {
        result = (value << 1) & 0xFF;
    }

    if (result & 0x80)
        set |= SIGN_MASK;
    if (result == 0)
        set |= ZERO_MASK;
    if (parity(result))
        set |= PARITY_MASK;
    if (value & 0x80)
        set |= CARRY_MASK;

    REG_F = (REG_F & ~clear) | set;

    return result;
}

static int rr_byte(int value)
{
    /*
     * Compute rotate-right-through-carry
     * operation, setting flags as appropriate.
     */

    uint8_t clear, set;
    int result;

    clear = SIGN_MASK | ZERO_MASK | HALF_CARRY_MASK | PARITY_MASK |
            SUBTRACT_MASK | CARRY_MASK;
    set = 0;

    if (CARRY_FLAG) {
        result = (value >> 1) | 0x80;
    } else {
        result = (value >> 1);
    }

    if (result & 0x80)
        set |= SIGN_MASK;
    if (result == 0)
        set |= ZERO_MASK;
    if (parity(result))
        set |= PARITY_MASK;
    if (value & 0x1)
        set |= CARRY_MASK;

    REG_F = (REG_F & ~clear) | set;

    return result;
}

static int rlc_byte(int value)
{
    /*
     * Compute the result of an RLC operation and set the flags appropriately.
     * This does not do the right thing for the RLCA instruction.
     */

    uint8_t clear, set;
    int result;

    clear = SIGN_MASK | ZERO_MASK | HALF_CARRY_MASK | PARITY_MASK |
            SUBTRACT_MASK | CARRY_MASK;
    set = 0;

    if (value & 0x80) {
        result = ((value << 1) & 0xFF) | 1;
        set |= CARRY_MASK;
    } else {
        result = (value << 1) & 0xFF;
    }

    if (result & 0x80)
        set |= SIGN_MASK;
    if (result == 0)
        set |= ZERO_MASK;
    if (parity(result))
        set |= PARITY_MASK;

    REG_F = (REG_F & ~clear) | set;

    return result;
}

static int rrc_byte(int value)
{
    uint8_t clear, set;
    int result;

    clear = SIGN_MASK | ZERO_MASK | HALF_CARRY_MASK | PARITY_MASK |
            SUBTRACT_MASK | CARRY_MASK;
    set = 0;

    if (value & 0x1) {
        result = (value >> 1) | 0x80;
        set |= CARRY_MASK;
    } else {
        result = (value >> 1); /* fixed /jonas-y */
    }

    if (result & 0x80)
        set |= SIGN_MASK;
    if (result == 0)
        set |= ZERO_MASK;
    if (parity(result))
        set |= PARITY_MASK;

    REG_F = (REG_F & ~clear) | set;

    return result;
}

/*
 * Perform the RLA, RLCA, RRA, RRCA instructions.  These set the flags
 * differently than the other rotate instrucitons.
 */
static void do_rla(void)
{
    uint8_t clear, set;

    clear = HALF_CARRY_MASK | SUBTRACT_MASK | CARRY_MASK;
    set = 0;

    if (REG_A & 0x80)
        set |= CARRY_MASK;

    if (CARRY_FLAG) {
        REG_A = ((REG_A << 1) & 0xFF) | 1;
    } else {
        REG_A = (REG_A << 1) & 0xFF;
    }

    REG_F = (REG_F & ~clear) | set;
}

static void do_rra(void)
{
    uint8_t clear, set;

    clear = HALF_CARRY_MASK | SUBTRACT_MASK | CARRY_MASK;
    set = 0;

    if (REG_A & 0x1)
        set |= CARRY_MASK;

    if (CARRY_FLAG) {
        REG_A = (REG_A >> 1) | 0x80;
    } else {
        REG_A = REG_A >> 1;
    }
    REG_F = (REG_F & ~clear) | set;
}

static void do_rlca(void)
{
    uint8_t clear, set;

    clear = HALF_CARRY_MASK | SUBTRACT_MASK | CARRY_MASK;
    set = 0;

    if (REG_A & 0x80) {
        REG_A = ((REG_A << 1) & 0xFF) | 1;
        set |= CARRY_MASK;
    } else {
        REG_A = (REG_A << 1) & 0xFF;
    }
    REG_F = (REG_F & ~clear) | set;
}

static void do_rrca(void)
{
    uint8_t clear, set;

    clear = HALF_CARRY_MASK | SUBTRACT_MASK | CARRY_MASK;
    set = 0;

    if (REG_A & 0x1) {
        REG_A = (REG_A >> 1) | 0x80;
        set |= CARRY_MASK;
    } else {
        REG_A = REG_A >> 1;
    }
    REG_F = (REG_F & ~clear) | set;
}

static int sla_byte(int value)
{
    uint8_t clear, set;
    int result;

    clear = SIGN_MASK | ZERO_MASK | HALF_CARRY_MASK | PARITY_MASK |
            SUBTRACT_MASK | CARRY_MASK;
    set = 0;

    result = value << 1;

    if (result & 0x80)
        set |= SIGN_MASK;
    if (result == 0)
        set |= ZERO_MASK;
    if (parity(result))
        set |= PARITY_MASK;
    if (value & 0x80)
        set |= CARRY_MASK;

    REG_F = (REG_F & ~clear) | set;

    return result;
}

/* SLL is an undocumented instruction which shifts left and sets the LSB */
static int sll_byte(int value)
{
    uint8_t clear, set;
    int result;

    clear = SIGN_MASK | ZERO_MASK | HALF_CARRY_MASK | PARITY_MASK |
            SUBTRACT_MASK | CARRY_MASK;
    set = 0;

    result = (value << 1) | 1;

    if (result & 0x80)
        set |= SIGN_MASK;
    if (result == 0)
        set |= ZERO_MASK;
    if (parity(result))
        set |= PARITY_MASK;
    if (value & 0x80)
        set |= CARRY_MASK;

    REG_F = (REG_F & ~clear) | set;

    return result;
}

static int sra_byte(int value)
{
    uint8_t clear, set;
    int result;

    clear = SIGN_MASK | ZERO_MASK | HALF_CARRY_MASK | PARITY_MASK |
            SUBTRACT_MASK | CARRY_MASK;
    set = 0;

    if (value & 0x80) {
        result = (value >> 1) & 0x80;
        set |= SIGN_MASK;
    } else {
        result = value >> 1;
    }

    if (result == 0)
        set |= ZERO_MASK;
    if (parity(result))
        set |= PARITY_MASK;
    if (value & 0x1)
        set |= CARRY_MASK;

    REG_F = (REG_F & ~clear) | set;

    return result;
}

static int srl_byte(int value)
{
    uint8_t clear, set;
    int result;

    clear = SIGN_MASK | ZERO_MASK | HALF_CARRY_MASK | PARITY_MASK |
            SUBTRACT_MASK | CARRY_MASK;
    set = 0;

    result = value >> 1;

    if (result & 0x80)
        set |= SIGN_MASK;
    if (result == 0)
        set |= ZERO_MASK;
    if (parity(result))
        set |= PARITY_MASK;
    if (value & 0x1)
        set |= CARRY_MASK;

    REG_F = (REG_F & ~clear) | set;

    return result;
}

static void do_ldid(int dir)
{
    mem_write(REG_DE, mem_read(REG_HL));
    REG_DE += dir;
    REG_HL += dir;
    REG_BC--;

    CLEAR_HALF_CARRY();
    CLEAR_SUBTRACT();
    if (REG_BC == 0)
        CLEAR_OVERFLOW();
    else
        SET_OVERFLOW();
}

static void do_ldidr(int dir)
{
    do_ldid(dir);

    if (REG_BC != 0) {
        TSTATE += 5;
        REG_PC -= 2;
    }
}

static void do_ld_a_ir(uint8_t val)
{
    uint8_t clear, set;

    clear =
        SIGN_MASK | ZERO_MASK | HALF_CARRY_MASK | OVERFLOW_MASK | SUBTRACT_MASK;
    set = 0;

    REG_A = val;

    if (REG_A & 0x80)
        set |= SIGN_MASK;
    if (REG_A == 0)
        set |= ZERO_MASK;

    if (z80_state.iff2)
        set |= OVERFLOW_MASK;

    REG_F = (REG_F & ~clear) | set;
}

static void do_daa(void)
{
    /*
     * The bizzare decimal-adjust-accumulator instruction....
     */

    int high_nibble, low_nibble, add, carry, subtract_flag;

    high_nibble = REG_A >> 4;
    low_nibble = REG_A & 0xf;
    subtract_flag = SUBTRACT_FLAG;

    if (subtract_flag == 0) /* add, adc, inc */
    {
        if (CARRY_FLAG == 0) /* no carry */
        {
            if (HALF_CARRY_FLAG == 0) /* no half-carry */
            {
                if (low_nibble < 10) {
                    if (high_nibble < 10) {
                        add = 0x00;
                        carry = 0;
                    } else {
                        add = 0x60;
                        carry = 1;
                    }
                } else {
                    if (high_nibble < 9) {
                        add = 0x06;
                        carry = 0;
                    } else {
                        add = 0x66;
                        carry = 1;
                    }
                }
            } else /* half-carry */
            {
                if (high_nibble < 10) {
                    add = 0x06;
                    carry = 0;
                } else {
                    add = 0x66;
                    carry = 1;
                }
            }
        } else /* carry */
        {
            if (HALF_CARRY_FLAG == 0) /* no half-carry */
            {
                if (low_nibble < 10) {
                    add = 0x60;
                    carry = 1;
                } else {
                    add = 0x66;
                    carry = 1;
                }
            } else /* half-carry */
            {
                add = 0x66;
                carry = 1;
            }
        }
    } else /* sub, sbc, dec, neg */
    {
        if (CARRY_FLAG == 0) /* no carry */
        {
            if (HALF_CARRY_FLAG == 0) /* no half-carry */
            {
                add = 0x00;
                carry = 0;
            } else /* half-carry */
            {
                add = 0xFA;
                carry = 0;
            }
        } else /* carry */
        {
            if (HALF_CARRY_FLAG == 0) /* no half-carry */
            {
                add = 0xA0;
                carry = 1;
            } else /* half-carry */
            {
                add = 0x9A;
                carry = 1;
            }
        }
    }

    do_add_byte(add); /* adjust the value */

    if (parity(REG_A)) /* This seems odd -- is it a mistake? */
        SET_PARITY();
    else
        CLEAR_PARITY();

    if (subtract_flag) /* leave the subtract flag intact (right?) */
        SET_SUBTRACT();
    else
        CLEAR_SUBTRACT();

    if (carry)
        SET_CARRY();
    else
        CLEAR_CARRY();
}

static void do_rld(void)
{
    /*
     * Rotate-left-decimal.
     */
    int old_value, new_value;
    uint8_t clear, set;

    clear =
        SIGN_MASK | ZERO_MASK | HALF_CARRY_MASK | PARITY_MASK | SUBTRACT_MASK;
    set = 0;

    old_value = mem_read(REG_HL);

    /* left-shift old value, add lower bits of a */
    new_value = ((old_value << 4) | (REG_A & 0x0f)) & 0xff;

    /* rotate high bits of old value into low bits of a */
    REG_A = (REG_A & 0xf0) | (old_value >> 4);

    if (REG_A & 0x80)
        set |= SIGN_MASK;
    if (REG_A == 0)
        set |= ZERO_MASK;
    if (parity(REG_A))
        set |= PARITY_MASK;

    REG_F = (REG_F & ~clear) | set;
    mem_write(REG_HL, new_value);
}

static void do_rrd(void)
{
    /*
     * Rotate-right-decimal.
     */
    int old_value, new_value;
    uint8_t clear, set;

    clear =
        SIGN_MASK | ZERO_MASK | HALF_CARRY_MASK | PARITY_MASK | SUBTRACT_MASK;
    set = 0;

    old_value = mem_read(REG_HL);

    /* right-shift old value, add lower bits of a */
    new_value = (old_value >> 4) | ((REG_A & 0x0f) << 4);

    /* rotate low bits of old value into low bits of a */
    REG_A = (REG_A & 0xf0) | (old_value & 0x0f);

    if (REG_A & 0x80)
        set |= SIGN_MASK;
    if (REG_A == 0)
        set |= ZERO_MASK;
    if (parity(REG_A))
        set |= PARITY_MASK;

    REG_F = (REG_F & ~clear) | set;
    mem_write(REG_HL, new_value);
}

/*
 * Input/output instruction support:
 */

static void do_inid(int dir)
{
    mem_write(REG_HL, z80_in(REG_C));
    REG_HL += dir;
    REG_B--;

    if (REG_B == 0)
        SET_ZERO();
    else
        CLEAR_ZERO();

    SET_SUBTRACT();
}

static void do_inidr(int dir)
{
    do_inid(dir);

    if (REG_B != 0) {
        TSTATE += 5;
        REG_PC -= 2;
    }
}

static int in_with_flags(int port)
{
    /*
     * Do the appropriate flag calculations for the in instructions
     * which compute the flags.  Return the input value.
     */

    int value;
    uint8_t clear, set;

    clear =
        SIGN_MASK | ZERO_MASK | HALF_CARRY_MASK | PARITY_MASK | SUBTRACT_MASK;
    set = 0;

    value = z80_in(port);

    if (value & 0x80)
        set |= SIGN_MASK;
    if (value == 0)
        set |= ZERO_MASK;
    if (parity(value))
        set |= PARITY_MASK;

    /* What should the half-carry do?  Is this a mistake? */

    REG_F = (REG_F & ~clear) | set;

    return value;
}

static void do_outid(int dir)
{
    z80_out(REG_C, mem_read(REG_HL));
    REG_HL += dir;
    REG_B--;

    if (REG_B == 0)
        SET_ZERO();
    else
        CLEAR_ZERO();

    SET_SUBTRACT();
}

static void do_outidr(int dir)
{
    do_outid(dir);

    if (REG_B != 0) {
        TSTATE += 5;
        REG_PC -= 2;
    }
}

/*
 * Interrupt handling routines:
 */

static void do_di(void)
{
    z80_state.iff1 = z80_state.iff2 = false;
}

static void do_ei(void)
{
    z80_state.iff1 = z80_state.iff2 = true;
    z80_state.ei_shadow = true;
}

static void do_im0(void)
{
    z80_state.interrupt_mode = 0;
}

static void do_im1(void)
{
    z80_state.interrupt_mode = 1;
}

static void do_im2(void)
{
    z80_state.interrupt_mode = 2;
}

static void do_nmi(void)
{
    bool nminterrupt = xchg(&z80_state.nminterrupt, false);

    if (!nminterrupt)
        return;

    /* handle a non-maskable interrupt */
    if (tracing(TRACE_IO | TRACE_CPU)) {
        fprintf(tracef, "[%12" PRIu64 "] NMI: PC=%04x\n", TSTATE, REG_PC);
    }

    REG_SP -= 2;
    mem_write_word(REG_SP, REG_PC);
    z80_state.iff2 = z80_state.iff1;
    z80_state.iff1 = false;
    z80_state.nmi_in_progress = true;
    REG_PC = 0x66;
    inc_r();
    TSTATE += 11;
}

static void do_int(void)
{
    uint16_t old_pc = REG_PC;
    uint64_t when = TSTATE;
    int i_vector;

    i_vector = z80_intack();
    if (i_vector < 0)
        return;

    switch (z80_state.interrupt_mode) {
    case 0:
        /* We blithly assume we are fed an RST instruction */
        do_di();
        REG_SP -= 2;
        mem_write_word(REG_SP, REG_PC);
        REG_PC = i_vector & 0x38;
        TSTATE += 11;
        break;

    case 1:
        do_di();
        REG_SP -= 2;
        mem_write_word(REG_SP, REG_PC);
        REG_PC = 0x38;
        TSTATE += 11;
        break;

    case 2:
        do_di();
        REG_SP -= 2;
        mem_write_word(REG_SP, REG_PC);
        REG_PC = mem_read_word((z80_state.i << 8) | (i_vector & ~1));
        TSTATE += 19;
        break;

    default: /* oops, unkown interrupt mode... */
        break;
    }

    z80_state.iff1 = false;

    if (tracing(TRACE_CPU | TRACE_IO)) {
        fprintf(tracef,
                "[%12" PRIu64 "] INT: "
                "vector 0x%02x (%3d) I=%02x PC=%04x -> %04x\n",
                when, i_vector, i_vector, z80_state.i, old_pc, REG_PC);
    }

    inc_r();
}

static uint16_t get_hl_addr(wordregister* ix)
{
    if (ix == &z80_state.hl) {
        return ix->word;
    } else {
        TSTATE += 8; /* Ouch! */
        return ix->word + (int8_t)mem_fetch(REG_PC++);
    }
}

/*
 * Extended instructions which have 0xCB as the first byte:
 */

static void do_CB_instruction(wordregister* ix)
{
    uint8_t instruction;
    uint16_t addr;
    uint8_t data;

    if (ix == &z80_state.hl) {
        /*
         * Normal operation sans DD/FD prefix
         */

        instruction = mem_fetch(REG_PC++);
        inc_r();

        /* (HL) = 7 additional clocks, otherwise 4 */
        if ((instruction & 7) == 6) {
            TSTATE += (instruction & 0xc0) == 0x40 ? 8 : 11;
        } else {
            TSTATE += 4;
        }

        switch (instruction) {
        case 0x47: /* bit 0, a */
            do_test_bit(REG_A, 0);
            break;
        case 0x40: /* bit 0, b */
            do_test_bit(REG_B, 0);
            break;
        case 0x41: /* bit 0, c */
            do_test_bit(REG_C, 0);
            break;
        case 0x42: /* bit 0, d */
            do_test_bit(REG_D, 0);
            break;
        case 0x43: /* bit 0, e */
            do_test_bit(REG_E, 0);
            break;
        case 0x44: /* bit 0, h */
            do_test_bit(REG_H, 0);
            break;
        case 0x45: /* bit 0, l */
            do_test_bit(REG_L, 0);
            break;
        case 0x4F: /* bit 1, a */
            do_test_bit(REG_A, 1);
            break;
        case 0x48: /* bit 1, b */
            do_test_bit(REG_B, 1);
            break;
        case 0x49: /* bit 1, c */
            do_test_bit(REG_C, 1);
            break;
        case 0x4A: /* bit 1, d */
            do_test_bit(REG_D, 1);
            break;
        case 0x4B: /* bit 1, e */
            do_test_bit(REG_E, 1);
            break;
        case 0x4C: /* bit 1, h */
            do_test_bit(REG_H, 1);
            break;
        case 0x4D: /* bit 1, l */
            do_test_bit(REG_L, 1);
            break;
        case 0x57: /* bit 2, a */
            do_test_bit(REG_A, 2);
            break;
        case 0x50: /* bit 2, b */
            do_test_bit(REG_B, 2);
            break;
        case 0x51: /* bit 2, c */
            do_test_bit(REG_C, 2);
            break;
        case 0x52: /* bit 2, d */
            do_test_bit(REG_D, 2);
            break;
        case 0x53: /* bit 2, e */
            do_test_bit(REG_E, 2);
            break;
        case 0x54: /* bit 2, h */
            do_test_bit(REG_H, 2);
            break;
        case 0x55: /* bit 2, l */
            do_test_bit(REG_L, 2);
            break;
        case 0x5F: /* bit 3, a */
            do_test_bit(REG_A, 3);
            break;
        case 0x58: /* bit 3, b */
            do_test_bit(REG_B, 3);
            break;
        case 0x59: /* bit 3, c */
            do_test_bit(REG_C, 3);
            break;
        case 0x5A: /* bit 3, d */
            do_test_bit(REG_D, 3);
            break;
        case 0x5B: /* bit 3, e */
            do_test_bit(REG_E, 3);
            break;
        case 0x5C: /* bit 3, h */
            do_test_bit(REG_H, 3);
            break;
        case 0x5D: /* bit 3, l */
            do_test_bit(REG_L, 3);
            break;
        case 0x67: /* bit 4, a */
            do_test_bit(REG_A, 4);
            break;
        case 0x60: /* bit 4, b */
            do_test_bit(REG_B, 4);
            break;
        case 0x61: /* bit 4, c */
            do_test_bit(REG_C, 4);
            break;
        case 0x62: /* bit 4, d */
            do_test_bit(REG_D, 4);
            break;
        case 0x63: /* bit 4, e */
            do_test_bit(REG_E, 4);
            break;
        case 0x64: /* bit 4, h */
            do_test_bit(REG_H, 4);
            break;
        case 0x65: /* bit 4, l */
            do_test_bit(REG_L, 4);
            break;
        case 0x6F: /* bit 5, a */
            do_test_bit(REG_A, 5);
            break;
        case 0x68: /* bit 5, b */
            do_test_bit(REG_B, 5);
            break;
        case 0x69: /* bit 5, c */
            do_test_bit(REG_C, 5);
            break;
        case 0x6A: /* bit 5, d */
            do_test_bit(REG_D, 5);
            break;
        case 0x6B: /* bit 5, e */
            do_test_bit(REG_E, 5);
            break;
        case 0x6C: /* bit 5, h */
            do_test_bit(REG_H, 5);
            break;
        case 0x6D: /* bit 5, l */
            do_test_bit(REG_L, 5);
            break;
        case 0x77: /* bit 6, a */
            do_test_bit(REG_A, 6);
            break;
        case 0x70: /* bit 6, b */
            do_test_bit(REG_B, 6);
            break;
        case 0x71: /* bit 6, c */
            do_test_bit(REG_C, 6);
            break;
        case 0x72: /* bit 6, d */
            do_test_bit(REG_D, 6);
            break;
        case 0x73: /* bit 6, e */
            do_test_bit(REG_E, 6);
            break;
        case 0x74: /* bit 6, h */
            do_test_bit(REG_H, 6);
            break;
        case 0x75: /* bit 6, l */
            do_test_bit(REG_L, 6);
            break;
        case 0x7F: /* bit 7, a */
            do_test_bit(REG_A, 7);
            break;
        case 0x78: /* bit 7, b */
            do_test_bit(REG_B, 7);
            break;
        case 0x79: /* bit 7, c */
            do_test_bit(REG_C, 7);
            break;
        case 0x7A: /* bit 7, d */
            do_test_bit(REG_D, 7);
            break;
        case 0x7B: /* bit 7, e */
            do_test_bit(REG_E, 7);
            break;
        case 0x7C: /* bit 7, h */
            do_test_bit(REG_H, 7);
            break;
        case 0x7D: /* bit 7, l */
            do_test_bit(REG_L, 7);
            break;

        case 0x46: /* bit 0, (hl) */
            do_test_bit(mem_read(REG_HL), 0);
            break;
        case 0x4E: /* bit 1, (hl) */
            do_test_bit(mem_read(REG_HL), 1);
            break;
        case 0x56: /* bit 2, (hl) */
            do_test_bit(mem_read(REG_HL), 2);
            break;
        case 0x5E: /* bit 3, (hl) */
            do_test_bit(mem_read(REG_HL), 3);
            break;
        case 0x66: /* bit 4, (hl) */
            do_test_bit(mem_read(REG_HL), 4);
            break;
        case 0x6E: /* bit 5, (hl) */
            do_test_bit(mem_read(REG_HL), 5);
            break;
        case 0x76: /* bit 6, (hl) */
            do_test_bit(mem_read(REG_HL), 6);
            break;
        case 0x7E: /* bit 7, (hl) */
            do_test_bit(mem_read(REG_HL), 7);
            break;

        case 0x87: /* res 0, a */
            REG_A &= ~(1 << 0);
            break;
        case 0x80: /* res 0, b */
            REG_B &= ~(1 << 0);
            break;
        case 0x81: /* res 0, c */
            REG_C &= ~(1 << 0);
            break;
        case 0x82: /* res 0, d */
            REG_D &= ~(1 << 0);
            break;
        case 0x83: /* res 0, e */
            REG_E &= ~(1 << 0);
            break;
        case 0x84: /* res 0, h */
            REG_H &= ~(1 << 0);
            break;
        case 0x85: /* res 0, l */
            REG_L &= ~(1 << 0);
            break;
        case 0x8F: /* res 1, a */
            REG_A &= ~(1 << 1);
            break;
        case 0x88: /* res 1, b */
            REG_B &= ~(1 << 1);
            break;
        case 0x89: /* res 1, c */
            REG_C &= ~(1 << 1);
            break;
        case 0x8A: /* res 1, d */
            REG_D &= ~(1 << 1);
            break;
        case 0x8B: /* res 1, e */
            REG_E &= ~(1 << 1);
            break;
        case 0x8C: /* res 1, h */
            REG_H &= ~(1 << 1);
            break;
        case 0x8D: /* res 1, l */
            REG_L &= ~(1 << 1);
            break;
        case 0x97: /* res 2, a */
            REG_A &= ~(1 << 2);
            break;
        case 0x90: /* res 2, b */
            REG_B &= ~(1 << 2);
            break;
        case 0x91: /* res 2, c */
            REG_C &= ~(1 << 2);
            break;
        case 0x92: /* res 2, d */
            REG_D &= ~(1 << 2);
            break;
        case 0x93: /* res 2, e */
            REG_E &= ~(1 << 2);
            break;
        case 0x94: /* res 2, h */
            REG_H &= ~(1 << 2);
            break;
        case 0x95: /* res 2, l */
            REG_L &= ~(1 << 2);
            break;
        case 0x9F: /* res 3, a */
            REG_A &= ~(1 << 3);
            break;
        case 0x98: /* res 3, b */
            REG_B &= ~(1 << 3);
            break;
        case 0x99: /* res 3, c */
            REG_C &= ~(1 << 3);
            break;
        case 0x9A: /* res 3, d */
            REG_D &= ~(1 << 3);
            break;
        case 0x9B: /* res 3, e */
            REG_E &= ~(1 << 3);
            break;
        case 0x9C: /* res 3, h */
            REG_H &= ~(1 << 3);
            break;
        case 0x9D: /* res 3, l */
            REG_L &= ~(1 << 3);
            break;
        case 0xA7: /* res 4, a */
            REG_A &= ~(1 << 4);
            break;
        case 0xA0: /* res 4, b */
            REG_B &= ~(1 << 4);
            break;
        case 0xA1: /* res 4, c */
            REG_C &= ~(1 << 4);
            break;
        case 0xA2: /* res 4, d */
            REG_D &= ~(1 << 4);
            break;
        case 0xA3: /* res 4, e */
            REG_E &= ~(1 << 4);
            break;
        case 0xA4: /* res 4, h */
            REG_H &= ~(1 << 4);
            break;
        case 0xA5: /* res 4, l */
            REG_L &= ~(1 << 4);
            break;
        case 0xAF: /* res 5, a */
            REG_A &= ~(1 << 5);
            break;
        case 0xA8: /* res 5, b */
            REG_B &= ~(1 << 5);
            break;
        case 0xA9: /* res 5, c */
            REG_C &= ~(1 << 5);
            break;
        case 0xAA: /* res 5, d */
            REG_D &= ~(1 << 5);
            break;
        case 0xAB: /* res 5, e */
            REG_E &= ~(1 << 5);
            break;
        case 0xAC: /* res 5, h */
            REG_H &= ~(1 << 5);
            break;
        case 0xAD: /* res 5, l */
            REG_L &= ~(1 << 5);
            break;
        case 0xB7: /* res 6, a */
            REG_A &= ~(1 << 6);
            break;
        case 0xB0: /* res 6, b */
            REG_B &= ~(1 << 6);
            break;
        case 0xB1: /* res 6, c */
            REG_C &= ~(1 << 6);
            break;
        case 0xB2: /* res 6, d */
            REG_D &= ~(1 << 6);
            break;
        case 0xB3: /* res 6, e */
            REG_E &= ~(1 << 6);
            break;
        case 0xB4: /* res 6, h */
            REG_H &= ~(1 << 6);
            break;
        case 0xB5: /* res 6, l */
            REG_L &= ~(1 << 6);
            break;
        case 0xBF: /* res 7, a */
            REG_A &= ~(1 << 7);
            break;
        case 0xB8: /* res 7, b */
            REG_B &= ~(1 << 7);
            break;
        case 0xB9: /* res 7, c */
            REG_C &= ~(1 << 7);
            break;
        case 0xBA: /* res 7, d */
            REG_D &= ~(1 << 7);
            break;
        case 0xBB: /* res 7, e */
            REG_E &= ~(1 << 7);
            break;
        case 0xBC: /* res 7, h */
            REG_H &= ~(1 << 7);
            break;
        case 0xBD: /* res 7, l */
            REG_L &= ~(1 << 7);
            break;

        case 0x86: /* res 0, (hl) */
            mem_write(REG_HL, mem_read(REG_HL) & ~(1 << 0));
            break;
        case 0x8E: /* res 1, (hl) */
            mem_write(REG_HL, mem_read(REG_HL) & ~(1 << 1));
            break;
        case 0x96: /* res 2, (hl) */
            mem_write(REG_HL, mem_read(REG_HL) & ~(1 << 2));
            break;
        case 0x9E: /* res 3, (hl) */
            mem_write(REG_HL, mem_read(REG_HL) & ~(1 << 3));
            break;
        case 0xA6: /* res 4, (hl) */
            mem_write(REG_HL, mem_read(REG_HL) & ~(1 << 4));
            break;
        case 0xAE: /* res 5, (hl) */
            mem_write(REG_HL, mem_read(REG_HL) & ~(1 << 5));
            break;
        case 0xB6: /* res 6, (hl) */
            mem_write(REG_HL, mem_read(REG_HL) & ~(1 << 6));
            break;
        case 0xBE: /* res 7, (hl) */
            mem_write(REG_HL, mem_read(REG_HL) & ~(1 << 7));
            break;

        case 0x17: /* rl a */
            REG_A = rl_byte(REG_A);
            break;
        case 0x10: /* rl b */
            REG_B = rl_byte(REG_B);
            break;
        case 0x11: /* rl c */
            REG_C = rl_byte(REG_C);
            break;
        case 0x12: /* rl d */
            REG_D = rl_byte(REG_D);
            break;
        case 0x13: /* rl e */
            REG_E = rl_byte(REG_E);
            break;
        case 0x14: /* rl h */
            REG_H = rl_byte(REG_H);
            break;
        case 0x15: /* rl l */
            REG_L = rl_byte(REG_L);
            break;
        case 0x16: /* rl (hl) */
            mem_write(REG_HL, rl_byte(mem_read(REG_HL)));
            break;

        case 0x07: /* rlc a */
            REG_A = rlc_byte(REG_A);
            break;
        case 0x00: /* rlc b */
            REG_B = rlc_byte(REG_B);
            break;
        case 0x01: /* rlc c */
            REG_C = rlc_byte(REG_C);
            break;
        case 0x02: /* rlc d */
            REG_D = rlc_byte(REG_D);
            break;
        case 0x03: /* rlc e */
            REG_E = rlc_byte(REG_E);
            break;
        case 0x04: /* rlc h */
            REG_H = rlc_byte(REG_H);
            break;
        case 0x05: /* rlc l */
            REG_L = rlc_byte(REG_L);
            break;
        case 0x06: /* rlc (hl) */
            mem_write(REG_HL, rlc_byte(mem_read(REG_HL)));
            break;

        case 0x1F: /* rr a */
            REG_A = rr_byte(REG_A);
            break;
        case 0x18: /* rr b */
            REG_B = rr_byte(REG_B);
            break;
        case 0x19: /* rr c */
            REG_C = rr_byte(REG_C);
            break;
        case 0x1A: /* rr d */
            REG_D = rr_byte(REG_D);
            break;
        case 0x1B: /* rr e */
            REG_E = rr_byte(REG_E);
            break;
        case 0x1C: /* rr h */
            REG_H = rr_byte(REG_H);
            break;
        case 0x1D: /* rr l */
            REG_L = rr_byte(REG_L);
            break;
        case 0x1E: /* rr (hl) */
            mem_write(REG_HL, rr_byte(mem_read(REG_HL)));
            break;

        case 0x0F: /* rrc a */
            REG_A = rrc_byte(REG_A);
            break;
        case 0x08: /* rrc b */
            REG_B = rrc_byte(REG_B);
            break;
        case 0x09: /* rrc c */
            REG_C = rrc_byte(REG_C);
            break;
        case 0x0A: /* rrc d */
            REG_D = rrc_byte(REG_D);
            break;
        case 0x0B: /* rrc e */
            REG_E = rrc_byte(REG_E);
            break;
        case 0x0C: /* rrc h */
            REG_H = rrc_byte(REG_H);
            break;
        case 0x0D: /* rrc l */
            REG_L = rrc_byte(REG_L);
            break;
        case 0x0E: /* rrc (hl) */
            mem_write(REG_HL, rrc_byte(mem_read(REG_HL)));
            break;

        case 0xC7: /* set 0, a */
            REG_A |= (1 << 0);
            break;
        case 0xC0: /* set 0, b */
            REG_B |= (1 << 0);
            break;
        case 0xC1: /* set 0, c */
            REG_C |= (1 << 0);
            break;
        case 0xC2: /* set 0, d */
            REG_D |= (1 << 0);
            break;
        case 0xC3: /* set 0, e */
            REG_E |= (1 << 0);
            break;
        case 0xC4: /* set 0, h */
            REG_H |= (1 << 0);
            break;
        case 0xC5: /* set 0, l */
            REG_L |= (1 << 0);
            break;
        case 0xCF: /* set 1, a */
            REG_A |= (1 << 1);
            break;
        case 0xC8: /* set 1, b */
            REG_B |= (1 << 1);
            break;
        case 0xC9: /* set 1, c */
            REG_C |= (1 << 1);
            break;
        case 0xCA: /* set 1, d */
            REG_D |= (1 << 1);
            break;
        case 0xCB: /* set 1, e */
            REG_E |= (1 << 1);
            break;
        case 0xCC: /* set 1, h */
            REG_H |= (1 << 1);
            break;
        case 0xCD: /* set 1, l */
            REG_L |= (1 << 1);
            break;
        case 0xD7: /* set 2, a */
            REG_A |= (1 << 2);
            break;
        case 0xD0: /* set 2, b */
            REG_B |= (1 << 2);
            break;
        case 0xD1: /* set 2, c */
            REG_C |= (1 << 2);
            break;
        case 0xD2: /* set 2, d */
            REG_D |= (1 << 2);
            break;
        case 0xD3: /* set 2, e */
            REG_E |= (1 << 2);
            break;
        case 0xD4: /* set 2, h */
            REG_H |= (1 << 2);
            break;
        case 0xD5: /* set 2, l */
            REG_L |= (1 << 2);
            break;
        case 0xDF: /* set 3, a */
            REG_A |= (1 << 3);
            break;
        case 0xD8: /* set 3, b */
            REG_B |= (1 << 3);
            break;
        case 0xD9: /* set 3, c */
            REG_C |= (1 << 3);
            break;
        case 0xDA: /* set 3, d */
            REG_D |= (1 << 3);
            break;
        case 0xDB: /* set 3, e */
            REG_E |= (1 << 3);
            break;
        case 0xDC: /* set 3, h */
            REG_H |= (1 << 3);
            break;
        case 0xDD: /* set 3, l */
            REG_L |= (1 << 3);
            break;
        case 0xE7: /* set 4, a */
            REG_A |= (1 << 4);
            break;
        case 0xE0: /* set 4, b */
            REG_B |= (1 << 4);
            break;
        case 0xE1: /* set 4, c */
            REG_C |= (1 << 4);
            break;
        case 0xE2: /* set 4, d */
            REG_D |= (1 << 4);
            break;
        case 0xE3: /* set 4, e */
            REG_E |= (1 << 4);
            break;
        case 0xE4: /* set 4, h */
            REG_H |= (1 << 4);
            break;
        case 0xE5: /* set 4, l */
            REG_L |= (1 << 4);
            break;
        case 0xEF: /* set 5, a */
            REG_A |= (1 << 5);
            break;
        case 0xE8: /* set 5, b */
            REG_B |= (1 << 5);
            break;
        case 0xE9: /* set 5, c */
            REG_C |= (1 << 5);
            break;
        case 0xEA: /* set 5, d */
            REG_D |= (1 << 5);
            break;
        case 0xEB: /* set 5, e */
            REG_E |= (1 << 5);
            break;
        case 0xEC: /* set 5, h */
            REG_H |= (1 << 5);
            break;
        case 0xED: /* set 5, l */
            REG_L |= (1 << 5);
            break;
        case 0xF7: /* set 6, a */
            REG_A |= (1 << 6);
            break;
        case 0xF0: /* set 6, b */
            REG_B |= (1 << 6);
            break;
        case 0xF1: /* set 6, c */
            REG_C |= (1 << 6);
            break;
        case 0xF2: /* set 6, d */
            REG_D |= (1 << 6);
            break;
        case 0xF3: /* set 6, e */
            REG_E |= (1 << 6);
            break;
        case 0xF4: /* set 6, h */
            REG_H |= (1 << 6);
            break;
        case 0xF5: /* set 6, l */
            REG_L |= (1 << 6);
            break;
        case 0xFF: /* set 7, a */
            REG_A |= (1 << 7);
            break;
        case 0xF8: /* set 7, b */
            REG_B |= (1 << 7);
            break;
        case 0xF9: /* set 7, c */
            REG_C |= (1 << 7);
            break;
        case 0xFA: /* set 7, d */
            REG_D |= (1 << 7);
            break;
        case 0xFB: /* set 7, e */
            REG_E |= (1 << 7);
            break;
        case 0xFC: /* set 7, h */
            REG_H |= (1 << 7);
            break;
        case 0xFD: /* set 7, l */
            REG_L |= (1 << 7);
            break;

        case 0xC6: /* set 0, (hl) */
            mem_write(REG_HL, mem_read(REG_HL) | (1 << 0));
            break;
        case 0xCE: /* set 1, (hl) */
            mem_write(REG_HL, mem_read(REG_HL) | (1 << 1));
            break;
        case 0xD6: /* set 2, (hl) */
            mem_write(REG_HL, mem_read(REG_HL) | (1 << 2));
            break;
        case 0xDE: /* set 3, (hl) */
            mem_write(REG_HL, mem_read(REG_HL) | (1 << 3));
            break;
        case 0xE6: /* set 4, (hl) */
            mem_write(REG_HL, mem_read(REG_HL) | (1 << 4));
            break;
        case 0xEE: /* set 5, (hl) */
            mem_write(REG_HL, mem_read(REG_HL) | (1 << 5));
            break;
        case 0xF6: /* set 6, (hl) */
            mem_write(REG_HL, mem_read(REG_HL) | (1 << 6));
            break;
        case 0xFE: /* set 7, (hl) */
            mem_write(REG_HL, mem_read(REG_HL) | (1 << 7));
            break;

        case 0x27: /* sla a */
            REG_A = sla_byte(REG_A);
            break;
        case 0x20: /* sla b */
            REG_B = sla_byte(REG_B);
            break;
        case 0x21: /* sla c */
            REG_C = sla_byte(REG_C);
            break;
        case 0x22: /* sla d */
            REG_D = sla_byte(REG_D);
            break;
        case 0x23: /* sla e */
            REG_E = sla_byte(REG_E);
            break;
        case 0x24: /* sla h */
            REG_H = sla_byte(REG_H);
            break;
        case 0x25: /* sla l */
            REG_L = sla_byte(REG_L);
            break;
        case 0x26: /* sla (hl) */
            mem_write(REG_HL, sla_byte(mem_read(REG_HL)));
            break;

        case 0x37: /* sll a */
            REG_A = sll_byte(REG_A);
            break;
        case 0x30: /* sll b */
            REG_B = sll_byte(REG_B);
            break;
        case 0x31: /* sll c */
            REG_C = sll_byte(REG_C);
            break;
        case 0x32: /* sll d */
            REG_D = sll_byte(REG_D);
            break;
        case 0x33: /* sll e */
            REG_E = sll_byte(REG_E);
            break;
        case 0x34: /* sll h */
            REG_H = sll_byte(REG_H);
            break;
        case 0x35: /* sll l */
            REG_L = sll_byte(REG_L);
            break;
        case 0x36: /* sll (hl) */
            mem_write(REG_HL, sll_byte(mem_read(REG_HL)));
            break;

        case 0x2F: /* sra a */
            REG_A = sra_byte(REG_A);
            break;
        case 0x28: /* sra b */
            REG_B = sra_byte(REG_B);
            break;
        case 0x29: /* sra c */
            REG_C = sra_byte(REG_C);
            break;
        case 0x2A: /* sra d */
            REG_D = sra_byte(REG_D);
            break;
        case 0x2B: /* sra e */
            REG_E = sra_byte(REG_E);
            break;
        case 0x2C: /* sra h */
            REG_H = sra_byte(REG_H);
            break;
        case 0x2D: /* sra l */
            REG_L = sra_byte(REG_L);
            break;
        case 0x2E: /* sra (hl) */
            mem_write(REG_HL, sra_byte(mem_read(REG_HL)));
            break;

        case 0x3F: /* srl a */
            REG_A = srl_byte(REG_A);
            break;
        case 0x38: /* srl b */
            REG_B = srl_byte(REG_B);
            break;
        case 0x39: /* srl c */
            REG_C = srl_byte(REG_C);
            break;
        case 0x3A: /* srl d */
            REG_D = srl_byte(REG_D);
            break;
        case 0x3B: /* srl e */
            REG_E = srl_byte(REG_E);
            break;
        case 0x3C: /* srl h */
            REG_H = srl_byte(REG_H);
            break;
        case 0x3D: /* srl l */
            REG_L = srl_byte(REG_L);
            break;
        case 0x3E: /* srl (hl) */
            mem_write(REG_HL, srl_byte(mem_read(REG_HL)));
            break;
        }
    } else {
        /*
         * Indexed instructions are weird.  They ALWAYS take the source from
         * (Ix+d) and ALWAYS write the result back, but ALSO write the result
         * to a GPR unless the register specifier is 6.  BIT never writes
         * anything back to either memory or GPR.
         */

        addr = ix->word + (int8_t)mem_fetch(REG_PC++);
        instruction = mem_fetch(REG_PC++);
        /* No R increment here, for some reason */

        TSTATE += ((instruction & 0xc0) == 0x40) ? 12 : 15;

        data = mem_read(addr);

        switch (instruction & 0xc0) {
        case 0x00:
            switch (instruction & ~7) {
            case 0x00: /* RLC */
                data = rlc_byte(data);
                break;
            case 0x08: /* RRC */
                data = rrc_byte(data);
                break;
            case 0x10: /* RL */
                data = rl_byte(data);
                break;
            case 0x18: /* RR */
                data = rr_byte(data);
                break;
            case 0x20: /* SLA */
                data = sla_byte(data);
                break;
            case 0x28: /* SRA */
                data = sra_byte(data);
                break;
            case 0x30: /* SLL */
                data = sll_byte(data);
                break;
            case 0x38: /* SRL */
                data = srl_byte(data);
                break;
            }
            break;

        case 0x40: /* BIT */
            do_test_bit(data, (instruction >> 3) & 7);
            return; /* No writeback! */

        case 0x80: /* RES */
            data &= ~(1 << ((instruction >> 3) & 7));
            break;

        case 0xc0: /* SET */
            data |= (1 << ((instruction >> 3) & 7));
            break;
        }

        switch (instruction & 7) {
        case 0:
            REG_B = data;
            break;
        case 1:
            REG_C = data;
            break;
        case 2:
            REG_D = data;
            break;
        case 3:
            REG_E = data;
            break;
        case 4:
            REG_H = data;
            break;
        case 5:
            REG_L = data;
            break;
        case 6:
            /* Only memory */
            break;
        case 7:
            REG_A = data;
            break;
        }

        mem_write(addr, data);
    }
}

static void do_ED_instruction(wordregister* ix)
{
    uint8_t instruction;

    (void)ix; /* DD/FD has no effect */

    /*
     * Undocumented instruction notes:
     * ED 00-3F = NOP
     * ED 80-BF = NOP unless documented
     * ED C0-FF = NOP
     * ED 40-7F duplicate:
     *   NEG       at ED4C, ED54, ED5C, ED64, ED6C, ED74, ED7C
     *   NOP       at ED77, ED7F
     *   RETN      at ED55, ED65, ED75
     *   RETI      at ED5D, ED6D, ED7D
     *   IM ?      at ED4E, ED6E
     *   IM 0      at ED66
     *   IM 1      at ED76
     *   IM 2      at ED7E
     *   IN F,(C)  at ED70
     *   OUT (C),0 at ED71  -- OUT (C),0FFh for CMOS Z80
     */

    instruction = mem_fetch(REG_PC++);
    inc_r();
    TSTATE += clk_ED[instruction];

    switch (instruction) {
    case 0x4A: /* adc hl, bc */
        do_adc_word(REG_BC);
        break;
    case 0x5A: /* adc hl, de */
        do_adc_word(REG_DE);
        break;
    case 0x6A: /* adc hl, hl */
        do_adc_word(REG_HL);
        break;
    case 0x7A: /* adc hl, sp */
        do_adc_word(REG_SP);
        break;

    case 0xA9: /* cpd */
        do_cpid(-1);
        break;
    case 0xB9: /* cpdr */
        do_cpidr(-1);
        break;

    case 0xA1: /* cpi */
        do_cpid(+1);
        break;
    case 0xB1: /* cpir */
        do_cpidr(+1);
        break;

    case 0x46: /* im 0 */
    case 0x66:
    case 0x4E:
    case 0x6E:
        do_im0();
        break;
    case 0x56: /* im 1 */
    case 0x76:
        do_im1();
        break;
    case 0x5E: /* im 2 */
    case 0x7E:
        do_im2();
        break;

    case 0x78: /* in a, (c) */
        REG_A = in_with_flags(REG_C);
        break;
    case 0x40: /* in b, (c) */
        REG_B = in_with_flags(REG_C);
        break;
    case 0x48: /* in c, (c) */
        REG_C = in_with_flags(REG_C);
        break;
    case 0x50: /* in d, (c) */
        REG_D = in_with_flags(REG_C);
        break;
    case 0x58: /* in e, (c) */
        REG_E = in_with_flags(REG_C);
        break;
    case 0x60: /* in h, (c) */
        REG_H = in_with_flags(REG_C);
        break;
    case 0x68: /* in l, (c) */
        REG_L = in_with_flags(REG_C);
        break;

    case 0xAA: /* ind */
        do_inid(-1);
        break;
    case 0xBA: /* indr */
        do_inidr(-1);
        break;
    case 0xA2: /* ini */
        do_inid(+1);
        break;
    case 0xB2: /* inir */
        do_inidr(+1);
        break;

    case 0x57: /* ld a, i */
        do_ld_a_ir(REG_I);
        break;
    case 0x47: /* ld i, a */
        REG_I = REG_A;
        break;

    case 0x5F: /* ld a, r */
        do_ld_a_ir(REG_R);
        break;
    case 0x4F: /* ld r, a */
        z80_state.rf = z80_state.rc = REG_A;
        break;

    case 0x4B: /* ld bc, (address) */
        REG_BC = mem_read_word(mem_fetch_word(REG_PC));
        REG_PC += 2;
        break;
    case 0x5B: /* ld de, (address) */
        REG_DE = mem_read_word(mem_fetch_word(REG_PC));
        REG_PC += 2;
        break;
    case 0x6B: /* ld hl, (address) */
        /* this instruction is redundant with the 2A instruction */
        REG_HL = mem_read_word(mem_fetch_word(REG_PC));
        REG_PC += 2;
        break;
    case 0x7B: /* ld sp, (address) */
        REG_SP = mem_read_word(mem_fetch_word(REG_PC));
        REG_PC += 2;
        break;

    case 0x43: /* ld (address), bc */
        mem_write_word(mem_fetch_word(REG_PC), REG_BC);
        REG_PC += 2;
        break;
    case 0x53: /* ld (address), de */
        mem_write_word(mem_fetch_word(REG_PC), REG_DE);
        REG_PC += 2;
        break;
    case 0x63: /* ld (address), hl */
        mem_write_word(mem_fetch_word(REG_PC), REG_HL);
        REG_PC += 2;
        break;
    case 0x73: /* ld (address), sp */
        mem_write_word(mem_fetch_word(REG_PC), REG_SP);
        REG_PC += 2;
        break;

    case 0xA8: /* ldd */
        do_ldid(-1);
        break;
    case 0xB8: /* lddr */
        do_ldidr(-1);
        break;
    case 0xA0: /* ldi */
        do_ldid(+1);
        break;
    case 0xB0: /* ldir */
        do_ldidr(+1);
        break;

    case 0x44: /* neg */
    case 0x4C:
    case 0x54:
    case 0x5C:
    case 0x64:
    case 0x6C:
    case 0x74:
    case 0x7C:
        do_negate();
        break;

    case 0x79: /* out (c), a */
        z80_out(REG_C, REG_A);
        break;
    case 0x41: /* out (c), b */
        z80_out(REG_C, REG_B);
        break;
    case 0x49: /* out (c), c */
        z80_out(REG_C, REG_C);
        break;
    case 0x51: /* out (c), d */
        z80_out(REG_C, REG_D);
        break;
    case 0x59: /* out (c), e */
        z80_out(REG_C, REG_E);
        break;
    case 0x61: /* out (c), h */
        z80_out(REG_C, REG_H);
        break;
    case 0x69: /* out (c), l */
        z80_out(REG_C, REG_L);
        break;
    case 0x71: /* out (c), 0 */
        z80_out(REG_C, 0);
        break;

    case 0xAB: /* outd */
        do_outid(-1);
        break;
    case 0xBB: /* outdr */
        do_outidr(-1);
        break;
    case 0xA3: /* outi */
        do_outid(+1);
        break;
    case 0xB3: /* outir */
        do_outidr(+1);
        break;

    case 0x4D: /* reti */
    case 0x5D:
    case 0x6D:
    case 0x7D: {
        REG_PC = mem_read_word(REG_SP);
        REG_SP += 2;
        z80_state.iff1 = z80_state.iff2;
        z80_state.signal_eoi = true; /* Send EOI before next instruction */
    } break;

    case 0x45: /* retn */
    case 0x55:
    case 0x65:
    case 0x75:
        REG_PC = mem_read_word(REG_SP);
        REG_SP += 2;
        z80_state.iff1 = z80_state.iff2;
        z80_state.nmi_in_progress = false;
        break;

    case 0x6F: /* rld */
        do_rld();
        break;

    case 0x67: /* rrd */
        do_rrd();
        break;

    case 0x42: /* sbc hl, bc */
        do_sbc_word(REG_BC);
        break;
    case 0x52: /* sbc hl, de */
        do_sbc_word(REG_DE);
        break;
    case 0x62: /* sbc hl, hl */
        do_sbc_word(REG_HL);
        break;
    case 0x72: /* sbc hl, sp */
        do_sbc_word(REG_SP);
        break;

    default:
        /* Assume all others are NOP */
        break;
    }
}

static inline void check_eoi(void)
{
    if (!likely(z80_state.signal_eoi))
        return;

    if (tracing(TRACE_IO)) {
        fprintf(tracef, "[%12" PRIu64 "] EOI: RETI executed\n", TSTATE);
    }

    z80_state.signal_eoi = false;
    z80_eoi();
}

int z80_run(bool continuous, bool halted)
{
    uint8_t instruction;
    uint16_t address; /* generic temps */
    wordregister* ix;

    /* loop to do a z80 instruction */
    do {
        if (tracing(TRACE_CPU)) {
            diffstate();
            tracemem();
            fputc('\n', tracef);
        }
        check_eoi();
        for (;;) {
            /* Poll for external event */
            if (z80_poll_external())
                return halted;

            /* Check for an interrupt */
            if (z80_state.nminterrupt && !z80_state.nmi_in_progress) {
                halted = false;
                do_nmi();
            } else if (z80_state.iff1 && !z80_state.ei_shadow && poll_irq()) {
                halted = false;
                do_int();
            }
            z80_state.ei_shadow = false;
            if (!halted)
                break;
            TSTATE += 4;

            if (!continuous)
                return halted;
        }

        if (tracing(TRACE_CPU)) {
            fprintf(tracef, "[%12" PRIu64 "] PC=%04X ", TSTATE, REG_PC);
            disassemble(z80_state.pc.word);
        }

        ix = &z80_state.hl; /* Not an index instruction */

        instruction = mem_fetch_m1(REG_PC++);

    indexed:
        TSTATE += clk_main[instruction];
        inc_r();

        switch (instruction) {
        case 0xCB: /* CB.. extended instruction */
            do_CB_instruction(ix);
            break;
        case 0xDD: /* DD.. extended instruction */
            ix = &z80_state.ix;
            instruction = mem_fetch(REG_PC++);
            goto indexed;
        case 0xED: /* ED.. extended instruction */
            do_ED_instruction(ix);
            break;
        case 0xFD: /* FD.. extended instruction */
            ix = &z80_state.iy;
            instruction = mem_fetch(REG_PC++);
            goto indexed;

        case 0x8F: /* adc a, a */
            do_adc_byte(REG_A);
            break;
        case 0x88: /* adc a, b */
            do_adc_byte(REG_B);
            break;
        case 0x89: /* adc a, c */
            do_adc_byte(REG_C);
            break;
        case 0x8A: /* adc a, d */
            do_adc_byte(REG_D);
            break;
        case 0x8B: /* adc a, e */
            do_adc_byte(REG_E);
            break;
        case 0x8C: /* adc a, h */
            do_adc_byte(ix->byte.high);
            break;
        case 0x8D: /* adc a, l */
            do_adc_byte(ix->byte.low);
            break;
        case 0xCE: /* adc a, value */
            do_adc_byte(mem_fetch(REG_PC++));
            break;
        case 0x8E: /* adc a, (hl) */
            do_adc_byte(mem_read(get_hl_addr(ix)));
            break;

        case 0x87: /* add a, a */
            do_add_byte(REG_A);
            break;
        case 0x80: /* add a, b */
            do_add_byte(REG_B);
            break;
        case 0x81: /* add a, c */
            do_add_byte(REG_C);
            break;
        case 0x82: /* add a, d */
            do_add_byte(REG_D);
            break;
        case 0x83: /* add a, e */
            do_add_byte(REG_E);
            break;
        case 0x84: /* add a, h */
            do_add_byte(ix->byte.high);
            break;
        case 0x85: /* add a, l */
            do_add_byte(ix->byte.low);
            break;
        case 0xC6: /* add a, value */
            do_add_byte(mem_fetch(REG_PC++));
            break;
        case 0x86: /* add a, (hl) */
            do_add_byte(mem_read(get_hl_addr(ix)));
            break;

        case 0x09: /* add hl, bc */
            do_add_word(ix, REG_BC);
            break;
        case 0x19: /* add hl, de */
            do_add_word(ix, REG_DE);
            break;
        case 0x29: /* add hl, hl */
            do_add_word(ix, ix->word);
            break;
        case 0x39: /* add hl, sp */
            do_add_word(ix, REG_SP);
            break;

        case 0xA7: /* and a */
            do_and_byte(REG_A);
            break;
        case 0xA0: /* and b */
            do_and_byte(REG_B);
            break;
        case 0xA1: /* and c */
            do_and_byte(REG_C);
            break;
        case 0xA2: /* and d */
            do_and_byte(REG_D);
            break;
        case 0xA3: /* and e */
            do_and_byte(REG_E);
            break;
        case 0xA4: /* and h */
            do_and_byte(ix->byte.high);
            break;
        case 0xA5: /* and l */
            do_and_byte(ix->byte.low);
            break;
        case 0xE6: /* and value */
            do_and_byte(mem_fetch(REG_PC++));
            break;
        case 0xA6: /* and (hl) */
            do_and_byte(mem_read(get_hl_addr(ix)));
            break;

        case 0xCD: /* call address */
            address = mem_fetch_word(REG_PC);
            REG_SP -= 2;
            mem_write_word(REG_SP, REG_PC + 2);
            REG_PC = address;
            break;

        case 0xC4: /* call nz, address */
            if (!ZERO_FLAG) {
                address = mem_fetch_word(REG_PC);
                REG_SP -= 2;
                mem_write_word(REG_SP, REG_PC + 2);
                REG_PC = address;
                TSTATE += 7;
                break;
            } else {
                REG_PC += 2;
            }
            break;
        case 0xCC: /* call z, address */
            if (ZERO_FLAG) {
                address = mem_fetch_word(REG_PC);
                REG_SP -= 2;
                mem_write_word(REG_SP, REG_PC + 2);
                REG_PC = address;
                TSTATE += 7;
                break;
            } else {
                REG_PC += 2;
            }
            break;
        case 0xD4: /* call nc, address */
            if (!CARRY_FLAG) {
                address = mem_fetch_word(REG_PC);
                REG_SP -= 2;
                mem_write_word(REG_SP, REG_PC + 2);
                REG_PC = address;
                TSTATE += 7;
                break;
            } else {
                REG_PC += 2;
            }
            break;
        case 0xDC: /* call c, address */
            if (CARRY_FLAG) {
                address = mem_fetch_word(REG_PC);
                REG_SP -= 2;
                mem_write_word(REG_SP, REG_PC + 2);
                REG_PC = address;
                TSTATE += 7;
                break;
            } else {
                REG_PC += 2;
            }
            break;
        case 0xE4: /* call po, address */
            if (!PARITY_FLAG) {
                address = mem_fetch_word(REG_PC);
                REG_SP -= 2;
                mem_write_word(REG_SP, REG_PC + 2);
                REG_PC = address;
                TSTATE += 7;
                break;
            } else {
                REG_PC += 2;
            }
            break;
        case 0xEC: /* call pe, address */
            if (PARITY_FLAG) {
                address = mem_fetch_word(REG_PC);
                REG_SP -= 2;
                mem_write_word(REG_SP, REG_PC + 2);
                REG_PC = address;
                TSTATE += 7;
                break;
            } else {
                REG_PC += 2;
            }
            break;
        case 0xF4: /* call p, address */
            if (!SIGN_FLAG) {
                address = mem_fetch_word(REG_PC);
                REG_SP -= 2;
                mem_write_word(REG_SP, REG_PC + 2);
                REG_PC = address;
                TSTATE += 7;
                break;
            } else {
                REG_PC += 2;
            }
            break;
        case 0xFC: /* call m, address */
            if (SIGN_FLAG) {
                address = mem_fetch_word(REG_PC);
                REG_SP -= 2;
                mem_write_word(REG_SP, REG_PC + 2);
                REG_PC = address;
                TSTATE += 7;
                break;
            } else {
                REG_PC += 2;
            }
            break;

        case 0x3F: /* ccf */
            REG_F = (REG_F ^ CARRY_MASK) & ~SUBTRACT_MASK;
            break;

        case 0xBF: /* cp a */
            do_cp(REG_A);
            break;
        case 0xB8: /* cp b */
            do_cp(REG_B);
            break;
        case 0xB9: /* cp c */
            do_cp(REG_C);
            break;
        case 0xBA: /* cp d */
            do_cp(REG_D);
            break;
        case 0xBB: /* cp e */
            do_cp(REG_E);
            break;
        case 0xBC: /* cp h */
            do_cp(ix->byte.high);
            break;
        case 0xBD: /* cp l */
            do_cp(ix->byte.low);
            break;
        case 0xFE: /* cp value */
            do_cp(mem_fetch(REG_PC++));
            break;
        case 0xBE: /* cp (hl) */
            do_cp(mem_read(get_hl_addr(ix)));
            break;

        case 0x2F: /* cpl */
            REG_A = ~REG_A;
            REG_F |= (HALF_CARRY_MASK | SUBTRACT_MASK);
            break;

        case 0x27: /* daa */
            do_daa();
            break;

        case 0x3D: /* dec a */
            do_flags_dec_byte(--REG_A);
            break;
        case 0x05: /* dec b */
            do_flags_dec_byte(--REG_B);
            break;
        case 0x0D: /* dec c */
            do_flags_dec_byte(--REG_C);
            break;
        case 0x15: /* dec d */
            do_flags_dec_byte(--REG_D);
            break;
        case 0x1D: /* dec e */
            do_flags_dec_byte(--REG_E);
            break;
        case 0x25: /* dec h */
            do_flags_dec_byte(--ix->byte.high);
            break;
        case 0x2D: /* dec l */
            do_flags_dec_byte(--ix->byte.low);
            break;

        case 0x35: /* dec (hl) */
        {
            uint16_t addr = get_hl_addr(ix);
            uint8_t value = mem_read(addr) - 1;
            mem_write(addr, value);
            do_flags_dec_byte(value);
        } break;

        case 0x0B: /* dec bc */
            REG_BC--;
            break;
        case 0x1B: /* dec de */
            REG_DE--;
            break;
        case 0x2B: /* dec hl */
            ix->word--;
            break;
        case 0x3B: /* dec sp */
            REG_SP--;
            break;

        case 0xF3: /* di */
            do_di();
            break;

        case 0x10: /* djnz offset */
            /* Zaks says no flag changes. */
            if (--REG_B != 0) {
                REG_PC += ((int8_t)mem_fetch(REG_PC));
                TSTATE += 5;
            }
            REG_PC++;
            break;

        case 0xFB: /* ei */
            do_ei();
            break;

        case 0x08: /* ex af, af' */
        {
            uint16_t temp;
            temp = REG_AF;
            REG_AF = REG_AF_PRIME;
            REG_AF_PRIME = temp;
        } break;

        case 0xEB: /* ex de, hl */
        {
            uint16_t temp;
            temp = REG_DE;
            REG_DE = ix->word;
            ix->word = temp;
        } break;

        case 0xE3: /* ex (sp), hl */
        {
            uint16_t temp;
            temp = mem_read_word(REG_SP);
            mem_write_word(REG_SP, ix->word);
            ix->word = temp;
        } break;

        case 0xD9: /* exx */
        {
            uint16_t tmp;
            tmp = REG_BC_PRIME;
            REG_BC_PRIME = REG_BC;
            REG_BC = tmp;
            tmp = REG_DE_PRIME;
            REG_DE_PRIME = REG_DE;
            REG_DE = tmp;
            tmp = REG_HL_PRIME;
            REG_HL_PRIME = REG_HL;
            REG_HL = tmp;
        } break;

        case 0x76: /* halt */
            halted = 1;
            break;

        case 0xDB: /* in a, (port) */
            REG_A = z80_in(mem_fetch(REG_PC++));
            break;

        case 0x3C: /* inc a */
            REG_A++;
            do_flags_inc_byte(REG_A);
            break;
        case 0x04: /* inc b */
            REG_B++;
            do_flags_inc_byte(REG_B);
            break;
        case 0x0C: /* inc c */
            REG_C++;
            do_flags_inc_byte(REG_C);
            break;
        case 0x14: /* inc d */
            REG_D++;
            do_flags_inc_byte(REG_D);
            break;
        case 0x1C: /* inc e */
            REG_E++;
            do_flags_inc_byte(REG_E);
            break;
        case 0x24: /* inc h */
            ix->byte.high++;
            do_flags_inc_byte(ix->byte.high);
            break;
        case 0x2C: /* inc l */
            ix->byte.low++;
            do_flags_inc_byte(ix->byte.low);
            break;

        case 0x34: /* inc (hl) */
        {
            uint16_t addr = get_hl_addr(ix);
            uint8_t value = mem_read(addr) + 1;
            mem_write(addr, value);
            do_flags_inc_byte(value);
        } break;

        case 0x03: /* inc bc */
            REG_BC++;
            break;
        case 0x13: /* inc de */
            REG_DE++;
            break;
        case 0x23: /* inc hl */
            ix->word++;
            break;
        case 0x33: /* inc sp */
            REG_SP++;
            break;

        case 0xC3: /* jp address */
            REG_PC = mem_fetch_word(REG_PC);
            break;

        case 0xE9: /* jp (hl) */
            REG_PC = ix->word;
            break;

        case 0xC2: /* jp nz, address */
            if (!ZERO_FLAG) {
                REG_PC = mem_fetch_word(REG_PC);
            } else {
                REG_PC += 2;
            }
            break;
        case 0xCA: /* jp z, address */
            if (ZERO_FLAG) {
                REG_PC = mem_fetch_word(REG_PC);
            } else {
                REG_PC += 2;
            }
            break;
        case 0xD2: /* jp nc, address */
            if (!CARRY_FLAG) {
                REG_PC = mem_fetch_word(REG_PC);
            } else {
                REG_PC += 2;
            }
            break;
        case 0xDA: /* jp c, address */
            if (CARRY_FLAG) {
                REG_PC = mem_fetch_word(REG_PC);
            } else {
                REG_PC += 2;
            }
            break;
        case 0xE2: /* jp po, address */
            if (!PARITY_FLAG) {
                REG_PC = mem_fetch_word(REG_PC);
            } else {
                REG_PC += 2;
            }
            break;
        case 0xEA: /* jp pe, address */
            if (PARITY_FLAG) {
                REG_PC = mem_fetch_word(REG_PC);
            } else {
                REG_PC += 2;
            }
            break;
        case 0xF2: /* jp p, address */
            if (!SIGN_FLAG) {
                REG_PC = mem_fetch_word(REG_PC);
            } else {
                REG_PC += 2;
            }
            break;
        case 0xFA: /* jp m, address */
            if (SIGN_FLAG) {
                REG_PC = mem_fetch_word(REG_PC);
            } else {
                REG_PC += 2;
            }
            break;

        case 0x18: /* jr offset */
            REG_PC += (int8_t)mem_fetch(REG_PC);
            REG_PC++;
            break;

        case 0x20: /* jr nz, offset */
            if (!ZERO_FLAG) {
                REG_PC += (int8_t)mem_fetch(REG_PC);
                TSTATE += 5;
            }
            REG_PC++;
            break;
        case 0x28: /* jr z, offset */
            if (ZERO_FLAG) {
                REG_PC += (int8_t)mem_fetch(REG_PC);
                TSTATE += 5;
            }
            REG_PC++;
            break;
        case 0x30: /* jr nc, offset */
            if (!CARRY_FLAG) {
                REG_PC += (int8_t)mem_fetch(REG_PC);
                TSTATE += 5;
            }
            REG_PC++;
            break;
        case 0x38: /* jr c, offset */
            if (CARRY_FLAG) {
                REG_PC += (int8_t)mem_fetch(REG_PC);
                TSTATE += 5;
            }
            REG_PC++;
            break;

        case 0x7F: /* ld a, a */
            REG_A = REG_A;
            break;
        case 0x78: /* ld a, b */
            REG_A = REG_B;
            break;
        case 0x79: /* ld a, c */
            REG_A = REG_C;
            break;
        case 0x7A: /* ld a, d */
            REG_A = REG_D;
            break;
        case 0x7B: /* ld a, e */
            REG_A = REG_E;
            break;
        case 0x7C: /* ld a, h */
            REG_A = ix->byte.high;
            break;
        case 0x7D: /* ld a, l */
            REG_A = ix->byte.low;
            break;
        case 0x47: /* ld b, a */
            REG_B = REG_A;
            break;
        case 0x40: /* ld b, b */
            REG_B = REG_B;
            break;
        case 0x41: /* ld b, c */
            REG_B = REG_C;
            break;
        case 0x42: /* ld b, d */
            REG_B = REG_D;
            break;
        case 0x43: /* ld b, e */
            REG_B = REG_E;
            break;
        case 0x44: /* ld b, h */
            REG_B = ix->byte.high;
            break;
        case 0x45: /* ld b, l */
            REG_B = ix->byte.low;
            break;
        case 0x4F: /* ld c, a */
            REG_C = REG_A;
            break;
        case 0x48: /* ld c, b */
            REG_C = REG_B;
            break;
        case 0x49: /* ld c, c */
            REG_C = REG_C;
            break;
        case 0x4A: /* ld c, d */
            REG_C = REG_D;
            break;
        case 0x4B: /* ld c, e */
            REG_C = REG_E;
            break;
        case 0x4C: /* ld c, h */
            REG_C = ix->byte.high;
            break;
        case 0x4D: /* ld c, l */
            REG_C = ix->byte.low;
            break;
        case 0x57: /* ld d, a */
            REG_D = REG_A;
            break;
        case 0x50: /* ld d, b */
            REG_D = REG_B;
            break;
        case 0x51: /* ld d, c */
            REG_D = REG_C;
            break;
        case 0x52: /* ld d, d */
            REG_D = REG_D;
            break;
        case 0x53: /* ld d, e */
            REG_D = REG_E;
            break;
        case 0x54: /* ld d, h */
            REG_D = ix->byte.high;
            break;
        case 0x55: /* ld d, l */
            REG_D = ix->byte.low;
            break;
        case 0x5F: /* ld e, a */
            REG_E = REG_A;
            break;
        case 0x58: /* ld e, b */
            REG_E = REG_B;
            break;
        case 0x59: /* ld e, c */
            REG_E = REG_C;
            break;
        case 0x5A: /* ld e, d */
            REG_E = REG_D;
            break;
        case 0x5B: /* ld e, e */
            REG_E = REG_E;
            break;
        case 0x5C: /* ld e, h */
            REG_E = ix->byte.high;
            break;
        case 0x5D: /* ld e, l */
            REG_E = ix->byte.low;
            break;
        case 0x67: /* ld h, a */
            ix->byte.high = REG_A;
            break;
        case 0x60: /* ld h, b */
            ix->byte.high = REG_B;
            break;
        case 0x61: /* ld h, c */
            ix->byte.high = REG_C;
            break;
        case 0x62: /* ld h, d */
            ix->byte.high = REG_D;
            break;
        case 0x63: /* ld h, e */
            ix->byte.high = REG_E;
            break;
        case 0x64: /* ld h, h */
            ix->byte.high = ix->byte.high;
            break;
        case 0x65: /* ld h, l */
            ix->byte.high = ix->byte.low;
            break;
        case 0x6F: /* ld l, a */
            ix->byte.low = REG_A;
            break;
        case 0x68: /* ld l, b */
            ix->byte.low = REG_B;
            break;
        case 0x69: /* ld l, c */
            ix->byte.low = REG_C;
            break;
        case 0x6A: /* ld l, d */
            ix->byte.low = REG_D;
            break;
        case 0x6B: /* ld l, e */
            ix->byte.low = REG_E;
            break;
        case 0x6C: /* ld l, h */
            ix->byte.low = ix->byte.high;
            break;
        case 0x6D: /* ld l, l */
            ix->byte.low = ix->byte.low;
            break;

        case 0x02: /* ld (bc), a */
            mem_write(REG_BC, REG_A);
            break;
        case 0x12: /* ld (de), a */
            mem_write(REG_DE, REG_A);
            break;
        case 0x77: /* ld (hl), a */
            mem_write(get_hl_addr(ix), REG_A);
            break;
        case 0x70: /* ld (hl), b */
            mem_write(get_hl_addr(ix), REG_B);
            break;
        case 0x71: /* ld (hl), c */
            mem_write(get_hl_addr(ix), REG_C);
            break;
        case 0x72: /* ld (hl), d */
            mem_write(get_hl_addr(ix), REG_D);
            break;
        case 0x73: /* ld (hl), e */
            mem_write(get_hl_addr(ix), REG_E);
            break;
        case 0x74: /* ld (hl), h */
            mem_write(get_hl_addr(ix), REG_H);
            break;
        case 0x75: /* ld (hl), l */
            mem_write(get_hl_addr(ix), REG_L);
            break;

        case 0x7E: /* ld a, (hl) */
            REG_A = mem_read(get_hl_addr(ix));
            break;
        case 0x46: /* ld b, (hl) */
            REG_B = mem_read(get_hl_addr(ix));
            break;
        case 0x4E: /* ld c, (hl) */
            REG_C = mem_read(get_hl_addr(ix));
            break;
        case 0x56: /* ld d, (hl) */
            REG_D = mem_read(get_hl_addr(ix));
            break;
        case 0x5E: /* ld e, (hl) */
            REG_E = mem_read(get_hl_addr(ix));
            break;
        case 0x66: /* ld h, (hl) */
            REG_H = mem_read(get_hl_addr(ix));
            break;
        case 0x6E: /* ld l, (hl) */
            REG_L = mem_read(get_hl_addr(ix));
            break;

        case 0x3E: /* ld a, value */
            REG_A = mem_fetch(REG_PC++);
            break;
        case 0x06: /* ld b, value */
            REG_B = mem_fetch(REG_PC++);
            break;
        case 0x0E: /* ld c, value */
            REG_C = mem_fetch(REG_PC++);
            break;
        case 0x16: /* ld d, value */
            REG_D = mem_fetch(REG_PC++);
            break;
        case 0x1E: /* ld e, value */
            REG_E = mem_fetch(REG_PC++);
            break;
        case 0x26: /* ld h, value */
            ix->byte.high = mem_fetch(REG_PC++);
            break;
        case 0x2E: /* ld l, value */
            ix->byte.low = mem_fetch(REG_PC++);
            break;

        case 0x01: /* ld bc, value */
            REG_BC = mem_fetch_word(REG_PC);
            REG_PC += 2;
            break;
        case 0x11: /* ld de, value */
            REG_DE = mem_fetch_word(REG_PC);
            REG_PC += 2;
            break;
        case 0x21: /* ld hl, value */
            ix->word = mem_fetch_word(REG_PC);
            REG_PC += 2;
            break;
        case 0x31: /* ld sp, value */
            REG_SP = mem_fetch_word(REG_PC);
            REG_PC += 2;
            break;

        case 0x3A: /* ld a, (address) */
            /* this one is missing from Zaks */
            REG_A = mem_read(mem_fetch_word(REG_PC));
            REG_PC += 2;
            break;

        case 0x0A: /* ld a, (bc) */
            REG_A = mem_read(REG_BC);
            break;
        case 0x1A: /* ld a, (de) */
            REG_A = mem_read(REG_DE);
            break;

        case 0x32: /* ld (address), a */
            mem_write(mem_fetch_word(REG_PC), REG_A);
            REG_PC += 2;
            break;

        case 0x22: /* ld (address), hl */
            mem_write_word(mem_fetch_word(REG_PC), ix->word);
            REG_PC += 2;
            break;

        case 0x36: /* ld (hl), value */
        {
            uint16_t addr = get_hl_addr(ix);
            mem_write(addr, mem_fetch(REG_PC++));
            break;
        }

        case 0x2A: /* ld hl, (address) */
            ix->word = mem_read_word(mem_fetch_word(REG_PC));
            REG_PC += 2;
            break;

        case 0xF9: /* ld sp, hl */
            REG_SP = ix->word;
            break;

        case 0x00: /* nop */
            break;

        case 0xF6: /* or value */
            do_or_byte(mem_fetch(REG_PC++));
            break;

        case 0xB7: /* or a */
            do_or_byte(REG_A);
            break;
        case 0xB0: /* or b */
            do_or_byte(REG_B);
            break;
        case 0xB1: /* or c */
            do_or_byte(REG_C);
            break;
        case 0xB2: /* or d */
            do_or_byte(REG_D);
            break;
        case 0xB3: /* or e */
            do_or_byte(REG_E);
            break;
        case 0xB4: /* or h */
            do_or_byte(ix->byte.high);
            break;
        case 0xB5: /* or l */
            do_or_byte(ix->byte.low);
            break;

        case 0xB6: /* or (hl) */
            do_or_byte(mem_read(get_hl_addr(ix)));
            break;

        case 0xD3: /* out (port), a */
            z80_out(mem_fetch(REG_PC++), REG_A);
            break;

        case 0xC1: /* pop bc */
            REG_BC = mem_read_word(REG_SP);
            REG_SP += 2;
            break;
        case 0xD1: /* pop de */
            REG_DE = mem_read_word(REG_SP);
            REG_SP += 2;
            break;
        case 0xE1: /* pop hl */
            ix->word = mem_read_word(REG_SP);
            REG_SP += 2;
            break;
        case 0xF1: /* pop af */
            REG_AF = mem_read_word(REG_SP);
            REG_SP += 2;
            break;

        case 0xC5: /* push bc */
            REG_SP -= 2;
            mem_write_word(REG_SP, REG_BC);
            break;
        case 0xD5: /* push de */
            REG_SP -= 2;
            mem_write_word(REG_SP, REG_DE);
            break;
        case 0xE5: /* push hl */
            REG_SP -= 2;
            mem_write_word(REG_SP, ix->word);
            break;
        case 0xF5: /* push af */
            REG_SP -= 2;
            mem_write_word(REG_SP, REG_AF);
            break;

        case 0xC9: /* ret */
            REG_PC = mem_read_word(REG_SP);
            REG_SP += 2;
            break;

        case 0xC0: /* ret nz */
            if (!ZERO_FLAG) {
                REG_PC = mem_read_word(REG_SP);
                REG_SP += 2;
                TSTATE += 6;
            }
            break;
        case 0xC8: /* ret z */
            if (ZERO_FLAG) {
                REG_PC = mem_read_word(REG_SP);
                REG_SP += 2;
                TSTATE += 6;
            }
            break;
        case 0xD0: /* ret nc */
            if (!CARRY_FLAG) {
                REG_PC = mem_read_word(REG_SP);
                REG_SP += 2;
                TSTATE += 6;
            }
            break;
        case 0xD8: /* ret c */
            if (CARRY_FLAG) {
                REG_PC = mem_read_word(REG_SP);
                REG_SP += 2;
                TSTATE += 6;
            }
            break;
        case 0xE0: /* ret po */
            if (!PARITY_FLAG) {
                REG_PC = mem_read_word(REG_SP);
                REG_SP += 2;
                TSTATE += 6;
            }
            break;
        case 0xE8: /* ret pe */
            if (PARITY_FLAG) {
                REG_PC = mem_read_word(REG_SP);
                REG_SP += 2;
                TSTATE += 6;
            }
            break;
        case 0xF0: /* ret p */
            if (!SIGN_FLAG) {
                REG_PC = mem_read_word(REG_SP);
                REG_SP += 2;
                TSTATE += 6;
            }
            break;
        case 0xF8: /* ret m */
            if (SIGN_FLAG) {
                REG_PC = mem_read_word(REG_SP);
                REG_SP += 2;
                TSTATE += 6;
            }
            break;

        case 0x17: /* rla */
            do_rla();
            break;

        case 0x07: /* rlca */
            do_rlca();
            break;

        case 0x1F: /* rra */
            do_rra();
            break;

        case 0x0F: /* rrca */
            do_rrca();
            break;

        case 0xC7: /* rst 00h */
            REG_SP -= 2;
            mem_write_word(REG_SP, REG_PC);
            REG_PC = 0x00;
            break;
        case 0xCF: /* rst 08h */
            REG_SP -= 2;
            mem_write_word(REG_SP, REG_PC);
            REG_PC = 0x08;
            break;
        case 0xD7: /* rst 10h */
            REG_SP -= 2;
            mem_write_word(REG_SP, REG_PC);
            REG_PC = 0x10;
            break;
        case 0xDF: /* rst 18h */
            REG_SP -= 2;
            mem_write_word(REG_SP, REG_PC);
            REG_PC = 0x18;
            break;
        case 0xE7: /* rst 20h */
            REG_SP -= 2;
            mem_write_word(REG_SP, REG_PC);
            REG_PC = 0x20;
            break;
        case 0xEF: /* rst 28h */
            REG_SP -= 2;
            mem_write_word(REG_SP, REG_PC);
            REG_PC = 0x28;
            break;
        case 0xF7: /* rst 30h */
            REG_SP -= 2;
            mem_write_word(REG_SP, REG_PC);
            REG_PC = 0x30;
            break;
        case 0xFF: /* rst 38h */
            REG_SP -= 2;
            mem_write_word(REG_SP, REG_PC);
            REG_PC = 0x38;
            break;

        case 0x37: /* scf */
            REG_F = (REG_F | CARRY_MASK) & ~(SUBTRACT_MASK | HALF_CARRY_MASK);
            break;

        case 0x9F: /* sbc a, a */
            do_sbc_byte(REG_A);
            break;
        case 0x98: /* sbc a, b */
            do_sbc_byte(REG_B);
            break;
        case 0x99: /* sbc a, c */
            do_sbc_byte(REG_C);
            break;
        case 0x9A: /* sbc a, d */
            do_sbc_byte(REG_D);
            break;
        case 0x9B: /* sbc a, e */
            do_sbc_byte(REG_E);
            break;
        case 0x9C: /* sbc a, h */
            do_sbc_byte(ix->byte.high);
            break;
        case 0x9D: /* sbc a, l */
            do_sbc_byte(ix->byte.low);
            break;
        case 0xDE: /* sbc a, value */
            do_sbc_byte(mem_fetch(REG_PC++));
            break;
        case 0x9E: /* sbc a, (hl) */
            do_sbc_byte(mem_read(get_hl_addr(ix)));
            break;

        case 0x97: /* sub a, a */
            do_sub_byte(REG_A);
            break;
        case 0x90: /* sub a, b */
            do_sub_byte(REG_B);
            break;
        case 0x91: /* sub a, c */
            do_sub_byte(REG_C);
            break;
        case 0x92: /* sub a, d */
            do_sub_byte(REG_D);
            break;
        case 0x93: /* sub a, e */
            do_sub_byte(REG_E);
            break;
        case 0x94: /* sub a, h */
            do_sub_byte(ix->byte.high);
            break;
        case 0x95: /* sub a, l */
            do_sub_byte(ix->byte.low);
            break;
        case 0xD6: /* sub a, value */
            do_sub_byte(mem_fetch(REG_PC++));
            break;
        case 0x96: /* sub a, (hl) */
            do_sub_byte(mem_read(get_hl_addr(ix)));
            break;

        case 0xEE: /* xor value */
            do_xor_byte(mem_fetch(REG_PC++));
            break;

        case 0xAF: /* xor a */
            do_xor_byte(REG_A);
            break;
        case 0xA8: /* xor b */
            do_xor_byte(REG_B);
            break;
        case 0xA9: /* xor c */
            do_xor_byte(REG_C);
            break;
        case 0xAA: /* xor d */
            do_xor_byte(REG_D);
            break;
        case 0xAB: /* xor e */
            do_xor_byte(REG_E);
            break;
        case 0xAC: /* xor h */
            do_xor_byte(ix->byte.high);
            break;
        case 0xAD: /* xor l */
            do_xor_byte(ix->byte.low);
            break;
        case 0xAE: /* xor (hl) */
            do_xor_byte(mem_read(get_hl_addr(ix)));
            break;
        }
    } while (continuous);
    return halted;
}

void z80_reset(void)
{
    REG_PC = 0;
    z80_state.i = 0;
    z80_state.iff1 = false;
    z80_state.iff2 = false;
    z80_state.ei_shadow = false;
    z80_state.interrupt_mode = 0;
    z80_state.nmi_in_progress = false;
    z80_state.signal_eoi = false;
    /* z80_state.r = 0; */
}

#define WREG(U, L)                                                             \
    if (z80_state.L.word != old_state.L.word) {                                \
        fprintf(tracef, " %s=%04X", #U, z80_state.L.word);                     \
        old_state.L.word = z80_state.L.word;                                   \
    }
#define BREG(U, L)                                                             \
    if (z80_state.L != old_state.L) {                                          \
        fprintf(tracef, " %s=%02X", #U, z80_state.L);                          \
        old_state.L = z80_state.L;                                             \
    }

static void diffstate(void)
{
    static struct z80_state_struct old_state;

    if (!tracing(TRACE_CPU))
        return;

    BREG(A, af.byte.high);
    WREG(BC, bc);
    WREG(DE, de);
    WREG(HL, hl);
    WREG(IX, ix);
    WREG(IY, iy);
    WREG(SP, sp);
    /* WREG(PC,pc); */
    BREG(F, af.byte.low);
    WREG(AFx, af_prime);
    WREG(BCx, bc_prime);
    WREG(DEx, de_prime);
    WREG(HLx, hl_prime);
}
