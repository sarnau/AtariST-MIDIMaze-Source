/************************************************************
 *** LOWMEM.C
 ************************************************************/
#include "globals.h"

#ifdef __atarist__

static miditimeout_t read_vbclock_ret;

/************************************************************
 *** Returns the lower 16-bit of the vbclock.
 *** However this is a BUG, which triggers random MIDI timeouts
 *** if the timer overflows at the wrong time.
 *** It should have return the full vbclock, all 32-bit
 ************************************************************/
static long sub_read_vbclock(void) {
#if BUGFIX_MIDI_TIMEOUT
    read_vbclock_ret = *(long*)0x462;
#else
    read_vbclock_ret = *(short*)0x464;
#endif
    return 0;
}

/************************************************************
 *** Returns the vbclock()
 ************************************************************/
miditimeout_t read_vbclock(void) {
    Supexec(&sub_read_vbclock);
    return read_vbclock_ret;
}

#else

#include <sys/time.h>

miditimeout_t read_vbclock(void)
{
    struct timeval tval;

	gettimeofday(&tval, NULL);
	/* Convert into a 60 Hz counter */
	return (tval.tv_sec * 1000000L + tval.tv_usec) / (1000 * (1000 / 60));
}

#endif
