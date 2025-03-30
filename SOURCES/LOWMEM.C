/************************************************************
 *** LOWMEM.C
 ************************************************************/
#include "GLOBALS.H"

#if BUGFIX_MIDI_TIMEOUT
static long read_vbclock_ret;
#else
static short read_vbclock_ret;
#endif

/************************************************************
 *** Returns the lower 16-bit of the vbclock.
 *** However this is a BUG, which triggers random MIDI timeouts
 *** if the timer overflows at the wrong time.
 *** It should have return the full vbclock, all 32-bit
 ************************************************************/
static void sub_read_vbclock(void) {
#if BUGFIX_MIDI_TIMEOUT
    read_vbclock_ret = *(long*)0x462;
#else
    read_vbclock_ret = *(int*)0x464;
#endif
}

/************************************************************
 *** Returns the vbclock()
 ************************************************************/
#if BUGFIX_MIDI_TIMEOUT
long read_vbclock(void) {
#else
int read_vbclock(void) {
#endif
    Supexec(&sub_read_vbclock);
    return read_vbclock_ret;
}
