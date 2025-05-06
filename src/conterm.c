/************************************************************
 *** CONTERM.C
 ************************************************************/
#include "globals.h"

#ifdef __atarist__

/************************************************************
 *** Cause Bconin to return the current value of Kbshift in the bits 24..31.
 ************************************************************/
static long _conterm_set_shift(void) {
    *(char*)0x484 |= 8;
    return 0;
}

/************************************************************
 *** Reset to default
 ************************************************************/
static long _conterm_clear_shift(void) {
    *(char*)0x484 &= ~8;
    return 0;
}

/************************************************************
 *** void _conterm_update_shift_status(int setFlag)
 ************************************************************/
void _conterm_update_shift_status(int setFlag) {
    Supexec(setFlag ? _conterm_set_shift : _conterm_clear_shift);
}

#else

/* Not conterm on non-atari platforms. The compatibility layer always returns it. */
void _conterm_update_shift_status(int setFlag)
{
    UNUSED(setFlag);
}

#endif
