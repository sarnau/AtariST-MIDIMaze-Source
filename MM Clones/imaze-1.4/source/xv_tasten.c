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
** Datei: xv_tasten.c
**
** Kommentar:
**  Wertet Events von der Tastatur aus und setzt definierte Signale
*/


#include <xview/xview.h>
#include <xview/canvas.h>
#include <xview/panel.h>

#include "argv.h"
#include "global.h"
#include "signale.h"
#include "ereignisse.h"
#include "einaus.h"
#include "grafik.h"
#include "xv_einaus.h"
#include "X_tasten.h"

static char sccsid[] = "@(#)xv_tasten.c	3.6 12/3/01";


struct arg_option xv_eingabe_opts[] =
{
	{ Arg_Include, NULL, X_eingabe_opts },
	{ Arg_End }
};


/*
** event_routine
**  verarbeitet die Tastatur-Events und ordnet sie den internen Signalen zu
**
** Parameter:
**  window: Fenster fuer den der Event bestimmt ist
**  event: Beschreibung des Events
**
** Seiteneffekte:
**  aktiv wird veraendert
*/
static void event_routine(Xv_Window window, Event *event)
{
	X_event_routine(XV_DISPLAY_FROM_WINDOW(event_window(event)),
		event_xevent(event));
}


/*
** frame_init
**  sucht in einem Fenster nach Unterfenstern vom Typ CANVAS und
**  initialisiert darin die Event-Abfrage
**
** Parameter:
**  frame: Fenster, in dem gesucht werden soll
*/
static void frame_init(Frame frame)
{
	int i;
	Window subwindow;

	for (i = 1; (subwindow = xv_get(frame,
		FRAME_NTH_SUBWINDOW, i)) != 0; i++)
		if ((Xv_pkg *)xv_get(subwindow, XV_TYPE) == CANVAS)
		{
			/* KBD_DONE: Fenster verliert Fokus
			   WIN_ASCII_EVENT: Tasten werden gedrueckt
			   WIN_UP_EVENT: Taste losgelassen (wird evtl.
			   nicht unterstuetzt) */
			xv_set(canvas_paint_window(subwindow),
				WIN_CONSUME_EVENTS,
					KBD_DONE, WIN_ASCII_EVENTS,
					WIN_UP_EVENTS, 0,
				WIN_EVENT_PROC, event_routine,
				0);
		}
}


/* bis hier lokaler Teil                       */
/***********************************************/
/* ab hier globaler Teil                       */


/*
** xv_eingabe_init
**  sucht nach Unterfenstern vom Typ CANVAS und initialisiert darin
**  die Event-Abfrage
**
** Parameter:
**  hauptframe: Fenster, von dem alle anderen Fenster abstammen
**  panel: Panel, in dem Buttons ergaenzt werden duerfen
**
** Rueckgabewert:
**  0 fuer Erfolg, 1 fuer Fehler
*/
int xv_eingabe_init(Frame hauptframe, Panel panel)
{
	int i;          /* Index */
	Frame subframe; /* ein Fenster */

	/* im Hauptfenster suchen */
	frame_init(hauptframe);

	/* in allen weiteren Fenstern suchen */
	for (i = 1; (subframe = xv_get(hauptframe,
		FRAME_NTH_SUBFRAME, i)) != 0; i++)
		frame_init(subframe);

	return X_eingabe_init();
}


/*
** xv_eingabe_ende
**  Dummy-Routine fuer das Schliessen von Eingabekanaelen etc.
*/
void xv_eingabe_ende(void)
{
	X_eingabe_ende();
}
