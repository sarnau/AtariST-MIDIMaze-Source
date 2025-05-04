#ifndef __GNUC__
# ifndef __PUREC__
#   ifndef __TURBOC__
#     ifndef LATTICE
        /* assume Alcyon, if none of the other known symbols is defined */
#       ifndef __ALCYON__
#         define __ALCYON__ 1
#       endif
#     endif
#   endif
# endif
#endif
#ifndef __m68k__
#ifdef __ALCYON__
#  define __m68k__ 1
#endif
#ifdef __TURBOC__
#  define __m68k__ 1
#endif
#ifdef LATTICE
#  define __m68k__ 1
#endif
#endif
#ifndef __atarist__
#ifdef __ALCYON__
#  define __atarist__ 1
#endif
#ifdef __TOS__
#  define __atarist__ 1
#endif
#ifdef LATTICE
#  define __atarist__ 1
#endif
#endif

// This is all Atari specific code, which shipped with the compiler:

#ifndef NO
    #define NO 0            /* "FALSE" */
#endif
#ifndef YES
    #define YES 1            /* "TRUE" */
#endif

#ifndef FALSE
#define TRUE 1
#define FALSE 0
#endif

#ifdef __TURBOC__
#define __CDECL cdecl
#endif

#ifndef __CDECL
#define __CDECL
#endif

#ifndef __attribute__
#  ifndef __GNUC__
#    define __attribute__(x)
#  endif
#endif

#ifndef __GNUC_PREREQ
# ifdef __GNUC__
#  ifdef __GNUC_MINOR__
#   define __GNUC_PREREQ(maj, min) ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#  endif
# endif
#endif
#ifndef __GNUC_PREREQ
# define __GNUC_PREREQ(maj, min) 0
#endif

#ifndef __CLOBBER_RETURN
#if __GNUC_PREREQ(3, 3)
# define __CLOBBER_RETURN(a) 
#else
# define __CLOBBER_RETURN(a) a,
#endif
#endif

#ifndef AND_MEMORY
#if __GNUC_PREREQ(2, 6)
#define AND_MEMORY , "memory"
#define ONLY_MEMORY "memory"
#else
#define AND_MEMORY
#define ONLY_MEMORY
#endif
#endif

#ifndef __EXITING
#if __GNUC_PREREQ(2, 5)
#define __EXITING void
#else
#define __EXITING volatile void
#endif
#endif
#ifndef __NORETURN
#define __NORETURN __attribute__ ((noreturn))
#endif

#ifndef UNUSED
# ifdef __ALCYON__
#  define UNUSED(x)
# else
#  define UNUSED(x) ((void)(x))
# endif
#endif

#ifndef NO_CONST
#  ifdef __GNUC__
#	 define NO_CONST(p) __extension__({ union { const void *cs; void *s; } x; x.cs = p; x.s; })
#  else
#    ifdef __ALCYON__ /* Alcyon parses the cast as function call??? */
#      define NO_CONST(p) p
#    else
#      define NO_CONST(p) ((void *)(p))
#    endif
#  endif
#endif

#ifndef NULL
#define NULL	((void *)0)			/*	Null pointer value		*/
#endif

#define FAILURE    (-1)         /* Function failure return val */
#define SUCCESS    (0)          /* Function success return val */

#ifdef __PUREC__
typedef unsigned long uint32_t;
#else
typedef unsigned int uint32_t;
#endif

#undef abs
#define abs(x) ((x) < 0 ? -(x) : (x))
