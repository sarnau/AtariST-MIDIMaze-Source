#include "portab.h"
#include "gemdefs.h"
#include "vdilib.h"

#ifdef __PUREC__
static long save_a2(long d0, long d1) 0x2f0a; /* move.l a2,-(sp) */
static long trap_2_opcode(long d0) 0x4e42;    /* trap #2 */
static void rest_a2(long) 0x245f;             /* move.l (sp)+,a2 */
#define trap_2(d0, d1) rest_a2(trap_2_opcode(save_a2(d0, d1)))
#endif

short	contrl[12];
short	intin[128];
short	ptsin[128];
short	intout[128];
short	ptsout[128];

VDIPB pblock = { contrl, intin, ptsin, intout, ptsout };

void vdi(void)
{
#ifdef __GNUC__
    __asm__ volatile
    (
        "move.l  %0,%%d1\n\t"
        "moveq   #115,%%d0\n\t"
        "trap    #2"
    :
    : "g"(&pblock)
    : "d0", "d1", "d2", "a0", "a1", "a2", "memory", "cc"
    );
#endif
#ifdef __PUREC__
	trap_2(115, (long)&pblock);
#endif
}
