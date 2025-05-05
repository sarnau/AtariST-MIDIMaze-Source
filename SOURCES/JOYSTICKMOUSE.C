/************************************************************
 *** Joystick/Mouse code
 ************************************************************/
#include "globals.h"

static void own_joyvec(char *buf);
static void own_mousevec(char *buf);

static const char mouse_defaults[4] = { 0x00,0x00,0x01,0x01 };
static short joystick_package_received_flag;
static short init_joy_flag; /* button is pressed (unused variable) */
static short mouse_record[3]; /* mouse report package (modified) */
static KBDVBASE *KBDVECS_ptr;
static char joystick[3]; /* joystick report package */

/* way to many vectors are stored, modified are only mousevec and joyvec */
static KBDVBASE savedKbdVectors;

/************************************************************
 *** int ask_mouse(void)
 ************************************************************/
int ask_mouse(void) {
static short mouseYspeed = 0;
int joystickMask;
int mouseXdelta;

    joystickMask = 0;
    mouseXdelta = mouse_record[1]; /* X delta */
    mouseYspeed += mouse_record[2]; /* Y delta is summed together to generate a speed, this will keep the player moving, even if the mouse stops */
    mouse_record[1] = mouse_record[2] = 0; /* reset the deltas */

    if(mouseXdelta > 5)
        joystickMask |= JOYSTICK_RIGHT;
    else if(mouseXdelta < -5)
        joystickMask |= JOYSTICK_LEFT;

    /* right mouse button pressed? */
    if(mouse_record[0] == 0xfb || mouse_record[0] == 0xf9)
        mouseYspeed = 0; /* stop Y movement immediately! */

    /* clip the maximum speed */
    if(mouseYspeed > 100)
        mouseYspeed = 100;
    else if(mouseYspeed < -100)
        mouseYspeed = -100;

    if(mouseYspeed > 20)
        joystickMask |= JOYSTICK_DOWN;
    else if(mouseYspeed < -20)
        joystickMask |= JOYSTICK_UP;

    /* the left mouse button is the fire button */
    if(mouse_record[0] == 0xfb || mouse_record[0] == 0xfa)
        joystickMask |= JOYSTICK_BUTTON;

    return joystickMask;
}

/************************************************************
 *** void init_joystick(void)
 ************************************************************/
void init_joystick(void) {
    KBDVECS_ptr = Kbdvbase();
    savedKbdVectors.kb_midivec = KBDVECS_ptr->kb_midivec;
    savedKbdVectors.kb_vkbderr = KBDVECS_ptr->kb_vkbderr;
    savedKbdVectors.kb_vmiderr = KBDVECS_ptr->kb_vmiderr;
    savedKbdVectors.kb_statvec = KBDVECS_ptr->kb_statvec;
    savedKbdVectors.kb_mousevec = KBDVECS_ptr->kb_mousevec;
    savedKbdVectors.kb_clockvec = KBDVECS_ptr->kb_clockvec;
    savedKbdVectors.kb_joyvec = KBDVECS_ptr->kb_joyvec;
    savedKbdVectors.kb_kbdsys = KBDVECS_ptr->kb_kbdsys;
    KBDVECS_ptr->kb_joyvec = (void*)own_joyvec; /* install a new joystick vector */

    Bconout(IKBD, 21); /* SET JOYSTICK INTERROGATION MODE (no automatic reporting, the host has to ask) */

    init_joy_flag = TRUE; /* no button pressed received yet (unused variable) */
    joystick_package_received_flag = FALSE; /* reset the variables  */
}

/************************************************************
 *** void exit_joystick(void)
 ************************************************************/
void exit_joystick(void) {
    Bconout(IKBD, 26); /* DISABLE JOYSTICKS (do not send any joystick events)  */
    KBDVECS_ptr->kb_joyvec = savedKbdVectors.kb_joyvec; /* restore old joystick vector */
    Initmouse(1, mouse_defaults, (void (*)())KBDVECS_ptr->kb_mousevec); /* Enable mouse in relative mode */
}

/************************************************************
 *** int ask_joystick(int index,int *buttonPressed)
 ************************************************************/
int ask_joystick(int index,int *buttonPressed) {
    Bconout(IKBD, 22); /* JOYSTICK INTERROGATE (request a report package) */

    while(!joystick_package_received_flag) {} /* wait for them (this is busy waiting, probably not a good idea) */
    joystick_package_received_flag = FALSE; /* reset the flag */

    if(joystick[1]&(1<<7)) { /* is the button pressed? */
        init_joy_flag = FALSE; /* button is pressed (unused variable)  */
        *buttonPressed = TRUE;
    } else {
        *buttonPressed = FALSE;
    }
    return joystick[index]; /* return the joystick data (only Joystick #1 is supported) */
}

/************************************************************
 *** void own_joyvec(char *buf)
 ************************************************************/
void own_joyvec(char *buf) {
int i;
    /* *buf points to a 3 byte package: */
    /* 0xfd = joystick report header */
    /* %x000yyyy = Joystick 0 */
    /* %x000yyyy = Joystick 1 */
    /* where x is the button and yyyy the 4 directions */

    /* if the last package has not been processed, we ignore newer ones */
    if(joystick_package_received_flag) return;

    for(i = 0; i < 2; i++) /* copy data for both joysticks */
        joystick[i] = buf[i];

    joystick_package_received_flag = TRUE; /* data is valid */
}

/************************************************************
 *** void init_mouse(void)
 ************************************************************/
void init_mouse(void) {
    KBDVECS_ptr = Kbdvbase();
    savedKbdVectors.kb_midivec = KBDVECS_ptr->kb_midivec;
    savedKbdVectors.kb_vkbderr = KBDVECS_ptr->kb_vkbderr;
    savedKbdVectors.kb_vmiderr = KBDVECS_ptr->kb_vmiderr;
    savedKbdVectors.kb_statvec = KBDVECS_ptr->kb_statvec;
    savedKbdVectors.kb_mousevec = KBDVECS_ptr->kb_mousevec;
    savedKbdVectors.kb_clockvec = KBDVECS_ptr->kb_clockvec;
    savedKbdVectors.kb_joyvec = KBDVECS_ptr->kb_joyvec;
    savedKbdVectors.kb_kbdsys = KBDVECS_ptr->kb_kbdsys;
    KBDVECS_ptr->kb_mousevec = (void *)own_mousevec; /* setup our own mouse vector */

    mouse_record[1] = mouse_record[2] = 0; /* reset the variables (BUG: should be done before, because _theoretically_ the interrupt could hit before) */
}

/************************************************************
 *** void exit_mouse(void)
 ************************************************************/
void exit_mouse(void) {
    KBDVECS_ptr->kb_mousevec = savedKbdVectors.kb_mousevec; /* restore old mouse vector */
    Initmouse(1, mouse_defaults, (void (*)())KBDVECS_ptr->kb_mousevec); /* Enable mouse in relative mode */
}

/************************************************************
 *** void own_mousevec(char *buf)
 ************************************************************/
void own_mousevec(char *buf) {
int i;

    /* *buf points to a 3 byte package: */
    /* %111110xy = mouse position record flag, with y the right and x the left button state */
    /* x = delta x as a twos complement integer */
    /* y = delta x as a twos complement integer */

    /* mouse button status in here */
    mouse_record[0] = *buf;
    for(i = 1; i < 3; i++)
        mouse_record[i] += buf[i]; /* add the X/Y relative speeds, so we sum several packages if necessary */
}
