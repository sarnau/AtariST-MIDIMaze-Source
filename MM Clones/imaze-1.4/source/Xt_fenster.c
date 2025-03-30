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
** Datei: Xt_fenster.c
**
** Kommentar:
**  Fensterverwaltung auf Basis des X Toolkit,
**  wird fuer Motif und Athena verwendet
*/


#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include "argv.h"
#include "grafik.h"
#include "signale.h"
#include "ereignisse.h"
#include "client.h"
#include "fehler.h"
#include "speicher.h"
#include "X_grafik.h"
#include "X_tasten.h"
#include "X_daten.h"
#include "X_icon.h"
#include "Xt_appdefs.h"
#include "audio.h"
#include "Xt_fenster.h"

static char sccsid[] = "@(#)Xt_fenster.c	3.18 12/05/01";


#define STR_LENGTH 256


struct arg_option einaus_opts[] =
{
	{ Arg_Include, NULL, X_eingabe_opts },
	{ Arg_Include, NULL, audio_opts },
	{ Arg_End }
};


/* application resources */
static struct res_data
{
	char *audio_directory;
} res_data;
static XtResource res_list[] =
{
	{
		XtNsoundDirectory,
		XtCSoundDirectory,
		XtRString,
		sizeof(char *),
		XtOffsetOf(struct res_data, audio_directory),
		XtRString,
		"" /* use DEFAULT_SOUND_DIR in audio.c */
	}
};

static int toplevel_exists = 0;

static struct blickfelddaten frontview_data;
static struct blickfelddaten rearview_data;
static int compass_data;
static struct punktedaten score_data;
static struct grundrissdaten map_data;

static int connected = 0;

static int network_proc_active = 0;
static XtInputId network_proc_id;

static XtInputId audio_select_id;

static int audio_is_active = 0;
static int audio_select_pending = 0;

static void (*audio_signal_handler)(void) = NULL;


static void audio_select_handler(XtPointer function, int *descriptor,
	XtInputId *id)
{
	((void (*)(void))function)();
}


static void handle_audio_signal(int signal)
{
	if (audio_signal_handler != NULL)
		audio_signal_handler();
}


static void popdown_view(struct xt_view *view)
{
	if (!view->is_open)
		return;

	XtPopdown(view->w);
	view->is_open = 0;
}


static void view_redraw(struct xt_view *view)
{
	X_zeichne_sync_anfang(view->window.display);
	view->redraw(view->X_data, &view->window, view->data);
	X_zeichne_sync_ende(view->window.display);
}


static void set_pause_mode(int in_pause)
{
	Arg arglist[1];

	XtSetArg(arglist[0], XtNsensitive, !in_pause);
	XtSetValues(xt_pause_button_w, arglist, 1);

	XtSetArg(arglist[0], XtNsensitive, in_pause);
	XtSetValues(xt_resume_button_w, arglist, 1);
}


static void stop_network_proc(void)
{
	if (!network_proc_active)
		return;

	network_proc_active = 0;
	XtRemoveInput(network_proc_id);

	xt_frontview.was_open = xt_frontview.is_open;
	xt_rearview.was_open = xt_rearview.is_open;
	xt_compassview.was_open = xt_compassview.is_open;
	xt_scoreview.was_open = xt_scoreview.is_open;
	xt_mapview.was_open = xt_mapview.is_open;
}


static void finish_disconnect(void)
{
	Arg arglist[1];

	XtSetArg(arglist[0], XtNsensitive, False);
	XtSetValues(xt_disconnect_button_w, arglist, 1);
	XtSetValues(xt_pause_button_w, arglist, 1);
	XtSetValues(xt_resume_button_w, arglist, 1);

	stop_network_proc();

	popdown_view(&xt_rearview);
	popdown_view(&xt_compassview);
	popdown_view(&xt_scoreview);
	popdown_view(&xt_mapview);

	frontview_data.titelbild = 1;
	if (xt_frontview.is_open)
		view_redraw(&xt_frontview);

	xt_set_status_label("Not connected");
	xt_set_option_sensitivity(True);
}


static void do_disconnect(void)
{
	Arg arglist[1];
	char *server_name;
	char status_msg[STR_LENGTH];

	if (!connected)
		return;

	connected = 0;

	XtSetArg(arglist[0], XtNsensitive, False);
	XtSetValues(xt_disconnect_button_w, arglist, 1);
	XtSetValues(xt_pause_button_w, arglist, 1);
	XtSetValues(xt_resume_button_w, arglist, 1);

	stop_network_proc();

	popdown_view(&xt_rearview);
	popdown_view(&xt_compassview);
	popdown_view(&xt_scoreview);
	popdown_view(&xt_mapview);

	frontview_data.titelbild = 1;
	if (xt_frontview.is_open)
		view_redraw(&xt_frontview);

	server_name = xt_get_string_value(xt_server_field_w);

	strcpy(status_msg, "Disconnecting from ");
	strncat(status_msg, server_name,
		sizeof status_msg - strlen(status_msg));

	xt_set_status_label(status_msg);

	XFlush(XtDisplay(xt_toplevel));

	client_spiel_beenden();

	xt_set_status_label("Not connected");
	xt_set_option_sensitivity(True);
}


static void network_ready_handler(XtPointer data, int *source, XtInputId *id)
{
	if (bewegung(0, 1))
	{
		char **error_message;
		char *button_label;

		milden_fehler_abfragen(&error_message, &button_label);
		ueblen_fehler_anzeigen(error_message, button_label);

		if (connected)
			do_disconnect();
		else
			finish_disconnect();
	}
}


static Boolean connect_work_proc(XtPointer closure)
{
	Arg arglist[1];
	char server_name[STR_LENGTH], message_name[STR_LENGTH];
	char status_msg[STR_LENGTH];
	Boolean camera_mode;
	int *descriptor;

	if (connected)
		return True;

	strncpy(server_name, xt_get_string_value(xt_server_field_w),
		sizeof server_name);
	server_name[sizeof server_name - 1] = 0;

	strncpy(message_name, xt_get_string_value(xt_message_field_w),
		sizeof message_name);
	message_name[sizeof message_name - 1] = 0;

	camera_mode = xt_get_toggle_value(xt_camera_toggle_w);

	if (client_spiel_starten(server_name, message_name, camera_mode))
	{
		char **error_message;
		char *button_label;

		milden_fehler_abfragen(&error_message, &button_label);
		ueblen_fehler_anzeigen(error_message, button_label);

		finish_disconnect();
		return True;
	}

	if ((descriptor = bewegung_deskriptor()) != NULL)
	{
		network_proc_id = XtAppAddInput(xt_app_context, *descriptor,
			(XtPointer)XtInputReadMask, network_ready_handler,
			NULL);
		network_proc_active = 1;
	}

	connected = 1;
	frontview_data.titelbild = 0;

	if (xt_frontview.was_open)
		xt_popup_view(&xt_frontview);

	if (xt_rearview.was_open)
		xt_popup_view(&xt_rearview);

	if (xt_compassview.was_open)
		xt_popup_view(&xt_compassview);

	if (xt_scoreview.was_open)
		xt_popup_view(&xt_scoreview);

	if (xt_mapview.was_open)
		xt_popup_view(&xt_mapview);

	strcpy(status_msg, "Connected to ");
	strncat(status_msg, server_name,
		sizeof status_msg - strlen(status_msg));

	xt_set_status_label(status_msg);

	XtSetArg(arglist[0], XtNsensitive, True);
	XtSetValues(xt_disconnect_button_w, arglist, 1);

	set_pause_mode(0);
	return True;
}


static void destroy_handler(Widget widget, XtPointer client_data,
	XtPointer call_data)
{
	toplevel_exists = 0;

	exit(0);
}


/* bis hier lokaler Teil                       */
/***********************************************/
/* ab hier globaler Teil                       */


void xt_view_init(struct xt_view *view)
{
	Arg arglist[3];
	Dimension width, height;
	unsigned int depth;

	XtSetArg(arglist[0], XtNwidth, &width);
	XtSetArg(arglist[1], XtNheight, &height);
	XtSetArg(arglist[2], XtNdepth, &depth);

	XtGetValues(view->w, arglist, 3);

	view->window.breite = width;
	view->window.hoehe = height;
	view->window.farbtiefe = depth;

	view->init(&view->X_data, &view->window);
}


void xt_view_resize(Widget widget, XtPointer client_data, XtPointer call_data)
{
	struct xt_view *view;

	view = (struct xt_view *)client_data;
	X_zeichne_sync_anfang(view->window.display);
	X_fenster_freigeben(&view->X_data, &view->window);
	xt_view_init(view);
	X_zeichne_sync_ende(view->window.display);
}


void xt_view_redraw(Widget widget, XtPointer client_data, XtPointer call_data)
{
	struct xt_view *view;

	view = (struct xt_view *)client_data;
	X_zeichne_sync_anfang(view->window.display);
	view->redraw(view->X_data, &view->window, view->data);
	X_zeichne_sync_ende(view->window.display);
}


void xt_view_destroy(Widget widget, XtPointer client_data, XtPointer call_data)
{
	struct xt_view *view;

	view = (struct xt_view *)client_data;

	view->is_open = 0;
	view->is_initialized = 0;
}


void xt_view_toggle(Widget widget, XtPointer client_data, XtPointer call_data)
{
	struct xt_view *view;

	view = (struct xt_view *)client_data;

	if (view->is_open)
		popdown_view(view);
	else
		xt_popup_view(view);
}


void xt_disconnect_handler(Widget widget, XtPointer client_data,
	XtPointer call_data)
{
	do_disconnect();
}


void xt_connect_handler(Widget widget, XtPointer client_data,
	XtPointer call_data)
{
	char *server_name;
	char status_msg[STR_LENGTH];

	if (connected)
		return;

	xt_set_option_sensitivity(False);

	server_name = xt_get_string_value(xt_server_field_w);

	strcpy(status_msg, "Connecting to ");
	strncat(status_msg, server_name,
		sizeof status_msg - strlen(status_msg));

	xt_set_status_label(status_msg);

	XtAppAddWorkProc(xt_app_context, connect_work_proc, NULL);
	XSync(XtDisplay(xt_toplevel), False);
}


void xt_pause_handler(Widget widget, XtPointer client_data,
	XtPointer call_data)
{
	eingabe_simulieren(STOPSIGNAL);
}


void xt_resume_handler(Widget widget, XtPointer client_data,
	XtPointer call_data)
{
	eingabe_simulieren(WEITERSIGNAL);
}


void xt_exit_handler(Widget widget, XtPointer client_data, XtPointer call_data)
{
	do_disconnect();

	XtDestroyWidget(xt_toplevel);
}


void zeichne_sync_anfang(void)
{
	X_zeichne_sync_anfang(XtDisplay(xt_toplevel));
}


void zeichne_sync_ende(void)
{
	X_zeichne_sync_ende(XtDisplay(xt_toplevel));
}


void zeichne_blickfeld(struct objektdaten *objekte_neu)
{
	objekte_kopieren(&frontview_data.objekte, objekte_neu);

	if (xt_frontview.is_open)
		X_zeichne_blickfeld(xt_frontview.X_data, &xt_frontview.window,
			&frontview_data);
}


void zeichne_rueckspiegel(struct objektdaten *objekte_rueck_neu)
{
	objekte_kopieren(&rearview_data.objekte, objekte_rueck_neu);

	if (xt_rearview.is_open)
		X_zeichne_blickfeld(xt_rearview.X_data, &xt_rearview.window,
			&rearview_data);
}


void zeichne_karte(int spieler_index, int anzkreise_neu,
	struct kartenkreis *kreise_neu)
{
	map_data.anzkreise_alt = map_data.anzkreise;
	map_data.kreise_alt = map_data.kreise;
	liste_kopieren(&map_data.anzkreise, (void **)&map_data.kreise,
		&anzkreise_neu, (void **)&kreise_neu,
		sizeof(struct kartenkreis));

	if (xt_mapview.is_open)
		X_zeichne_karte(xt_mapview.X_data, &xt_mapview.window,
			&map_data);

	liste_freigeben(&map_data.anzkreise_alt,
		(void **)&map_data.kreise_alt);
	map_data.anzkreise_alt = 0;
	map_data.kreise_alt = NULL;
}


void zeichne_grundriss(int anzlinien_neu, struct kartenlinie *linien_neu)
{
	liste_freigeben(&map_data.anzlinien, (void **)&map_data.linien);
	liste_kopieren(&map_data.anzlinien, (void **)&map_data.linien,
		&anzlinien_neu, (void **)&linien_neu,
		sizeof(struct kartenlinie));

	if (xt_mapview.is_open)
		X_zeichne_grundriss(xt_mapview.X_data, &xt_mapview.window,
			&map_data);
}


void zeichne_kompass(int blickrichtung_neu)
{
	compass_data = blickrichtung_neu;

	if (xt_compassview.is_open)
		X_zeichne_kompass(xt_compassview.X_data,
			&xt_compassview.window, &compass_data);
}


void zeichne_punkte(struct punktedaten *punkte_neu)
{
	liste_freigeben(&score_data.anzpunkte, (void **)&score_data.punkte);
	liste_kopieren(&score_data.anzpunkte, (void **)&score_data.punkte,
		&punkte_neu->anzpunkte, (void **)&punkte_neu->punkte,
		sizeof(struct punktestand));

	if (xt_scoreview.is_open)
		X_zeichne_punkte(xt_scoreview.X_data, &xt_scoreview.window,
			&score_data);
}


void grafik_schleife(void)
{
	connected = 0;

	XtAppMainLoop(xt_app_context);
}


int grafik_init(int *argc, char **argv)
{
	Colormap colormap;
	Arg arglist[1];

	audio_is_active = 0;

	xt_toplevel = XtAppInitialize(&xt_app_context, "IMaze",
		NULL, 0, argc, argv, fallback_app_resources, NULL, 0);
	XtAddCallback(xt_toplevel, XtNdestroyCallback, destroy_handler, NULL);

	toplevel_exists = 1;

	XtGetApplicationResources(xt_toplevel, (XtPointer)&res_data,
		res_list, XtNumber(res_list), NULL, 0);

	xt_fill_main(xt_toplevel);
	XtRealizeWidget(xt_toplevel);

	XtSetArg(xt_icon_arglist[0], XtNiconPixmap,
		X_create_icon_pixmap(XtDisplay(xt_toplevel),
		XScreenNumberOfScreen(XtScreen(xt_toplevel))));

	XtSetValues(xt_toplevel, xt_icon_arglist, 1);

	if (X_farben_init(XtDisplay(xt_toplevel),
		XScreenNumberOfScreen(XtScreen(xt_toplevel)), &colormap))
	{
		XtRemoveCallback(xt_toplevel, XtNdestroyCallback,
			destroy_handler, NULL);
		XtDestroyWidget(xt_toplevel);

		toplevel_exists = 0;

		return 1;
	}

	XtSetArg(arglist[0], XtNcolormap, colormap);
	XtSetValues(xt_toplevel, arglist, 1);

	xt_frontview.name = "frontView";
	xt_frontview.data = &frontview_data;
	xt_frontview.is_open = 0;
	xt_frontview.is_initialized = 0;
	xt_frontview.was_open = 1;
	xt_frontview.init = X_fenster_init;
	xt_frontview.redraw = X_zeichne_blickfeld;
	objekt_listen_init(&frontview_data.objekte);
	frontview_data.fadenkreuz = 1;
	frontview_data.titelbild = 1;

	xt_scoreview.name = "score";
	xt_scoreview.data = &score_data;
	xt_scoreview.is_open = 0;
	xt_scoreview.is_initialized = 0;
	xt_scoreview.was_open = 1;
	xt_scoreview.init = X_fenster_init;
	xt_scoreview.redraw = X_zeichne_punkte;
	liste_initialisieren(&score_data.anzpunkte,
		(void **)&score_data.punkte);

	xt_compassview.name = "compass";
	xt_compassview.data = &compass_data;
	xt_compassview.is_open = 0;
	xt_compassview.is_initialized = 0;
	xt_compassview.was_open = 0;
	xt_compassview.init = X_fenster_init;
	xt_compassview.redraw = X_zeichne_kompass;
	compass_data = -1;

	xt_rearview.name = "rearView";
	xt_rearview.data = &rearview_data;
	xt_rearview.is_open = 0;
	xt_rearview.is_initialized = 0;
	xt_rearview.was_open = 0;
	xt_rearview.init = X_fenster_init;
	xt_rearview.redraw = X_zeichne_blickfeld;
	objekt_listen_init(&rearview_data.objekte);
	rearview_data.fadenkreuz = 0;
	rearview_data.titelbild = 0;

	xt_mapview.name = "map";
	xt_mapview.data = &map_data;
	xt_mapview.is_open = 0;
	xt_mapview.is_initialized = 0;
	xt_mapview.was_open = 0;
	xt_mapview.init = X_karte_init;
	xt_mapview.redraw = X_zeichne_grundriss;
	liste_initialisieren(&map_data.anzlinien, (void **)&map_data.linien);
	liste_initialisieren(&map_data.anzkreise, (void **)&map_data.kreise);
	map_data.anzkreise_alt = 0;
	map_data.kreise_alt = NULL;

	if (X_eingabe_init())
		return 1;

	xt_popup_view(&xt_frontview);

	switch (audio_start(res_data.audio_directory))
	{
	case Audio_Started:
		audio_is_active = 1;
		return 0;

	case Audio_Failed:
		return 0;

	case Audio_Abort:
	default:
		return 1;
	}
}


void grafik_ende(void)
{
	if (toplevel_exists)
		XtDestroyWidget(xt_toplevel);

	X_eingabe_ende();

	if (!audio_is_active)
		return;

	audio_close();
	audio_is_active = 0;
}


void ueblen_fehler_anzeigen(char **meldung, char *knopf)
{
	if (!toplevel_exists)
	{
		while (*meldung != NULL)
			fprintf(stderr, "iMaze: %s\n", *meldung++);

		return;
	}

	if (connected)
	{
		connected = 0;
		stop_network_proc();
	}

	xt_request_window(meldung, knopf, NULL, 0, "error");
}


void ereignisse_darstellen(char ereignisse[ERG_ANZ])
{
	if (ereignisse[PAUSE_ERG])
		set_pause_mode(1);
	else if (ereignisse[ERWACHT_ERG])
		set_pause_mode(0);

	if (audio_is_active)
	{
		audio_play(ereignisse);
		return;
	}

	if (ereignisse[ABGESCHOSSEN_ERG] || ereignisse[GETROFFEN_ERG] ||
		ereignisse[AKTION_FEHLER_ERG] || ereignisse[SUSPENDIERT_ERG])
		XBell(XtDisplay(xt_toplevel), 0);
}


int rueckfrage(char **meldung, char *weiter_knopf, char *abbruch_knopf)
{
	if (!toplevel_exists)
	{
		static char *message[] = { "iMaze - Fatal Error", "",
			"Couldn't open notice window.", NULL };

		uebler_fehler(message, NULL);
	}

	return xt_request_window(meldung, weiter_knopf,
		abbruch_knopf, 1, "question") == 1;
}


void disconnect_abfrage(void)
{
	static char *message[] = { "Do you really want do disconnect?", NULL };

	if (!rueckfrage(message, "Sure", "Play on"))
		return;

	do_disconnect();
}


void audio_select_descriptor(int descriptor, void (*function)(void),
	int for_output)
{
	if (audio_select_pending)
	{
		XtRemoveInput(audio_select_id);
		audio_select_pending = 0;
	}

	if (function != NULL)
	{
		audio_select_id = XtAppAddInput(xt_app_context, descriptor,
			for_output ? (XtPointer)XtInputWriteMask :
				(XtPointer)XtInputReadMask,
			audio_select_handler, (XtPointer)function);
		audio_select_pending = 1;
	}
}


void audio_handle_signal(int signal, void (*function)(void))
{
	audio_signal_handler = function;

	if (audio_signal_handler == NULL)
		handle_signal(signal, SIG_DFL);
	else
		handle_signal(signal, handle_audio_signal);
}
