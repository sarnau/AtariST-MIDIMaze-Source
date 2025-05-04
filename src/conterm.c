/************************************************************
 *** CONTERM.C
 ************************************************************/
#include "globals.h"

/************************************************************
 *** Cause Bconin to return the current value of Kbshift in the bits 24..31.
 ************************************************************/
static void _conterm_set_shift(void) {
    *(char*)0x484 |= 8;
}

/************************************************************
 *** Reset to default
 ************************************************************/
static void _conterm_clear_shift(void) {
    *(char*)0x484 &= ~8;
}

/************************************************************
 *** void _conterm_update_shift_status(int setFlag)
 ************************************************************/
void _conterm_update_shift_status(int setFlag) {
    Supexec(setFlag ? &_conterm_set_shift : &_conterm_clear_shift);
}
