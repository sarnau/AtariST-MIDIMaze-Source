/*
 *  MAC.C
 *  MIDIMAZE
 *
 *  Created by Markus Fritze on 9/1/10.
 *
 */

#include "GLOBALS.H"
#include <sys/time.h>

// We do not have low-memory variables, so we implement VBL Clock as a 60Hz counter
long read_vbclock(void) {
    struct timeval tval;
    gettimeofday(&tval, NULL);
    // Convert into a 60 Hz counter
    return (tval.tv_sec * 1000000LL + tval.tv_usec)/(1000*(1000/60));
}

// If mouse is selected, it will behave the same as joystick
void init_mouse(void)
{
    init_joystick();
}

void exit_mouse(void)
{
    exit_joystick();
}

int ask_mouse(void)
{
    int button = 0;
    return ask_joystick(0, &button) | ((button != 0) << 4);
}

// Not conterm on macOS. The compatibility layer always returns it.
void _conterm_update_shift_status(int setFlag)
{
}
