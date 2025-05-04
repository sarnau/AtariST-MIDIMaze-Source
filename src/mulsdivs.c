#include "globals.h"

/*
* // Multiply and divide 3 16-bit signed integer values with the temporary
* // result being 32-bit wide.
* // 
*int muls_divs(a,b,c);
*int a,b,c;
*{
*  return (a * b) / c;
*}

.globl	muls_divs
.text
muls_divs:
_muls_divs:
move    4(A7),d0
muls    6(A7),d0
divs    8(A7),d0
rts

* // This code is never called. It is probably some leftover, maybe from a former
* // overflow check in the function above?

.globl	return_zero
.text
return_zero:
_return_zero:
clr     d0
rts
*/

int muls_divs(int a,int b,int c)
{
#if BUGFIX_DIVISION_BY_ZERO
    // This is probably not necessary, but there might be edge-cases where it is.
    if(c == 0)
        return a;
#endif
    return a * b / c;
}

