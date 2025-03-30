/*
** - - -  iMaze  - - -
**
** Copyright (c) 1993-2001 by Hans-Ulrich Kiel & Joerg Czeranski
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**
** 1. Redistributions of source code must retain the above copyright
**    notice, this list of conditions and the following disclaimer.
** 2. Redistributions in binary form must reproduce the above copyright
**    notice, this list of conditions and the following disclaimer in the
**    documentation and/or other materials provided with the distribution.
** 3. The name of the authors may not be used to endorse or promote
**    products derived from this software without specific prior written
**    permission.
** 4. The name ``iMaze'' may not be used for products derived from this
**    software unless a prefix or a suffix is added to the name.
**
** THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
** IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY DIRECT,
** INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
** SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
** HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
** STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
** IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
**
**
** Datei: X_tasten.c
**
** Kommentar:
**  Wertet Events von der Tastatur aus und setzt definierte Signale
*/


#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>

#include "argv.h"
#include "global.h"
#include "system.h"
#include "signale.h"
#include "ereignisse.h"
#include "einaus.h"
#include "grafik.h"
#include "X_tasten.h"
#include "joystick.h"

static char sccsid[] = "@(#)X_tasten.c	3.8 12/3/01";


#define GEDRUECKT 1       /* Bit 0 wird gesetzt, wenn Taste gedrueckt;
                             und geloescht, wenn diese losgelassen */
#define NICHT_ABGEFRAGT 2 /* Bit 1 bleibt gesetzt, bis ein Tastendruck
                             registriert wurde */

/* die Cursor-Tasten haben eine Repeat-Funktion, alle anderen nicht */
#define HAT_REPEAT(signalnr) \
	((signalnr) == VORSIGNAL || (signalnr) == ZURUECKSIGNAL || \
	(signalnr) == LINKSSIGNAL || (signalnr) == RECHTSSIGNAL)


struct arg_option X_eingabe_opts[] =
{
	{ Arg_Include, NULL, joystick_opts },
	{ Arg_End }
};


static char tasten_aktiv[SIGNALANZ]; /* aktueller Tastenzustand
                                        (GEDRUECKT / NICHT_ABGEFRAGT) */
static char joystick_aktiv[SIGNALANZ]; /* dito fuer Joystick */


/*
** aktiv_setzen
**  setzt oder loescht das "gedrueckt"-Flag fuer ein Signal
**
** Parameter:
**  aktiv: Feld mit Flags fuer die Signale
**  signalnr: Nummer des Signals
**  gedrueckt: 1, falls Taste gedrueckt wurde; 0, falls losgelassen
**
** Seiteneffekte:
**  aktiv[signalnr] wird veraendert
*/
static void aktiv_setzen(char aktiv[SIGNALANZ], int signalnr, int gedrueckt)
{
	/* signalnr ungueltig? */
	if (signalnr < 0 || signalnr >= SIGNALANZ)
		return;

	/* Taste wurde losgelassen */
	if (!gedrueckt)
		aktiv[signalnr] &= ~GEDRUECKT;
	/* Taste ist niedergedrueckt (neu oder durch repeat);
	   falls Taste nicht zuvor schon gedrueckt war, als
	   gedrueckt und nicht abgefragt markieren */
	else if (!(aktiv[signalnr] & GEDRUECKT))
		aktiv[signalnr] = GEDRUECKT | NICHT_ABGEFRAGT;
}


/* bis hier lokaler Teil                       */
/***********************************************/
/* ab hier globaler Teil                       */


/*
** X_event_routine
**  verarbeitet die Tastatur-Events und ordnet sie den internen Signalen zu
**
** Parameter:
**  event: Beschreibung des Events
**
** Seiteneffekte:
**  aktiv wird veraendert
*/
void X_event_routine(Display *display, XEvent *event)
{
	int signalnr;  /* Index auf Tabelle mit Signalen */
	KeySym action; /* Typ bzw. ASCII-Code des Events */

	switch (event->type)
	{
		case FocusOut: /* Fenster verliert Fokus (erhaelt keine
		                  Tastatur-Eingaben mehr) */
			/* dann werden alle Tasten wie losgelassen behandelt */
			for (signalnr = 0; signalnr < SIGNALANZ; signalnr++)
				tasten_aktiv[signalnr] &= ~GEDRUECKT;
			return;
		case KeyPress:
		case KeyRelease:
			break;
		default:
			return;
	}

	switch (action = XKeycodeToKeysym(display, event->xkey.keycode, 0))
	{
		case XK_c: /* Control-C */
			if (event->type == KeyPress &&
				!(event->xkey.state & ControlMask))
				return;
			if (event->type == KeyPress)
				disconnect_abfrage();
			return;
		case XK_q: /* Control-Q */
			if (event->type == KeyPress &&
				!(event->xkey.state & ControlMask))
				return;
			signalnr = WEITERSIGNAL;
			break;
		case XK_s: /* Control-S */
			if (event->type == KeyPress &&
				!(event->xkey.state & ControlMask))
				return;
			signalnr = STOPSIGNAL;
			break;
		case XK_space: /* Leertaste */
		case XK_Shift_L:
		case XK_Shift_R:
		case XK_Alt_L:
		case XK_Alt_R:
			signalnr = SCHUSSSIGNAL;
			break;
		case XK_Up: /* Cursor hoch */
			signalnr = VORSIGNAL;
			break;
		case XK_Down: /* Cursor runter */
		case XK_KP_5: /* 5 auf dem Ziffernblock */
			signalnr = ZURUECKSIGNAL;
			break;
		case XK_Left: /* Cursor links */
			signalnr = LINKSSIGNAL;
			break;
		case XK_Right: /* Cursor rechts */
			signalnr = RECHTSSIGNAL;
			break;
		case XK_Tab:
			signalnr = QUICKTURNSIGNAL;
			break;
		case XK_0: /* Tasten 0 bis 9 */
			signalnr = AKTIONSSIGNALE + 9;
			break;
		case XK_1:
		case XK_2:
		case XK_3:
		case XK_4:
		case XK_5:
		case XK_6:
		case XK_7:
		case XK_8:
		case XK_9:
			signalnr = AKTIONSSIGNALE + action - XK_1;
			break;
		default:
			return;
	}

	/* Tastenflags setzen */
	aktiv_setzen(tasten_aktiv, signalnr, event->type == KeyPress);
}


void eingabe_simulieren(int signalnr)
{
	/* signalnr ungueltig? */
	if (signalnr < 0 || signalnr >= SIGNALANZ)
		return;

	tasten_aktiv[signalnr] |= NICHT_ABGEFRAGT;
}


void joystick_event(int signal_no, int active)
{
	aktiv_setzen(joystick_aktiv, signal_no, active);
}


/*
** eingabe_abfragen
**  setzt in einem Feld aller Signale diejenigen, die seit der letzten
**  Abfrage aufgetreten sind. Die Tasten werden als abgefragt markiert
**
** Paramter:
**  signale: Feld mit allen Signalen
**           (0 = nicht aufgetreten; 1 = aufgetreten)
**
** Seiteneffekte:
**  aktiv und signale wird veraendert
*/
void eingabe_abfragen(char signale[SIGNALANZ])
{
	int signalnr; /* Index auf Tabelle mit Signalen */

	joystick_trigger();

	for (signalnr = 0; signalnr < SIGNALANZ; signalnr++)
	{
		/* Tastatur */

		/* falls REPEAT-Funktion vorgesehen (Cursor-Tasten),
		   Signal setzen, wenn GEDRUECKT oder NICHT_ABGEFRAGT,
		   sonst nur wenn NICHT_ABGEFRAGT */
		if (HAT_REPEAT(signalnr))
			signale[signalnr] = tasten_aktiv[signalnr] != 0;
		else
			signale[signalnr] = (tasten_aktiv[signalnr] &
				NICHT_ABGEFRAGT) != 0;

		/* alle Tasten als ABGEFRAGT markieren */
		tasten_aktiv[signalnr] &= ~NICHT_ABGEFRAGT;


		/* Joystick */

		/* falls REPEAT-Funktion vorgesehen (Joystick-Bewegung),
		   Signal setzen, wenn GEDRUECKT oder NICHT_ABGEFRAGT,
		   sonst nur wenn NICHT_ABGEFRAGT */
		if (HAT_REPEAT(signalnr))
			signale[signalnr] |= joystick_aktiv[signalnr] != 0;
		else
			signale[signalnr] |= (joystick_aktiv[signalnr] &
				NICHT_ABGEFRAGT) != 0;

		/* alle Tasten als ABGEFRAGT markieren */
		joystick_aktiv[signalnr] &= ~NICHT_ABGEFRAGT;
	}
}


/*
** X_eingabe_init
**  initialisiert die Event-Abfrage
**
** Rueckgabewert:
**  0 fuer Erfolg, 1 fuer Fehler
*/
int X_eingabe_init(void)
{
	if (joystick_init())
		return 1;

	return 0; /* Erfolg */
}


/*
** X_eingabe_ende
**  Routine fuer das Schliessen von Eingabekanaelen etc.
*/
void X_eingabe_ende(void)
{
	joystick_close();
}
