/************************************************************
 *** Dispatch code to switch between the different modes: master, solo, slave and MIDIcam
 ************************************************************/
#include "globals.h"


/************************************************************
 *** Main Loop to allow switching between different game modes
 ************************************************************/
void dispatch(void) {
int action;

#if NON_ATARI_HACK
    action = DISPATCH_SOLO;
#else
    action = DISPATCH_AUTOMATIC;   /* start with automatic detection */
#endif
    do {
        switch(action) {
        case DISPATCH_QUIT:
            break;

        case DISPATCH_MASTER:
            action = master_solo_loop(NO);
            break;

        case DISPATCH_SLAVE:
            action = slave_midicam_loop(NO);
            break;

        case DISPATCH_SOLO:
            action = master_solo_loop(YES);
            break;

        case DISPATCH_AUTOMATIC:
            /* check if somebody is responding to a MIDI byte */
            /* If not, we'll be a Master */

            /* remove all pending MIDI bytes */
            while(Bconstat(MIDI))
                Bconin(MIDI);

            Bconout(MIDI, 0);
            own_number = get_midi(MIDI_DEFAULT_TIMEOUT);
            action = own_number != 0 ? DISPATCH_SLAVE : DISPATCH_MASTER;
            break;

        case DISPATCH_MIDICAM:
            action = slave_midicam_loop(YES);
            break;
        }
    } while(action != DISPATCH_QUIT);
}
