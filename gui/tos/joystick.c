/************************************************************
 *** Joystick/Mouse code
 ************************************************************/
#include "globals.h"

#define MOUSE_BUTTON_BOTH  -5 /* 0xfb */
#define MOUSE_BUTTON_RIGHT -7 /* 0xf9 */
#define MOUSE_BUTTON_LEFT  -6 /* 0xfa */

static const char mouse_defaults[4] = { 0x00, 0x00, 0x01, 0x01 };

static volatile short joystick_package_received_flag;
static volatile short mouse_record[3];	/* mouse report package (modified) */
static KBDVBASE *KBDVECS_ptr;
static unsigned char joystick[2];		/* joystick status */
static short mouseYspeed = 0;

#ifdef __GNUC__
#define ASM_NAME(x) __asm__(x)
#else
#define ASM_NAME(x)
#endif

/* way to many vectors are stored, used are only mousevec and joyvec */
KBDVBASE savedKbdVectors ASM_NAME("savedKbdVectors");
IOREC *kbdiorec ASM_NAME("kbdiorec");
void *kbdvec ASM_NAME("kbdvec");
static int vectors_saved;

extern void ikbd_install(void) ASM_NAME("ikbd_install");
extern void ikbd_uninstall(void) ASM_NAME("ikbd_uninstall");
extern signed char keyboard_state_buffer[128] ASM_NAME("keyboard_state_buffer");
void own_mousevec(signed char *buf) ASM_NAME("own_mousevec");
void own_joyvec(unsigned char *buf) ASM_NAME("own_joyvec");


/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

static void save_vectors(void)
{
	if (vectors_saved)
		return;
	KBDVECS_ptr = Kbdvbase();
	savedKbdVectors.kb_midivec = KBDVECS_ptr->kb_midivec;
	savedKbdVectors.kb_vkbderr = KBDVECS_ptr->kb_vkbderr;
	savedKbdVectors.kb_vmiderr = KBDVECS_ptr->kb_vmiderr;
	savedKbdVectors.kb_statvec = KBDVECS_ptr->kb_statvec;
	savedKbdVectors.kb_mousevec = KBDVECS_ptr->kb_mousevec;
	savedKbdVectors.kb_clockvec = KBDVECS_ptr->kb_clockvec;
	savedKbdVectors.kb_joyvec = KBDVECS_ptr->kb_joyvec;
	savedKbdVectors.kb_kbdsys = KBDVECS_ptr->kb_kbdsys;
	kbdiorec = Iorec(1);
	/*
	 * the kbdvec (handler for keyboard bytes) is located
	 * just before the KBDVECS structure
	 */
	kbdvec = ((void **)KBDVECS_ptr)[-1];
	/*
	 * For old TOS versions however, we must search for it
	 */
	{
		unsigned short *int_acia = Setexc(0x118 / 4, (void *)-1);
		int i;
		
		for (i = 0; i < 300; i++)
		{
			if (int_acia[0] == 0x0c00 && int_acia[1] == 0x002a)
			{
				kbdvec = int_acia - 2;
				break;
			}
			int_acia++;
		}
	}
	/*
	 * now install our own handler
	 */
	ikbd_install();

	vectors_saved = TRUE;
}

/*** ---------------------------------------------------------------------- ***/

static void restore_vectors(void)
{
	if (!vectors_saved)
		return;
	ikbd_uninstall();
	vectors_saved = FALSE;
}

/*** ---------------------------------------------------------------------- ***/

/*
 * called from our handler.
 * Compatible to KBDVBASE.kb_joyvec
 */
void own_joyvec(unsigned char *buf)
{
	/* *buf points to a 3 byte package: */
	/* 0xfd = joystick report header */
	/* %x000yyyy = Joystick 0 */
	/* %x000yyyy = Joystick 1 */
	/* where x is the button and yyyy the 4 directions */

	/*
	 * copy data for both joysticks
	 * If data was from interrogate (0xfd packet)
	 * the buffer contains just the data.
	 * Otherwise, it will include the packet header
	 */
	if (buf[0] == 0xfe || buf[0] == 0xff)
	{
		joystick[0] = buf[1];
		joystick[1] = buf[2];
	} else
	{
		joystick[0] = buf[0];
		joystick[1] = buf[1];
	}

	joystick_package_received_flag = TRUE;	/* data is valid */
}

/*** ---------------------------------------------------------------------- ***/

void init_joystick(void)
{
	/* reset the variables */
	joystick[0] = joystick[1] = 0;
	joystick_package_received_flag = FALSE;
	/* install a new joystick vector */
	save_vectors();
	Bconout(IKBD, 21);					/* SET JOYSTICK INTERROGATION MODE (no automatic reporting, the host has to ask) */
	Bconout(IKBD, 22);					/* JOYSTICK INTERROGATE (request a report package) */
}

/*** ---------------------------------------------------------------------- ***/

void exit_joystick(void)
{
	Bconout(IKBD, 26);					/* DISABLE JOYSTICKS (do not send any joystick events)  */
	/* restore old acia vector */
	restore_vectors();
	Initmouse(1, mouse_defaults, (void (*)(void)) KBDVECS_ptr->kb_mousevec);	/* Enable mouse in relative mode */
}

/*** ---------------------------------------------------------------------- ***/

int ask_joystick(void)
{
	int packet;

	packet = joystick[1] & 0x0f;		/* return the joystick data (only Joystick #1 is supported) */
	if (joystick[1] & (1 << 7))
	{									/* is the button pressed? */
		packet |= JOYSTICK_BUTTON;
	}

	if (joystick_package_received_flag)
	{
		joystick_package_received_flag = FALSE;	/* reset the flag */
		Bconout(IKBD, 22);					/* JOYSTICK INTERROGATE (request a report package) */
	}
	return packet;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

/*
 * called from our handler.
 * Compatible to KBDVBASE.kb_mousevec
 */
void own_mousevec(signed char *buf)
{
	/* *buf points to a 3 byte package: */
	/* %111110xy = mouse position record flag, with y the right and x the left button state */
	/* x = delta x as a twos complement integer */
	/* y = delta x as a twos complement integer */

	/* mouse button status in here */
	mouse_record[0] = *buf++; /* NOTE: char expected to be signed here */
	mouse_record[1] += *buf++;	/* add the X/Y relative speeds, so we sum several packages if necessary */
	mouse_record[2] += *buf++;
}

/*** ---------------------------------------------------------------------- ***/

void init_mouse(void)
{
	/* reset the variables */
	mouse_record[0] = mouse_record[1] = mouse_record[2] = 0;
	mouseYspeed = 0;
	/* setup our own mouse vector */
	save_vectors();
}

/*** ---------------------------------------------------------------------- ***/

void exit_mouse(void)
{
	/* restore old acia vector */
	restore_vectors();
	Initmouse(1, mouse_defaults, (void (*)(void)) KBDVECS_ptr->kb_mousevec);	/* Enable mouse in relative mode */
}

/*** ---------------------------------------------------------------------- ***/

int ask_mouse(void)
{
	int mouseXdelta;
	int joystickMask;

	joystickMask = 0;
	mouseXdelta = mouse_record[1];		/* X delta */
	mouseYspeed += mouse_record[2];		/* Y delta is summed together to generate a speed, this will keep the player moving, even if the mouse stops */
	mouse_record[1] = mouse_record[2] = 0;	/* reset the deltas */

	if (mouseXdelta > 5)
		joystickMask |= JOYSTICK_RIGHT;
	else if (mouseXdelta < -5)
		joystickMask |= JOYSTICK_LEFT;

	/* right mouse button pressed? */
	if (mouse_record[0] == MOUSE_BUTTON_BOTH || mouse_record[0] == MOUSE_BUTTON_RIGHT)
		mouseYspeed = 0;				/* stop Y movement immediately! */

	/* clip the maximum speed */
	if (mouseYspeed > 100)
		mouseYspeed = 100;
	else if (mouseYspeed < -100)
		mouseYspeed = -100;

	if (mouseYspeed > 20)
		joystickMask |= JOYSTICK_DOWN;
	else if (mouseYspeed < -20)
		joystickMask |= JOYSTICK_UP;

	/* the left mouse button is the fire button */
	if (mouse_record[0] == MOUSE_BUTTON_BOTH || mouse_record[0] == MOUSE_BUTTON_LEFT)
		joystickMask |= JOYSTICK_BUTTON;

	return joystickMask;
}

/******************************************************************************/
/*** ---------------------------------------------------------------------- ***/
/******************************************************************************/

void init_keyboard(void)
{
	int i;
	
	/* reset the variables */
	for (i = 0; i < 128; i++)
		keyboard_state_buffer[i] = 0x80;
	/* setup our own keyboard vector */
	save_vectors();
	/*
	 * No keyboard handler is directly called.
	 * Instead, the state of the keyboard is maintained in keyboard_state_buffer
	 */
}

/*** ---------------------------------------------------------------------- ***/

void exit_keyboard(void)
{
	/* restore old acia vector */
	restore_vectors();
}

/*** ---------------------------------------------------------------------- ***/

int ask_keyboard(void)
{
	int packet = 0;
	
	if (keyboard_state_buffer[0x48] > 0) /* cursor up */
		packet |= JOYSTICK_UP;
	if (keyboard_state_buffer[0x50] > 0) /* cursor down */
		packet |= JOYSTICK_DOWN;
	if (keyboard_state_buffer[0x4b] > 0) /* cursor left */
		packet |= JOYSTICK_LEFT;
	if (keyboard_state_buffer[0x4d] > 0) /* cursor right */
		packet |= JOYSTICK_RIGHT;
	if (keyboard_state_buffer[0x38] > 0) /* Alt */
		packet |= JOYSTICK_BUTTON;
	
	return packet;
}
