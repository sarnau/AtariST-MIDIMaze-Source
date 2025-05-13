/*
 *  MAC.C
 *  MIDIMAZE
 *
 *  Created by Markus Fritze on 9/1/10.
 *
 */

extern "C" {
#include "globals.h"
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
    return ask_joystick();
}

/* If keyboard is selected, it will behave the same as joystick */
void init_keyboard(void)
{
    init_joystick();
}

void exit_keyboard(void)
{
    exit_joystick();
}

int ask_keyboard(void)
{
    return ask_joystick();
}
