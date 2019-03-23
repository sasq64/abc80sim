/*
 * Common definitions that we need for everything
 */

#ifndef COMPILER_H
#define COMPILER_H

#ifdef HAVE_CONFIG_H
#    include "../config/config.h"
#else
#    error "Need compiler-specific hacks here"
#endif

/* On Microsoft platforms we support multibyte character sets in filenames */
#define _MBCS 1

#ifdef HAVE_INTTYPES_H
#    include <inttypes.h>
#else
#    include "clib/inttypes.h" /* Ersatz header file */
#endif

/* These header files should pretty much always be included... */
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef HAVE_FCNTL_H
#    include <fcntl.h>
#endif
#ifdef HAVE_SYS_STAT_H
#    include <sys/stat.h>
#endif

#ifdef HAVE_UNISTD_H
#    include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#    include <sys/types.h>
#endif
#ifdef HAVE_DIRENT_H
#    include <dirent.h>
#endif
#ifdef HAVE_VFORK_H
#    include <vfork.h>
#endif
#ifdef HAVE_PATHS_H
#    include <paths.h>
#endif

#ifdef HAVE_IO_H
#    include <io.h>
#endif
#ifdef HAVE_WINDOWS_H
#    include <windows.h>
#endif
#ifdef HAVE_DIRECT_H
#    include <direct.h>
#endif

#include <SDL.h> /* This includes endian definitions */

#define WORDS_LITTLEENDIAN (SDL_BYTEORDER == SDL_LIL_ENDIAN)
#define WORDS_BIGENDIAN (SDL_BYTEORDER == SDL_BIG_ENDIAN)

#ifndef __cplusplus /* C++ has false, true, bool as keywords */
#    ifdef HAVE_STDBOOL_H
#        include <stdbool.h>
#    else
/* This is sort of dangerous, since casts will behave different than
   casting to the standard boolean type.  Always use !!, not (bool). */
typedef enum bool
{
    false,
    true
} bool;
#    endif
#endif

/*
 * asprintf()
 */
#ifndef HAVE_ASPRINTF
extern int asprintf(char**, const char*, ...);
#endif

/*
 * mode_t
 */
#ifndef HAVE_MODE_T
typedef int mode_t;
#endif

/*
 * Hack to support external-linkage inline functions
 */
#ifndef HAVE_STDC_INLINE
#    ifdef __GNUC__
#        ifdef __GNUC_STDC_INLINE__
#            define HAVE_STDC_INLINE
#        else
#            define HAVE_GNU_INLINE
#        endif
#    elif defined(__GNUC_GNU_INLINE__)
/* Some other compiler implementing only GNU inline semantics? */
#        define HAVE_GNU_INLINE
#    elif defined(__STDC_VERSION__)
#        if __STDC_VERSION__ >= 199901L
#            define HAVE_STDC_INLINE
#        endif
#    endif
#endif

#ifdef HAVE_STDC_INLINE
#    define extern_inline inline
#elif defined(HAVE_GNU_INLINE)
#    define extern_inline extern inline
#    define inline_prototypes
#else
#    define inline_prototypes
#endif

/*
 * Hints to the compiler that a particular branch of code is more or
 * less likely to be taken.
 */
#if HAVE___BUILTIN_EXPECT
#    define likely(x) __builtin_expect(!!(x), 1)
#    define unlikely(x) __builtin_expect(!!(x), 0)
#else
#    define likely(x) (!!(x))
#    define unlikely(x) (!!(x))
#endif

/*
 * How to tell the compiler that a function doesn't return
 */
/* #ifdef HAVE_STDNORETURN_H */
/* #    include <stdnoreturn.h> */
/* #    define no_return noreturn void */
/* #elif defined(HAVE_FUNC_ATTRIBUTE_NORETURN) */
/* #    define no_return void __attribute__((noreturn)) */
/* #elif defined(_MSC_VER) */
/* #    define no_return __declspec(noreturn) void */
/* #else */
/* #    define no_return void */
/* #endif */

/*
 * How to tell the compiler that a function is pure arithmetic
 */
#ifdef HAVE_FUNC_ATTRIBUTE_CONST
#    define const_func __attribute__((const))
#else
#    define const_func
#endif

/*
 * This function has no side effects, but depends on its arguments,
 * memory pointed to by its arguments, or global variables.
 * NOTE: functions that return a value by modifying memory pointed to
 * by a pointer argument are *NOT* considered pure.
 */
#ifdef HAVE_FUNC_ATTRIBUTE_PURE
#    define pure_func __attribute__((pure))
#else
#    define pure_func
#endif

/* Determine probabilistically if something is a compile-time constant */
#ifdef HAVE___BUILTIN_CONSTANT_P
#    define is_constant(x) __builtin_constant_p(x)
#else
#    define is_constant(x) false
#endif

/* Simple atomic operations */
#ifdef __GNUC__
#    define atomic_load(p) __atomic_load_n((p), __ATOMIC_ACQUIRE)
#    define atomic_store(p, v) __atomic_store_n((p), (v), __ATOMIC_RELEASE)
#    define cmpxchg(p, e, d)                                                   \
        likely(__atomic_compare_exchange_n(                                    \
            (p), (e), (d), false, __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE))
#    define xchg(p, v) __atomic_exchange_n((p), (v), __ATOMIC_ACQ_REL)
#    define barrier() __atomic_thread_fence(__ATOMIC_ACQ_REL)

#    if 0 // defined(__i386__) || defined(__x86_64__)
static inline bool
atomic_test_set_bit(volatile unsigned int *p, unsigned int v)
{
  bool b;
  asm volatile("lock btsl %2,%0"
	       : "+m" (*p), "=@ccc" (b)
	       : "rN" (v)
	       : "memory");
  return b;
}
static inline bool
atomic_test_clear_bit(volatile unsigned int *p, unsigned int v)
{
  bool b;
  asm volatile("lock btrl %2,%0"
	       : "+m" (*p), "=@ccc" (b)
	       : "rN" (v)
	       : "memory");
  return b;
}
#    else
static inline bool atomic_test_set_bit(volatile unsigned int* p, unsigned int v)
{
    return (__atomic_fetch_or(p, 1U << v, __ATOMIC_ACQ_REL) >> v) & 1;
}
static inline bool atomic_test_clear_bit(volatile unsigned int* p,
                                         unsigned int v)
{
    return (__atomic_fetch_and(p, ~(1U << v), __ATOMIC_ACQ_REL) >> v) & 1;
}
#    endif

#    define atomic_set_bit(p, v) ((void)atomic_test_set_bit(p, v))
#    define atomic_clear_bit(p, v) ((void)atomic_test_clear_bit(p, v))

#else /* __GNUC__ */
#    error "Define atomic operations for your compiler here"
#endif

#endif /* COMPILER_H */
