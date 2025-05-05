#include "globals.h"

#ifdef __PUREC__
static long muls(short a, short b, short c) 0xc1c1; /* muls.w d1,d0 */
static short divs(long a) 0x81c2; /* divs.w d2,d0 */
#endif

/*
 * Multiply and divide 3 16-bit signed integer values with the temporary
 * result being 32-bit wide.
 */
short muls_divs(short a, short b, short c)
{
#if BUGFIX_DIVISION_BY_ZERO
    /* This is probably not necessary, but there might be edge-cases where it is. */
    if (c == 0)
        return a;
#endif
#ifdef __m68k__
#ifdef __GNUC__
	short ret;
	__asm__ __volatile__(
		"muls %2,%0\n"
		"divs %3,%0\n"
	: "=d"(ret)
	: "0"(a), "d"(b), "g"(c)
	: "cc");
	return ret;
#endif
#ifdef __PUREC__
	return divs(muls(a, b, c));
#endif
#else
    return (a * b) / c;
#endif
}
