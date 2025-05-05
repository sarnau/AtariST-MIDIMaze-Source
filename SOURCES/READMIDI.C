/************************************************************
 *** READMIDI.C
 ************************************************************/
#include "globals.h"


/*
MIDImaze 1 suffered from weird MIDI timeouts. This function
is the reason for it, because it calculates the timeout only
in 16-bit instead of the full 32-bit. On an overflow this
will fail. This can happen every few hours, but also depends
on how long a machine is already running.
*/

/************************************************************
 *** Wait for a MIDI byte, return FAILURE, if a timeout occurs.
 ************************************************************/
int get_midi(int timeout) {
#if BUGFIX_MIDI_TIMEOUT
    static long midi_timeout_value;
#else
    static short midi_timeout_value;
#endif
    midi_timeout_value = read_vbclock()+timeout;
    do {
        if(Bconstat(MIDI))
            return Bconin(MIDI)&0xff;

#if BUGFIX_MIDI_TIMEOUT
        if((read_vbclock() - midi_timeout_value) >= 0)
#else
        if(!(read_vbclock()-midi_timeout_value&0x8000)) /* check for a negative 16-bit sign */
#endif
            return FAILURE;
    } while(1);
}
