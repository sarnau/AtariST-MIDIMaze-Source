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
** Datei: xv_ereignisse.c
**
** Kommentar:
**  Gibt bei Auftreten von Ereignissen audio-visuelle Signale aus
*/


#include <string.h>
#include <xview/xview.h>
#include <xview/canvas.h>
#include <xview/panel.h>
#include <xview/defaults.h>
#include <xview/notify.h>

#include "argv.h"
#include "system.h"
#include "signale.h"
#include "ereignisse.h"
#include "einaus.h"
#include "xv_einaus.h"
#include "audio.h"

static char sccsid[] = "@(#)xv_ereignisse.c	3.22 12/05/01";


struct arg_option xv_ereignisse_opts[] =
{
	{ Arg_Include, NULL, audio_opts },
	{ Arg_End }
};


static int audio_laeuft;  /* Audio aktiviert? */

static Frame audio_frame; /* Fenster fuer audio_select_descriptor */

static void (*pause_callback_handler)(int);
static void (*audio_callback_handler)(void);
static void (*audio_signal_handler)(void);
static Window flash_window, beep_window; /* Fenster, in dem Signale
                                            ausgegeben werden sollen */


static Notify_value handle_audio_callback(void)
{
	if (audio_callback_handler != NULL)
		audio_callback_handler();

	return NOTIFY_DONE;
}


static Notify_value handle_audio_signal(void)
{
	if (audio_signal_handler != NULL)
		audio_signal_handler();

	return NOTIFY_DONE;
}


/* bis hier lokaler Teil                       */
/***********************************************/
/* ab hier globaler Teil                       */


/*
** ereignisse_darstellen
**  fuehrt die Ausgabe der Signale aus
**
** Parameter:
**  ereignisse: Feld aller Ereignisse, in dem die auszugebenen gesetzt sind
**
** Seiteneffekte:
**  abspiel und abspiel_anz werden veraendert
*/
void ereignisse_darstellen(char ereignisse[ERG_ANZ])
{
	if (ereignisse[PAUSE_ERG] && pause_callback_handler != NULL)
		pause_callback_handler(1);
	else if (ereignisse[ERWACHT_ERG] && pause_callback_handler != NULL)
		pause_callback_handler(0);

	/* Audio spielen? */
	if (audio_laeuft)
	{
		audio_play(ereignisse);

		/* nicht Blinken/Piepen ausloesen */
		return;
	}

	/* Falls Spieler selbst abgeschossen wurde, wird das Fenster kurz
	   invertiert und ein Piep ausgegeben*/
	if (ereignisse[ABGESCHOSSEN_ERG])
		xv_set(flash_window,
			WIN_ALARM,
			0);

	/* Falls Spieler selbst getroffen wurde, wird ein Piep ausgegeben */
	else if (ereignisse[GETROFFEN_ERG])
		xv_set(beep_window,
			WIN_ALARM,
			0);

	/* Falls eine Aktion nicht moeglich war, wird ein Piep ausgegeben */
	else if (ereignisse[AKTION_FEHLER_ERG])
		xv_set(beep_window,
			WIN_ALARM,
			0);

	/* Falls Spieler vom Server aus dem Spiel genommen wurde, wird
	   ein Piep ausgegeben */
	else if (ereignisse[SUSPENDIERT_ERG])
		xv_set(beep_window,
			WIN_ALARM,
			0);
}


/*
** xv_ereignisse_init
**  sucht nach XView-Objekten, in denen die Signale ausgegeben werden
**
** Parameter:
**  hauptframe: Fenster, von dem alle anderen Fenster abstammen
**  panel: Panel, in dem Buttons ergaenzt werden duerfen
**
** Rueckgabewert:
**  0 fuer Erfolg, 1 fuer Fehler
**
** Seiteneffekte:
**  beep_window und flash_window werden gesetzt
*/
int xv_ereignisse_init(void (*pause_callback)(int), Frame hauptframe,
	Panel panel)
{
	int i;            /* Index fuer Unterfenster */
	Window subwindow; /* Unterfenster */
	static char audio_verzeichnis[64]; /* Verzeichnis, in dem die
	                                      Sound-Dateien liegen */

	pause_callback_handler = pause_callback;

	/* Pieps werden im Hauptfenster ausgeloest */
	beep_window = hauptframe;

	/* Suche nach einem Unterfenster vom Typ CANVAS */
	for (i = 1; (subwindow = xv_get(hauptframe,
		FRAME_NTH_SUBWINDOW, i)) != 0; i++)
		if ((Xv_pkg *)xv_get(subwindow, XV_TYPE) == CANVAS)
		{
			/* Das canvas_paint_window wird zum Invertieren
			   genommen */
			flash_window = canvas_paint_window(subwindow);
			goto gefunden;
		}

	/* falls kein CANVAS gefunden */
	flash_window = beep_window;

gefunden:
	;

	/* Fenster fuer audio_select_descriptor merken */
	audio_frame = hauptframe;

	/* Parameter initialisieren */
	audio_laeuft = 0;

	/* Name des Audio-Verzeichnisses abfragen und kopieren;
	   "" = DEFAULT_SOUND_DIR in audio.c */
	strncpy(audio_verzeichnis,
		defaults_get_string("iMaze.sound.directory",
		"IMaze.Sound.Directory", ""),
		sizeof audio_verzeichnis);
	audio_verzeichnis[sizeof audio_verzeichnis - 1] = 0;

	/* Audio-Device oeffnen */
	switch (audio_start(audio_verzeichnis))
	{
	case Audio_Started:
		audio_laeuft = 1;
		return 0;

	case Audio_Failed:
		audio_laeuft = 0;
		return 0;

	case Audio_Abort:
	default:
		audio_laeuft = 0;
		return 1;
	}
}


/*
** xv_ereignisse_ende
**  Routine fuer das Schliessen von Ausgabekanaelen etc.
*/
void xv_ereignisse_ende(void)
{
	pause_callback_handler = NULL;

	/* laeuft Audio? */
	if (!audio_laeuft)
		return;

	audio_close();

	/* Audio laeuft nicht mehr */
	audio_laeuft = 0;
}


void audio_select_descriptor(int descriptor, void (*function)(void),
	int for_output)
{
	Notify_func handler;

	audio_callback_handler = function;
	if (function == NULL)
		handler = (Notify_func)NULL;
	else
		handler = (Notify_func)handle_audio_callback;

	if (for_output)
		notify_set_output_func(audio_frame, handler, descriptor);
	else
		notify_set_input_func(audio_frame, handler, descriptor);
}


void audio_handle_signal(int signal, void (*function)(void))
{
	Notify_func handler;

	audio_signal_handler = function;
	if (function == NULL)
		handler = (Notify_func)NULL;
	else
		handler = (Notify_func)handle_audio_signal;

	notify_set_signal_func(audio_frame, handler, signal, NOTIFY_ASYNC);
}
