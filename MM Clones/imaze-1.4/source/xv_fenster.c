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
** Datei: xv_fenster.c
**
** Kommentar:
**  Fensterverwaltung auf Basis von XView
*/


#include <stdio.h>
#include <xview/xview.h>
#include <xview/icon.h>
#include <xview/panel.h>
#include <xview/notice.h>
#include <xview/canvas.h>
#include <xview/xv_xrect.h>
#include <xview/notify.h>
#include <xview/defaults.h>
#include <xview/attr.h>
#include <xview/scrollbar.h>

#include "argv.h"
#include "speicher.h"
#include "grafik.h"
#include "signale.h"
#include "fehler.h"
#include "client.h"
#include "system.h"
#include "X_grafik.h"
#include "X_daten.h"
#include "X_tasten.h"
#include "xv_einaus.h"

static char sccsid[] = "@(#)xv_fenster.c	3.17 12/08/01";


/* Laenge eines Eingabefeldes fuer Text */
#define EINGABE_TEXT_LAENGE 64

/* Anzahl der Frames in frame_feld (siehe unten) */
#define FRAME_ANZ (sizeof frame_feld / sizeof *frame_feld)


/* die schwarz-weiss-Daten des Icons: */
static unsigned short icon_daten[] = {
#include "xv_icon.h"
};

/* die Maske des Icons: */
static unsigned short icon_maske_daten[] = {
#include "xv_icon_maske.h"
};


struct arg_option einaus_opts[] =
{
	{ Arg_Include, NULL, xv_eingabe_opts },
	{ Arg_Include, NULL, xv_ereignisse_opts },
	{ Arg_End }
};


/* Struktur enthaelt alle benoetigten Informationen ueber ein Fenster
   und Zeiger auf Funktionen, die beim Neuaufbau und Veraendern der
   Groesse auszufuehren sind. Diese Struktur wird dann an ein Canvas
   angehaengt */
struct fenster_beschreibung
{
	struct fenster fenster;  /* Fensterinformation, die von xview an X11
	                            uebergeben werden muessen */
	void *daten;             /* Zeiger auf eine Kopien der im Fenster zu
	                            zu zeichnenden Grafikobjekte */
	void *X_daten;           /* private Daten des X11-Teil */
	/* Zeiger auf Funktionen, die neu zeichnen,
	   X_daten setzen bzw. X_daten freigeben */
	void (*X_zeichne)(void *X_daten, struct fenster *fenster, void *daten);
	void (*X_init)(void **X_daten, struct fenster *fenster);
	void (*X_freigeben)(void **X_daten, struct fenster *fenster);
};

/* Strukturen mit den Informationen zu den einzelnen Fenstern */
static struct fenster_beschreibung blickfeld_fenster;
static struct fenster_beschreibung rueckspiegel_fenster;
static struct fenster_beschreibung karte_fenster;
static struct fenster_beschreibung kompass_fenster;
static struct fenster_beschreibung punkte_fenster;

/* die Frames zu den Fenstern muessen global gespeichert werden */
static Frame hauptframe, blickfeld_frame, rueckspiegel_frame;
static Frame karte_frame, kompass_frame, punkte_frame;

/* Zeiger auf Frames der einzelnen Fenster */
static Frame *frame_feld[] =
{
	&blickfeld_frame,
	&rueckspiegel_frame,
	&karte_frame,
	&kompass_frame,
	&punkte_frame
};

/* Namen der Fenster, wie sie in den Xresources verwendet werden */
static char *frame_name[] =
{ "frontView", "rearView", "map", "compass", "scores" };

/* Flags, ob die Fenster als default sichtbar sind */
static int frame_sichtbar_default[] =
{ 1, 0, 0, 0, 1 };

/* der Canvas der Karte wird zum Neuerzeugen der Scrollbars benoetigt */
static Canvas karte_canvas;

/* Scrollbars fuer karte_canvas */
static Scrollbar vertikaler_scrollbar;
static Scrollbar horizontaler_scrollbar;

/* Panel-Items werden von properties_apply/reset_knopf benoetigt, um
   die Werte abzufragen oder zurueckzusetzen */
static Panel_choice_item karte_scrollbar_checkbox;
static Panel_choice_item karte_zentriert_checkbox;
static Panel_slider_item zentrier_schritt_slider;
static Panel_button_item verbindung_aufbauen_knopf, pause_umschalten_knopf;

/* Panel-Text-Items fuer Servername und Spruch */
static Panel_text_item server_text, spruch_text;
/* Kamera-Modus */
static Panel_choice_item camera_mode_checkbox;

/* Werte der Panel-Items bei letztem Klicken auf Apply */
static int karte_hat_scrollbars = 0;
static int karte_ist_zentriert = 0;
static int zentrier_schritt = 25;

/* aktuelle Position der Scrolllbars von karte_canvas */
static struct punkt kartenzentrum =
	{ FENSTER_BREITE / 2, FENSTER_HOEHE / 2 };

/* der Hauptframe ist am Anfang nicht vorhanden */
static int hauptframe_gueltig = 0;

/* zu Anfang besteht noch keine Verbindung zum Server */
static int verbindung_aktiv = 0;

/* ist der Spiel-Handler aktiv und sind die Fenster offen? */
static int network_proc_active = 0;
static int network_proc_descriptor;

/* diese Strukturen werden an fenster_beschreibung.daten zugewiesen und
   enthalten die Kopien der zu zeichnenden Grafikobjekte */
static struct blickfelddaten objekte, objekte_rueck;
static struct grundrissdaten grundriss;
static int blickrichtung;
static struct punktedaten punkte;

/* Prototypen der Behandlungsroutinen fuer den connect/disconnect-Knopf */
static int connect_knopf(Panel_item knopf, Event *event);
static int disconnect_knopf(Panel_item knopf, Event *event);


/*
** fenster_init
**  setzt die Fensterinformationen, die an Canvas angehaengt sind
**
** Paramter:
**  canvas: Canvas des Fensters
**
** Seiteneffekte:
**  die an Canvas angehaengten Daten werden veraendert
*/
static void fenster_init(Canvas canvas)
{
	Xv_Window window;                     /* Window des Canvas */
	struct fenster_beschreibung *fenster; /* Daten und Funktionen
	                                         zum Fenster */

	/* Zeiger auf Informationen zum Fenster, die an Canvas angehaengt
	   sind */
	fenster = (struct fenster_beschreibung *)xv_get(canvas, XV_KEY_DATA, 1);

	/* Window ermitteln und ablegen */
	window = canvas_paint_window(canvas);
	fenster->fenster.window = xv_get(window, XV_XID);
	fenster->fenster.display = XV_DISPLAY_FROM_WINDOW(window);

	/* Ausmasse und Farbtiefe des Fensters ablegen */
	fenster->fenster.breite = (int)xv_get(window, XV_WIDTH);
	fenster->fenster.hoehe = (int)xv_get(window, XV_HEIGHT);
	fenster->fenster.farbtiefe = xv_get(window, WIN_DEPTH);

	/* der X11-Teil legt ebenfalls seine Informationen zum Fenster ab */
	fenster->X_init(&fenster->X_daten, &fenster->fenster);
}


static void do_repaint_fenster(struct fenster_beschreibung *fenster)
{
	/* Anfang des Neuzeichnens synchronisieren */
	X_zeichne_sync_anfang(fenster->fenster.display);

	/* X11-Routine, die das Neuzeichnen uebernimmt */
	fenster->X_zeichne(fenster->X_daten, &fenster->fenster,
		fenster->daten);

	/* Ende des Neuzeichnens synchronisieren */
	X_zeichne_sync_ende(fenster->fenster.display);
}


/*
** repaint_fenster
**  wird aufgerufen, wenn ein Repaint-Signal ausgeloest wurde;
**  der komplette Fensterinhalt wird neu gezeichnet
**
** Parameter:
**  canvas: Canvas des Fensters
**  window: Dummy-Parameter
**  display: Dummy-Parameter
**  xid: Dummy-Parameter
**  area: Dummy-Parameter
*/
static void repaint_fenster(Canvas canvas, Xv_Window window, Display *display,
	Window xid, Xv_xrectlist area)
{
	struct fenster_beschreibung *fenster; /* Daten und Funktionen
	                                         zum Fenster */

	/* Zeiger auf Informationen zum Fenster, die an Canvas angehaengt
	   sind */
	fenster = (struct fenster_beschreibung *)xv_get(canvas, XV_KEY_DATA, 1);

	do_repaint_fenster(fenster);
}


/*
** resize_fenster
**  wird aufgerufen, wenn die Groesse eines Fensters veraendert wurde;
**  die privaten Daten aus dem X11-Grafikteil werden neu gesetzt;
**  Neuzeichnen erfolgt jedoch erst beim repaint
**
** Parameter:
**  canvas: Canvas des Fensters
**  breite: Dummy-Parameter
**  hoehe: Dummy-Parameter
*/
static void resize_fenster(Canvas canvas, int breite, int hoehe)
{
	struct fenster_beschreibung *fenster; /* Daten und Funktionen
	                                         zum Fenster */

	/* Zeiger auf Informationen zum Fenster, die an Canvas
	   angehaengt sind */
	fenster = (struct fenster_beschreibung *)xv_get(canvas, XV_KEY_DATA, 1);

	/* Anfang des Neuzeichnens synchronisieren */
	X_zeichne_sync_anfang(fenster->fenster.display);

	/* Freigeben der privaten Daten des X11-Grafikteil */
	fenster->X_freigeben(&fenster->X_daten, &fenster->fenster);

	/* private Daten des X11-Grafikteil neu ermitteln */
	fenster_init(canvas);

	/* Ende des Neuzeichnens synchronisieren */
	X_zeichne_sync_ende(fenster->fenster.display);
}


static int pause_knopf(Panel_item knopf, Event *event)
{
	if (verbindung_aktiv)
		eingabe_simulieren(STOPSIGNAL);

	return XV_OK;
}


static int resume_knopf(Panel_item knopf, Event *event)
{
	if (verbindung_aktiv)
		eingabe_simulieren(WEITERSIGNAL);

	return XV_OK;
}


static void pause_handler(int pause_mode)
{
	if (pause_mode)
		xv_set(pause_umschalten_knopf,
			PANEL_LABEL_STRING, "Resume",
			/* auszufuehrende Funktion: */
			PANEL_NOTIFY_PROC, resume_knopf,
			0);
	else
		xv_set(pause_umschalten_knopf,
			PANEL_LABEL_STRING, "Pause",
			/* auszufuehrende Funktion: */
			PANEL_NOTIFY_PROC, pause_knopf,
			0);
}


/*
** knopf_auf_connect_schalten
**  schaltet den connect-/disconnect-Knopf auf die Aufschrift "connect";
**  macht die Eingabefelder schreibbar und schaltet die Fusszeile um
*/
static void knopf_auf_connect_schalten(void)
{
	/* Fusszeile setzen */
	xv_set(hauptframe,
		FRAME_BUSY, FALSE,
		FRAME_LEFT_FOOTER, "Not connected",
		0);

	/* disconnect-Knopf wieder in connect-Knopf umbauen */
	xv_set(verbindung_aufbauen_knopf,
		PANEL_LABEL_STRING, "Connect",
		PANEL_NOTIFY_PROC, connect_knopf,
		0);

	/* pause-Knopf initialisieren */
	xv_set(pause_umschalten_knopf,
		PANEL_LABEL_STRING, "Pause",
		PANEL_NOTIFY_PROC, pause_knopf,
		0);

	/* Servername kann wieder veraendert werden */
	xv_set(server_text,
		PANEL_READ_ONLY, FALSE,
		0);

	/* Spruch kann wieder veraendert werden */
	xv_set(spruch_text,
		PANEL_READ_ONLY, FALSE,
		0);

	/* Kamera-Modus kann wieder veraendert werden */
	xv_set(camera_mode_checkbox,
		PANEL_INACTIVE, FALSE,
		0);
}


/*
** spiel_deskriptor
**  fragt den Spiel-Deskriptor des Netzwerkteils ab
**
** Rueckgabewert:
**  der Deskriptor oder -1 falls der Deskriptor nicht
**  abgefragt werden konnte
*/
static int spiel_deskriptor(void)
{
	int *deskriptor; /* Zeiger auf den Deskriptor */

	/* Deskriptor abfragen */
	if ((deskriptor = bewegung_deskriptor()) == NULL)
		/* es wurde kein Deskriptor zurueckgegeben */
		return -1;

	/* Deskriptor weiterreichen */
	return *deskriptor;
}


static void stop_network_proc(void)
{
	int i;

	if (!network_proc_active)
		return;

	/* removed the handler for the game descriptor */
	network_proc_active = 0;
	if (network_proc_descriptor >= 0)
		notify_set_input_func(hauptframe, NULL,
			network_proc_descriptor);

	/* pop down all windows and save their previous state */
	for (i = 0; i < FRAME_ANZ; i++)
	{
		int was_open;

		was_open = xv_get(*frame_feld[i], XV_SHOW);
		xv_set(*frame_feld[i],
			XV_KEY_DATA, 0, was_open,
			FRAME_CMD_PIN_STATE, FALSE,
			XV_SHOW, i == 0 ? was_open : FALSE,
			FRAME_CMD_PIN_STATE, TRUE,
			0);
	}

	objekte.titelbild = 1;
	if (xv_get(*frame_feld[0], XV_SHOW) == TRUE)
		do_repaint_fenster(&blickfeld_fenster);
}


/*
** ende_ausloesen
**  leitet das Programmende ohne Sicherheitsabfrage ein
*/
static void ende_ausloesen(void)
{
	char text[EINGABE_TEXT_LAENGE + 30]; /* Puffer fuer Text in
	                                        der Fussleiste */

	/* muss die Verbindung noch beendet werden? */
	if (!verbindung_aktiv)
		return;

	/* Verbindung wird jetzt beendet */
	verbindung_aktiv = 0;

	/* Handler loeschen; Fensters schliessen */
	stop_network_proc();

	/* Fusszeile setzen */
	sprintf(text, "Disconnecting from %s...",
		(char *)xv_get(server_text, PANEL_VALUE));
	xv_set(hauptframe,
		FRAME_BUSY, TRUE,
		FRAME_LEFT_FOOTER, text,
		0);

	/* Verbindung zum Server beenden (mit Timeout) */
	client_spiel_beenden();

	/* korrigiert Eingabefelder, Fusszeile etc. */
	knopf_auf_connect_schalten();
}


/*
** deskriptor_bereit_handler
**  wird aufgerufen, wenn der Spiel-Deskriptor des Netzwerkteils
**  Daten bereit haelt
**
** Parameter:
**  client: Dummy-Parameter
**
** Rueckgabewert:
**  NOTIFY_DONE oder NOTIFY_ERROR
*/
static Notify_value deskriptor_bereit_handler(Notify_client client)
{
	/* falls die Verbindung bereits beendet wurde, nichts tun */
	if (!verbindung_aktiv)
		return NOTIFY_DONE;

	/* der Netzwerkteil kann hier aufgelaufene Pakete abarbeiten,
	   Ergebnis grafisch darstellen durch Aufruf der Zeichenroutinen;
	   Anmerkung: bewegung wird immer mit (0, 1) aufgerufen */
	if (bewegung(0, 1))
	{
		char **meldung; /* Feld mit Fehlermeldung */
		char *knopf;    /* Beschriftung des Knopfs */

		/* Fehlertext abfragen */
		milden_fehler_abfragen(&meldung, &knopf);

		/* Fehler anzeigen */
		ueblen_fehler_anzeigen(meldung, knopf);

		/* Verbindung beenden */
		ende_ausloesen();

		/* Eingabefelder, Fusszeile etc. umschalten */
		knopf_auf_connect_schalten();

		return NOTIFY_DONE;
	}

	return NOTIFY_DONE;
}


/*
** disconnect_knopf
**  wird aufgerufen, falls der Disconnect-Knopf gedrueckt wird, und beendet
**  nach einer Sicherheitsabfrage die Verbindung
**
** Parameter:
**  knopf: betroffener Knopf
**  event: Event, der den Knopf ausgeloest hat
**
** Rueckgabewert:
**  XV_OK oder XV_ERROR
*/
static int disconnect_knopf(Panel_item knopf, Event *event)
{
	/* Verbindung gar nicht aktiv? */
	if (!verbindung_aktiv)
		return NOTIFY_DONE;

	/* rueckfragen und Verbindung beenden */
	disconnect_abfrage();

	return XV_OK;
}


/*
** fenster_knopf
**  behandelt alle Knoepfe, mit denen ein Unterfenster geoeffnet wird,
**  in dem eine X11-Grafik aufgebaut ist
**
** Parameter:
**  menu: Menue, in dem sich der betroffene Knopf befindet
**  knopf: betroffener Knopf
**
** Rueckgabewert:
**  XV_OK oder XV_ERROR
*/
static int fenster_knopf(Menu menu, Menu_item knopf)
{
	/* enthaelt Daten und Funktionen zum Fenster */
	struct fenster_beschreibung *fenster;

	/* Zeiger auf Informationen zum Fenster, die an Panel_item angehaengt
	   sind */
	fenster = (struct fenster_beschreibung *)xv_get(knopf, XV_KEY_DATA, 1);

	/* Betroffenes Fenster wird auf sichtbar gesetzt */
	xv_set(xv_get(knopf, XV_KEY_DATA, 0),
		XV_SHOW, TRUE,
		0);

	/* Anfang des Neuzeichnens synchronisieren */
	X_zeichne_sync_anfang(fenster->fenster.display);

	/* Funktion zum Neuzeichnen des Fensters aufrufen */
	fenster->X_zeichne(fenster->X_daten,
		&fenster->fenster, fenster->daten);

	/* Ende des Neuzeichnens synchronisieren */
	X_zeichne_sync_ende(fenster->fenster.display);

	return XV_OK;
}


/*
** fenster_oeffnen_knopf
**  behandelt alle Knoepfe, mit denen ein Unterfenster geoeffnet wird
**
** Parameter:
**  knopf: betroffener Knopf
**  event: Event, durch den der betroffene Knopf ausgeloest wurde
**
** Rueckgabewert:
**  XV_OK oder XV_ERROR
*/
static int fenster_oeffnen_knopf(Panel_item knopf, Event *event)
{
	/* Betroffenes Fenster wird auf sichtbar gesetzt */
	xv_set(xv_get(knopf, XV_KEY_DATA, 0),
		XV_SHOW, TRUE,
		0);

	return XV_OK;
}


/*
** slider_aktivieren_knopf
**  passt die Sichtbarkeit eines Sliders an den Wert
**  der zugehoerigen Checkbox an
**
** Parameter:
**  knopf: die Checkbox
**  wert: neuer Zustand der Checkbox
**  event: Event, durch den die Checkbox veraendert wurde
**
** Rueckgabewert:
**  XV_OK oder XV_ERROR
*/
static int slider_aktivieren_knopf(Panel_item knopf, int wert, Event *event)
{
	/* Slider auf sichtbar/faded out setzen */
	xv_set(xv_get(knopf, XV_KEY_DATA, 0),
		PANEL_INACTIVE, wert ? FALSE : TRUE,
		0);

	return XV_OK;
}


/*
** properties_apply_knopf
**  der Apply-Knopf uebernimmt die gewaehlten Werte
**
** Parameter:
**  knopf: der Apply-Knopf
**  event: Event, durch den der Knopf ausgeloest wurde
**
** Rueckgabewert:
**  XV_OK oder XV_ERROR
*/
static int properties_apply_knopf(Panel_item knopf, Event *event)
{
	int scrollbars_existieren; /* Scrollbars existieren; nicht
	                              notwendig sichtbar */

	/* Scrollbars werden auch fuer das Zentrieren benoetigt */
	scrollbars_existieren = karte_hat_scrollbars ||
		karte_ist_zentriert;

	/* Scrolling-Parameter uebernehmen */
	karte_hat_scrollbars = xv_get(karte_scrollbar_checkbox,
		PANEL_VALUE);
	karte_ist_zentriert = xv_get(karte_zentriert_checkbox,
		PANEL_VALUE);
	zentrier_schritt = xv_get(zentrier_schritt_slider,
		PANEL_VALUE);

	/* Scrollbars erzeugen/zerstoeren? */
	if (scrollbars_existieren !=
		(karte_hat_scrollbars || karte_ist_zentriert))
		if (scrollbars_existieren)
		{
			/* Scrollbars zerstoeren */
			xv_destroy(vertikaler_scrollbar);
			xv_destroy(horizontaler_scrollbar);
		}
		else
		{
			/* Scrollbars neu erzeugen */
			vertikaler_scrollbar = xv_create(karte_canvas,
					SCROLLBAR,
				SCROLLBAR_DIRECTION, SCROLLBAR_VERTICAL,
				0);
			horizontaler_scrollbar = xv_create(karte_canvas,
					SCROLLBAR,
				SCROLLBAR_DIRECTION, SCROLLBAR_HORIZONTAL,
				0);
		}

	/* existieren jetzt Scrollbars? */
	scrollbars_existieren = karte_hat_scrollbars ||
		karte_ist_zentriert;

	if (scrollbars_existieren)
	{
		/* Scrollbars auf sichtbar/unsichtbar/faded out
		   setzen */
		xv_set(vertikaler_scrollbar,
			XV_SHOW, karte_hat_scrollbars ? TRUE : FALSE,
			SCROLLBAR_INACTIVE, karte_ist_zentriert ?
				TRUE : FALSE,
			0);
		xv_set(horizontaler_scrollbar,
			XV_SHOW, karte_hat_scrollbars ? TRUE : FALSE,
			SCROLLBAR_INACTIVE, karte_ist_zentriert ?
				TRUE : FALSE,
			0);
	}

	/* automatisches Anpassen der Canvasgroesse an die
	   Fenstergroesse nur, falls keine Scrollbars existieren */
	xv_set(karte_canvas,
		CANVAS_AUTO_SHRINK, scrollbars_existieren ?
			FALSE : TRUE,
		0);

	return XV_OK;
}


/*
** properties_reset_knopf
**  der Reset-Knopf setzt die Werte auf die vom letzten Apply zurueck
**
** Parameter:
**  knopf: der Reset-Knopf
**  event: Event, durch den der Knopf ausgeloest wurde
**
** Rueckgabewert:
**  XV_OK oder XV_ERROR
*/
static int properties_reset_knopf(Panel_item knopf, Event *event)
{
	xv_set(karte_scrollbar_checkbox,
		PANEL_VALUE, karte_hat_scrollbars,
		0);
	xv_set(karte_zentriert_checkbox,
		PANEL_VALUE, karte_ist_zentriert,
		0);
	xv_set(zentrier_schritt_slider,
		PANEL_INACTIVE, karte_ist_zentriert ? FALSE : TRUE,
		PANEL_VALUE, zentrier_schritt,
		0);

	return XV_OK;
}


/*
** abbruch_signal_handler
**  wird aufgerufen, falls Conrol-C gedrueckt wird (o.ae.) und veranlasst
**  nach einer Sicherheitsabfrage das Programmende
**
** Parameter:
**  client: Dummy-Parameter
**  signal: Dummy-Parameter
**  mode: Dummy-Parameter
**
** Rueckgabewert:
**  NOTIFY_DONE oder NOTIFY_ERROR
*/
static Notify_value abbruch_signal_handler(Notify_client client, int signal,
	Notify_signal_mode mode)
{
	ende_ausloesen();

	/* Standard-xview-Funktion fuer Programmende */
	xv_destroy_safe(hauptframe);

	return NOTIFY_DONE;
}


/*
** connect_knopf
**  wird aufgerufen, falls der Connect-Knopf gedrueckt wird,
**  und baut die Verbindung auf
**
** Parameter:
**  knopf: betroffener Knopf
**  event: Event, der den Knopf ausgeloest hat
**
** Rueckgabewert:
**  XV_OK oder XV_ERROR
*/
static int connect_knopf(Panel_item knopf, Event *event)
{
	int i;                               /* Index fuer die Fenster */
	char text[EINGABE_TEXT_LAENGE + 30]; /* Puffer fuer Text in
	                                        der Fussleiste */

	/* ist die Verbindung schon aufgebaut? */
	if (verbindung_aktiv)
		return XV_OK;

	/* Servername darf nicht mehr veraendert werden */
	xv_set(server_text,
		PANEL_READ_ONLY, TRUE,
		0);

	/* Spruch darf nicht mehr veraendert werden */
	xv_set(spruch_text,
		PANEL_READ_ONLY, TRUE,
		0);

	/* Kamera-Modus darf nicht mehr veraendert werden */
	xv_set(camera_mode_checkbox,
		PANEL_INACTIVE, TRUE,
		0);

	/* Fusszeile setzen */
	sprintf(text, "Connecting to %s...",
		(char *)xv_get(server_text, PANEL_VALUE));
	xv_set(hauptframe,
		FRAME_BUSY, TRUE,
		FRAME_LEFT_FOOTER, text,
		0);

	/* Verbindung zum Server aufbauen */
	if (client_spiel_starten((char *)xv_get(server_text, PANEL_VALUE),
		(char *)xv_get(spruch_text, PANEL_VALUE),
		(int)xv_get(camera_mode_checkbox, PANEL_VALUE)))
	{
		char **meldung; /* Feld mit Fehlermeldung */
		char *knopf;    /* Beschriftung des Knopfs */

		/* Fehlertext abfragen */
		milden_fehler_abfragen(&meldung, &knopf);

		/* Fehler anzeigen */
		ueblen_fehler_anzeigen(meldung, knopf);

		/* Eingabefelder, Fusszeile etc. umschalten */
		knopf_auf_connect_schalten();

		return XV_OK;
	}

	/* jetzt ist die Verbindung aufgebaut */
	verbindung_aktiv = 1;

	/* falls es einen Spiel-Deskriptor gibt, Bereit-Handler
	   aktivieren */
	if ((network_proc_descriptor = spiel_deskriptor()) >= 0)
		notify_set_input_func(hauptframe, deskriptor_bereit_handler,
			network_proc_descriptor);
	network_proc_active = 1;

	/* connect-Knopf wieder in disconnect-Knopf umbauen */
	xv_set(knopf,
		PANEL_LABEL_STRING, "Disconnect",
		PANEL_NOTIFY_PROC, disconnect_knopf,
		0);

	/* Fusszeile setzen */
	sprintf(text, "Connected to %s", (char *)xv_get(server_text, PANEL_VALUE));
	xv_set(hauptframe,
		FRAME_BUSY, FALSE,
		FRAME_LEFT_FOOTER, text,
		0);

	objekte.titelbild = 0;

	/* alle Fenster, die bei der letzten Verbindung sichtbar waren,
	   wieder auf sichtbar setzen */
	for (i = 0; i < FRAME_ANZ; i++)
		xv_set(*frame_feld[i],
			XV_SHOW, xv_get(*frame_feld[i], XV_KEY_DATA, 0),
			0);

	return XV_OK;
}


/*
** hauptframe_ende_handler
**  wird vor dem Zerstoeren des Hauptframes aufgerufen und setzt
**  hauptframe_gueltig zurueck, falls notwendig
**
** Parameter:
**  client: Dummy-Parameter
**  status: Grund des Aufrufs
**
** Rueckgabewert:
**  NOTIFY_DONE oder NOTIFY_ERROR
**
** Seiteneffekte:
**  hauptframe_gueltig wird veraendert
*/
static Notify_value hauptframe_ende_handler(Notify_client client,
	Destroy_status status)
{
	switch (status)
	{
		case DESTROY_CHECKING: /* Endeabfrage moeglich */
			/* muss die Verbindung noch beendet werden? */
			if (verbindung_aktiv)
			{
				verbindung_aktiv = 0;

				/* Handler loeschen; Fenster schliessen */
				stop_network_proc();

				/* Hauptfenster auf "arbeitet gerade" setzen */
				xv_set(hauptframe,
					FRAME_BUSY, TRUE,
					0);

				/* Verbindung zum Server beenden (mit Timeout) */
				client_spiel_beenden();

				/* Hauptfenster auf "fertig" setzen */
				xv_set(hauptframe,
					FRAME_BUSY, FALSE,
					0);
			}
			break;

		case DESTROY_CLEANUP: /* Hauptframe zerstoert */
		case DESTROY_PROCESS_DEATH: /* Prozessende */
			hauptframe_gueltig = 0;
			break;
	}

	/* naechsten Handler aufrufen */
	return notify_next_destroy_func(client, status);
}


/*
** notice_oeffnen
**  oeffnet ein Notice-Fenster (eine Abfragebox mit Knoepfen), wartet bis
**  ein Knopf gedrueckt wurde und gibt dessen Nummer zurueck
**
** Parameter:
**  attribute: mit attr_create_list erzeugt Liste von Attributen
**             fuer das Fenster
**
** Rueckgabewert:
**  Nummer des Knopfes oder NOTICE_FAILED fuer Fehler
*/
static int notice_oeffnen(Attr_avlist attribute)
{
	Frame frame;      /* Frame, zu dem das Notice-Fenster gehoeren soll */
	Xv_Notice notice; /* Notice-Fenster */
	int wahl;         /* Nummer des gewaehlten Knopfes */

	/* falls keine Attributliste in der uebergeordneten Routine
	   erzeugt werden konnte, Fehler zurueckgeben */
	if (attribute == NULL)
		return NOTICE_FAILED;

	/* falls hauptframe noch nicht gueltig, extra einen Frame erzeugen */
	if (hauptframe_gueltig)
		frame = hauptframe;
	else
		frame = xv_create(XV_NULL, FRAME,
			0);

	/* Notice-Fenster erzeugen, anzeigen, auf Knopf warten und
	   wieder schliessen */
	notice = xv_create(frame, NOTICE,
		ATTR_LIST, attribute,
		XV_SHOW, TRUE,
		0);

	/* welcher Knopf wurde gedrueckt? */
	wahl = xv_get(notice, NOTICE_STATUS);

	/* Notice-Fenster zerstoeren */
	xv_destroy(notice);

	/* falls extra ein Frame erzeugt wurde, wieder freigeben */
	if (!hauptframe_gueltig)
		xv_destroy(frame);

	/* Attributliste wieder freigeben */
	free(attribute);

	/* Nummer des gewaehlten Knopfes zurueckgeben */
	return wahl;
}


/*
** scrollbar_zentrieren
**  zentriert einen Scrollbar neu
**
** Parameter:
**  scrollbar: der Scrollbar
**  zentrum: das neue Zentrum in virtuellen Bildpunkten
**  skalierung: die Gesamtgroesse in virtuellen Bildpunkten
*/
static void scrollbar_zentrieren(Scrollbar scrollbar, int zentrum,
	int skalierung)
{
	int gesamt;        /* Gesamtgroesse des zu scrollenden Objekts */
	int ausschnitt;    /* Groesse des sichtbaren Ausschnitts */
	int neue_position; /* neue Position des Ausschnitts */
	int alte_position; /* alte Position des Ausschnitts */

	/* Gesamt-, Ausschnittgroesse und alte Position abfragen */
	gesamt = xv_get(scrollbar, SCROLLBAR_OBJECT_LENGTH);
	ausschnitt = xv_get(scrollbar, SCROLLBAR_VIEW_LENGTH);
	alte_position = xv_get(scrollbar, SCROLLBAR_VIEW_START);

	/* neue Position */
	neue_position = zentrum * gesamt / skalierung - ausschnitt / 2;

	/* weicht die neue Position schon um die Zentrier-Schrittweite von
	   der alten ab? */
	if (neue_position < alte_position -
		ausschnitt * zentrier_schritt / 100 ||
		neue_position > alte_position +
		ausschnitt * zentrier_schritt / 100)
	{
		/* neue Position mit 0 und gesamt - ausschnitt beschraenken */
		if (neue_position < 0)
			neue_position = 0;
		if (neue_position > gesamt - ausschnitt)
			neue_position = gesamt - ausschnitt;

		if (neue_position != alte_position)
			/* neue Position setzen */
			xv_set(scrollbar,
				SCROLLBAR_VIEW_START, neue_position,
				0);
	}
}


/* bis hier lokaler Teil                       */
/***********************************************/
/* ab hier globaler Teil                       */


/*
** zeichne_blickfeld
**  zeichnet 3D-Darstellung im Hauptfenster
**
** Parameter:
**  objekte_neu: Liste mit allen zu zeichnenden Grafikobjekten
**
** Seiteneffekte:
**  objekte wird neu gesetzt
*/
void zeichne_blickfeld(struct objektdaten *objekte_neu)
{
	/* lokale Kopie der Daten anlegen */
	objekte_kopieren(&objekte.objekte, objekte_neu);

	/* falls Fenster geoeffnet, X11-Routine aufrufen */
	if (xv_get(blickfeld_frame, FRAME_CLOSED) == FALSE &&
		xv_get(blickfeld_frame, XV_SHOW) == TRUE)
		blickfeld_fenster.X_zeichne(blickfeld_fenster.X_daten,
			&blickfeld_fenster.fenster, blickfeld_fenster.daten);
}


/*
** zeichne_rueckspiegel
**  zeichnet 3D-Darstellung im Rueckspiegel
**
** Parameter:
**  objekte_rueck_neu: Liste mit allen zu zeichnenden Grafikobjekten
**
** Seiteneffekte:
**  objekte_rueck wird neu gesetzt
*/
void zeichne_rueckspiegel(struct objektdaten *objekte_rueck_neu)
{
	/* lokale Kopie der Daten anlegen */
	objekte_kopieren(&objekte_rueck.objekte, objekte_rueck_neu);

	/* falls Fenster geoeffnet und sichtbar, X11-Routine aufrufen */
	if (xv_get(rueckspiegel_frame, FRAME_CLOSED) == FALSE &&
		xv_get(rueckspiegel_frame, XV_SHOW) == TRUE)
		rueckspiegel_fenster.X_zeichne(rueckspiegel_fenster.X_daten,
			&rueckspiegel_fenster.fenster, rueckspiegel_fenster.daten);
}


/*
** zeichne_karte
**  bewegt Spieler auf der Karte
**
** Parameter
**  spieler_index: Index des Spielers selbst in der Liste
**                 oder -1, falls Spieler tot
**  anzkreise_neu: Anzahl der Spieler
**  kreise_neu: Liste mit Positionen der Spieler
**
** Seiteneffekte:
**  grundriss wird neu gesetzt
*/
void zeichne_karte(int spieler_index, int anzkreise_neu,
	struct kartenkreis *kreise_neu)
{
	if (spieler_index >= 0)
	{
		kartenzentrum.x = kreise_neu[spieler_index].mittelpunkt.x;
		kartenzentrum.y = kreise_neu[spieler_index].mittelpunkt.y;

		if (karte_ist_zentriert)
		{
			scrollbar_zentrieren(horizontaler_scrollbar,
				kartenzentrum.x, FENSTER_BREITE);
			scrollbar_zentrieren(vertikaler_scrollbar,
				kartenzentrum.y, FENSTER_HOEHE);
		}
	}

	/* lokale Kopie der Daten fuer X_zeichne_karte retten und neu anlegen */
	grundriss.anzkreise_alt = grundriss.anzkreise;
	grundriss.kreise_alt = grundriss.kreise;
	liste_kopieren(&grundriss.anzkreise, (void **)&grundriss.kreise,
		&anzkreise_neu, (void **)&kreise_neu, sizeof(struct kartenkreis));

	/* falls Fenster sichtbar, X11-Routine aufrufen */
	if (xv_get(karte_frame, XV_SHOW) == TRUE)
		X_zeichne_karte(karte_fenster.X_daten, &karte_fenster.fenster,
			karte_fenster.daten);

	/* lokale Kopie der veralteten Daten freigeben */
	liste_freigeben(&grundriss.anzkreise_alt,
		(void **)&grundriss.kreise_alt);
	grundriss.anzkreise_alt = 0;
	grundriss.kreise_alt = NULL;
}


/*
** zeichne_grundriss
**  zeichnet den Grundriss der Karte und die Spieler neu
**
** Parameter:
**  anzlinien: Anzahl der Linienstuecke
**  linien_neu: Liste mit Daten ueber Linienstuecke
**
** Seiteneffekte:
**  grundriss wird neu gesetzt
*/
void zeichne_grundriss(int anzlinien_neu, struct kartenlinie *linien_neu)
{
	/* lokale Kopie der Daten freigeben und neu anlegen */
	liste_freigeben(&grundriss.anzlinien, (void **)&grundriss.linien);
	liste_kopieren(&grundriss.anzlinien, (void **)&grundriss.linien,
		&anzlinien_neu, (void **)&linien_neu, sizeof(struct kartenlinie));

	/* falls Fenster sichtbar, X11-Routine aufrufen */
	if (xv_get(karte_frame, XV_SHOW) == TRUE)
		karte_fenster.X_zeichne(karte_fenster.X_daten,
			&karte_fenster.fenster, karte_fenster.daten);
}


/*
** zeichne_kompass
**  zeichnet Kompass
**
** Parameter:
**  blickrichtung_neu: Blickrichtung des Spieler (0..TRIGANZ)
**                     oder -1, falls Spieler tot
**
** Seiteneffekte:
**  setzt blickrichtung
*/
void zeichne_kompass(int blickrichtung_neu)
{
	/* lokale Kopie der Blickrichtung */
	blickrichtung = blickrichtung_neu;

	/* falls Fenster geoeffnet und sichtbar, X11-Routine aufrufen */
	if (xv_get(kompass_frame, FRAME_CLOSED) == FALSE &&
		xv_get(kompass_frame, XV_SHOW) == TRUE)
		kompass_fenster.X_zeichne(kompass_fenster.X_daten,
			&kompass_fenster.fenster, kompass_fenster.daten);
}


/*
** zeichne_punkte
**  zeichnet den Punktestand
**
** Parameter:
**  punkte_neu: Daten, die den Punktestand beschreiben
**
** Seiteneffekte:
**  punkte wird neu gesetzt
*/
void zeichne_punkte(struct punktedaten *punkte_neu)
{
	/* lokale Kopie der Daten freigeben und neu anlegen */
	liste_freigeben(&punkte.anzpunkte, (void **)&punkte.punkte);
	liste_kopieren(&punkte.anzpunkte, (void **)&punkte.punkte,
		&punkte_neu->anzpunkte, (void **)&punkte_neu->punkte,
		sizeof(struct punktestand));

	/* falls Fenster geoeffnet und sichtbar, X11-Routine aufrufen */
	if (xv_get(punkte_frame, FRAME_CLOSED) == FALSE &&
		xv_get(punkte_frame, XV_SHOW) == TRUE)
		punkte_fenster.X_zeichne(punkte_fenster.X_daten,
			&punkte_fenster.fenster, punkte_fenster.daten);
}


/*
** zeichne_sync_anfang
**  synchronisiert den Anfang des Zeichnens
*/
void zeichne_sync_anfang(void)
{
	/* weiterreichen an X11-Teil */
	X_zeichne_sync_anfang(XV_DISPLAY_FROM_WINDOW(hauptframe));
}


/*
** zeichne_sync_ende
**  synchronisiert das Ende des Zeichnens
*/
void zeichne_sync_ende(void)
{
	/* weiterreichen an X11-Teil */
	X_zeichne_sync_ende(XV_DISPLAY_FROM_WINDOW(hauptframe));
}


/*
** grafik_schleife
**  startet xview-grafik-schleife,
**  wird erst nach Abbau der Grafik wieder verlassen
*/
void grafik_schleife(void)
{
	int i; /* Index fuer Fenster */

	/* fuer alle Fenster die Grundeinstellung
	   (sichtbar/unsichtbar) feststellen */
	for (i = 0; i < FRAME_ANZ; i++)
	{
		/* Name und Klasse des Fensters fuer die Xresources */
		char name[40], klasse[40];

		/* Name und Klasse erzeugen */
		sprintf(name, "iMaze.%s.show", frame_name[i]);
		sprintf(klasse, "IMaze.%s.Show", frame_name[i]);
		if (klasse[6] >= 'a' && klasse[6] <= 'z')
			klasse[6] += 'A' - 'a';

		/* Xresources abfragen und default in
		   frame_sichtbar_default beruecksichtigen */
		xv_set(*frame_feld[i],
			XV_KEY_DATA, 0, defaults_get_boolean(name, klasse,
				frame_sichtbar_default[i]) ? TRUE : FALSE,
			0);
	}

	/* Titelbild ist zu anfang immer sichtbar */
	xv_set(*frame_feld[0], XV_SHOW, TRUE, 0);

	/* Haupt-Schleife von xview */
	xv_main_loop(hauptframe);

	/* der Hauptframe ist nach dem Ende nicht mehr vorhanden */
	hauptframe_gueltig = 0;
}


/*
** grafik_init
**  erzeugt die Fensterstruktur mit Knoepfen etc.
**
** Parameter:
**  argc: Zeiger auf Anzahl der an das Programm uebergebenen Parameter
**  argv: Zeiger auf Liste mit Strings, die die Aufrufparameter enthalten
**
** Rueckgabewert:
**  0 bei ordentlicher Initialisierung, 1 bei Fehler
**
** Seiteneffekte:
**  *argc und *argv werden veraendert
**  alle globalen Variablen werden gesetzt bzw. initialisiert
*/
int grafik_init(int *argc, char **argv)
{
	Panel hauptpanel; /* Panel mit allen linksbuendigen Knoepfen */

	/* Listen fuer Zwischenspeicherung der Grafikobjekte initialisieren */
	objekt_listen_init(&objekte.objekte);
	objekt_listen_init(&objekte_rueck.objekte);
	liste_initialisieren(&grundriss.anzlinien, (void **)&grundriss.linien);
	liste_initialisieren(&grundriss.anzkreise, (void **)&grundriss.kreise);
	grundriss.anzkreise_alt = 0;
	grundriss.kreise_alt = NULL;
	liste_initialisieren(&punkte.anzpunkte, (void **)&punkte.punkte);

	/* Rueckspiegel hat im Gegensatz zur Frontansicht kein Zielkreuz */
	objekte.fadenkreuz = 1;
	objekte_rueck.fadenkreuz = 0;

	/* die Frontansicht zeigt zu anfang das Titelbild */
	objekte.titelbild = 1;
	objekte_rueck.titelbild = 0;

	/* Initialisierung der blickrichtung (Kompassnadel nicht zeichnen) */
	blickrichtung = -1;

	/* xview wertet seine Kommandozeilen-Parameter aus und fuehrt
	   Initialisierung aus */
	xv_init(XV_INIT_ARGC_PTR_ARGV, argc, argv, 0);

	/* Erzeugen des Haupfensters */
	hauptframe = xv_create(XV_NULL, FRAME,
		FRAME_LABEL, "iMaze - Version 1.4",
		FRAME_ICON, xv_create(XV_NULL, ICON, /* das Icon */
			ICON_IMAGE, xv_create(XV_NULL, SERVER_IMAGE,
				XV_WIDTH, 64,
				XV_HEIGHT, 64,
				SERVER_IMAGE_BITS, icon_daten,
				0),
			ICON_MASK_IMAGE, xv_create(XV_NULL, SERVER_IMAGE,
				XV_WIDTH, 64,
				XV_HEIGHT, 64,
				SERVER_IMAGE_BITS, icon_maske_daten,
				0),
			XV_LABEL, "iMaze", /* der Text unter dem Icon */
			0),
		FRAME_SHOW_RESIZE_CORNER, FALSE,
		FRAME_SHOW_FOOTER, TRUE,
		FRAME_LEFT_FOOTER, "Not connected",
		0);

	/* Erzeugen des Fensters fuer den Blick nach vorne */
	blickfeld_frame = xv_create(hauptframe, FRAME_CMD,
		FRAME_LABEL, "iMaze: Front View",
		FRAME_CMD_PUSHPIN_IN, TRUE,
		FRAME_SHOW_RESIZE_CORNER, TRUE, /* Groesse veraendern erlaubt */
		0);

	/* kein Panel-Bereich erwuenscht */
	xv_set(xv_get(blickfeld_frame, FRAME_CMD_PANEL),
		XV_SHOW, FALSE,
		0);

	/* Handler fuer Zerstoeren des Hauptframes */
	notify_interpose_destroy_func(hauptframe, hauptframe_ende_handler);

	/* das Hauptfenster ist vorhanden */
	hauptframe_gueltig = 1;

	/* X11-Funktion, die die Farben initialisiert */
	if (X_farben_init(XV_DISPLAY_FROM_WINDOW(hauptframe),
		(int)xv_get(hauptframe, SCREEN_NUMBER), NULL))
	{
		/* Standard-xview-Funktion zum Zerstoeren des Hauptframes */
		xv_destroy_safe(hauptframe);

		/* das Hauptfenster ist schon wieder weg */
		hauptframe_gueltig = 0;

		return 1;
	}

	/* Handler fuer die Verwaltung von Signalen vom TTY
	   (Control-C und Hangup) und normalem Kill */
	notify_set_signal_func(hauptframe, abbruch_signal_handler,
		SIGINT, NOTIFY_SYNC);
	notify_set_signal_func(hauptframe, abbruch_signal_handler,
		SIGHUP, NOTIFY_SYNC);
	notify_set_signal_func(hauptframe, abbruch_signal_handler,
		SIGTERM, NOTIFY_SYNC);


	{
		Canvas blickfeld_canvas;

		/* Zeichenfeld fuer Fenster mit Blick nach vorne initialisieren */
		blickfeld_canvas = xv_create(blickfeld_frame, CANVAS,
			XV_Y, 0,               /* y-Koordinate auf 0 */
			CANVAS_X_PAINT_WINDOW, TRUE,
			CANVAS_FIXED_IMAGE, FALSE,
			CANVAS_RETAINED, FALSE,
			/* Anhaengen von Struktur mit Daten und Funktion, die beim
			   Neuzeichnen erforderlich sind */
			XV_KEY_DATA, 1, &blickfeld_fenster,
			0);

		/* Initialisierung dieser Struktur */
		blickfeld_fenster.daten = &objekte;
		blickfeld_fenster.X_zeichne = X_zeichne_blickfeld;
		blickfeld_fenster.X_init = X_fenster_init;
		blickfeld_fenster.X_freigeben = X_fenster_freigeben;

		/* initialisiert Daten fuer X11-Grafik in blickfeld_fenster */
		fenster_init(blickfeld_canvas);

		/* Groesse des Zeichenfeldes setzen */
		xv_set(blickfeld_canvas,
			CANVAS_REPAINT_PROC, repaint_fenster,
			CANVAS_RESIZE_PROC, resize_fenster,
			XV_WIDTH, defaults_get_integer("iMaze.frontView.width",
				"IMaze.FrontView.Width",
				xv_get(blickfeld_canvas, XV_WIDTH)),
			XV_HEIGHT, defaults_get_integer("iMaze.frontView.height",
				"IMaze.FrontView.Height",
				xv_get(blickfeld_canvas, XV_HEIGHT)),
			0);

		/* Groesse des Fensters anpassen */
		window_fit(blickfeld_frame);

		{
			/* display und screen zum Abfragen der Bildschirmgroesse */
			Display *display;
			int screen;

			/* display und screen setzen */
			display = XV_DISPLAY_FROM_WINDOW(hauptframe);
			screen = xv_get(hauptframe, SCREEN_NUMBER);

			/* Position des Fensters setzen */
			xv_set(blickfeld_frame,
				XV_X, defaults_get_integer("iMaze.frontView.x",
					"IMaze.FrontView.X", (DisplayWidth(display, screen) -
					xv_get(blickfeld_frame, XV_WIDTH)) / 2),
				XV_Y, defaults_get_integer("iMaze.frontView.y",
					"IMaze.FrontView.Y", (DisplayHeight(display, screen) -
					xv_get(blickfeld_frame, XV_HEIGHT)) / 2),
				0);
		}
	}


	{
		/* initialisiert das Kartenfenster als Unterfenster */
		karte_frame = xv_create(hauptframe, FRAME_CMD,
			XV_X, defaults_get_integer("iMaze.map.x", "IMaze.Map.X", 0),
			XV_Y, defaults_get_integer("iMaze.map.y", "IMaze.Map.Y", 0),
			FRAME_LABEL, "iMaze: Map",
			FRAME_CMD_PUSHPIN_IN, TRUE,
			FRAME_SHOW_RESIZE_CORNER, TRUE, /* Groesse veraendern erlaubt */
			0);

		/* kein Panel-Bereich erwuenscht */
		xv_set(xv_get(karte_frame, FRAME_CMD_PANEL),
			XV_SHOW, FALSE,
			0);

		/* Zeichenfeld initialisieren */
		karte_canvas = xv_create(karte_frame, CANVAS,
			CANVAS_X_PAINT_WINDOW, TRUE,
			CANVAS_FIXED_IMAGE, FALSE,
			CANVAS_RETAINED, FALSE,
			XV_Y, 0,
			/* Anhaengen von Struktur mit Daten und Funktion, die beim
			   Neuzeichnen erforderlich sind */
			XV_KEY_DATA, 1, &karte_fenster,
			0);

		if (karte_hat_scrollbars || karte_ist_zentriert)
		{
			/* Scrollbars erzeugen und auf sichtbar/unsichtbar/faded out
			   setzen */
			vertikaler_scrollbar = xv_create(karte_canvas, SCROLLBAR,
				SCROLLBAR_DIRECTION, SCROLLBAR_VERTICAL,
				XV_SHOW, karte_hat_scrollbars ? TRUE : FALSE,
				SCROLLBAR_INACTIVE, karte_ist_zentriert ? TRUE : FALSE,
				0);
			horizontaler_scrollbar = xv_create(karte_canvas, SCROLLBAR,
				SCROLLBAR_DIRECTION, SCROLLBAR_HORIZONTAL,
				XV_SHOW, karte_hat_scrollbars ? TRUE : FALSE,
				SCROLLBAR_INACTIVE, karte_ist_zentriert ? TRUE : FALSE,
				0);
		}

		/* Standard-Fenstergroesse setzen */
		xv_set(karte_canvas,
			XV_WIDTH, defaults_get_integer("iMaze.map.width",
				"IMaze.Map.Width",
				xv_get(karte_canvas, XV_HEIGHT) * 3 / 4),
			XV_HEIGHT, defaults_get_integer("iMaze.map.height",
				"IMaze.Map.Height",
				xv_get(karte_canvas, XV_HEIGHT) * 3 / 4),
			0);
		window_fit(karte_frame);

		/* Initialisierung der oben genannten Struktur */
		karte_fenster.daten = &grundriss;
		karte_fenster.X_zeichne = X_zeichne_grundriss;
		karte_fenster.X_init = X_karte_init;
		karte_fenster.X_freigeben = X_karte_freigeben;

		/* initialisiert Daten fuer X11-Grafik in karte_fenster */
		fenster_init(karte_canvas);

		/* Funktionen fuer Neuzeichen setzen */
		xv_set(karte_canvas,
			CANVAS_REPAINT_PROC, repaint_fenster,
			CANVAS_RESIZE_PROC, resize_fenster,
			0);

		/* Kartengroesse automatisch an Fenstergroesse anpassen? */
		xv_set(karte_canvas,
			CANVAS_AUTO_SHRINK, karte_hat_scrollbars ||
				karte_ist_zentriert ? FALSE : TRUE,
			0);
	}


	{
		Canvas rueckspiegel_canvas;

		/* initialisiert das Rueckspiegelfenster als Unterfenster */
		rueckspiegel_frame = xv_create(hauptframe, FRAME_CMD,
			XV_X, defaults_get_integer("iMaze.rearView.x",
				"IMaze.RearView.X", xv_get(karte_frame, XV_WIDTH) + 12),
			XV_Y, defaults_get_integer("iMaze.rearView.y",
				"IMaze.RearView.Y", 0),
			FRAME_LABEL, "iMaze: Rear View",
			FRAME_CMD_PUSHPIN_IN, TRUE,
			FRAME_SHOW_RESIZE_CORNER, TRUE, /* Groesse veraendern erlaubt */
			0);

		/* kein Panel-Bereich erwuenscht */
		xv_set(xv_get(rueckspiegel_frame, FRAME_CMD_PANEL),
			XV_SHOW, FALSE,
			0);

		/* Zeichenfeld initialisieren */
		rueckspiegel_canvas = xv_create(rueckspiegel_frame, CANVAS,
			CANVAS_X_PAINT_WINDOW, TRUE,
			CANVAS_FIXED_IMAGE, FALSE,
			CANVAS_RETAINED, FALSE,
			XV_Y, 0,
			/* Anhaengen von Struktur mit Daten und Funktion, die beim
			   Neuzeichnen erforderlich sind */
			XV_KEY_DATA, 1, &rueckspiegel_fenster,
			0);

		/* Standard-Fenstergroesse setzen */
		xv_set(rueckspiegel_canvas,
			XV_WIDTH, defaults_get_integer("iMaze.rearView.width",
				"IMaze.RearView.Width",
				xv_get(rueckspiegel_canvas, XV_WIDTH) / 3),
			XV_HEIGHT, defaults_get_integer("iMaze.rearView.height",
				"IMaze.RearView.Height",
				xv_get(rueckspiegel_canvas, XV_HEIGHT) / 3),
			0);
		window_fit(rueckspiegel_frame);

		/* Initialisierung der oben genannten Struktur */
		rueckspiegel_fenster.daten = &objekte_rueck;
		rueckspiegel_fenster.X_zeichne = X_zeichne_blickfeld;
		rueckspiegel_fenster.X_init = X_fenster_init;
		rueckspiegel_fenster.X_freigeben = X_fenster_freigeben;

		/* initialisiert Daten fuer X11-Grafik in rueckspiegel_fenster */
		fenster_init(rueckspiegel_canvas);

		/* Funktionen fuer Neuzeichen setzen */
		xv_set(rueckspiegel_canvas,
			CANVAS_REPAINT_PROC, repaint_fenster,
			CANVAS_RESIZE_PROC, resize_fenster,
			0);
	}


	{
		Canvas kompass_canvas;

		/* initialisiert das Kompass-Fenster als Unterfenster */
		kompass_frame = xv_create(hauptframe, FRAME_CMD,
			XV_X, defaults_get_integer("iMaze.compass.x",
				"IMaze.Compass.X", xv_get(karte_frame, XV_WIDTH) + 12 +
				xv_get(rueckspiegel_frame, XV_WIDTH) + 12),
			XV_Y, defaults_get_integer("iMaze.compass.y",
				"IMaze.Compass.Y", 0),
			FRAME_LABEL, "iMaze: Compass",
			FRAME_CMD_PUSHPIN_IN, TRUE,
			FRAME_SHOW_RESIZE_CORNER, TRUE, /* Groesse veraendern erlaubt */
			0);

		/* kein Panel-Bereich erwuenscht */
		xv_set(xv_get(kompass_frame, FRAME_CMD_PANEL),
			XV_SHOW, FALSE,
			0);

		/* Zeichenfeld initialisieren */
		kompass_canvas = xv_create(kompass_frame, CANVAS,
			CANVAS_X_PAINT_WINDOW, TRUE,
			CANVAS_FIXED_IMAGE, FALSE,
			CANVAS_RETAINED, FALSE,
			/* Anhaengen von Struktur mit Daten und Funktion, die beim
			   Neuzeichnen erforderlich sind */
			XV_Y, 0,
			XV_KEY_DATA, 1, &kompass_fenster,
			0);

		/* Standard-Fenstergroesse setzen */
		xv_set(kompass_canvas,
			XV_WIDTH, defaults_get_integer("iMaze.compass.width",
				"IMaze.Compass.Width",
				xv_get(kompass_canvas, XV_HEIGHT) / 4),
			XV_HEIGHT, defaults_get_integer("iMaze.compass.height",
				"IMaze.Compass.Height",
				xv_get(kompass_canvas, XV_HEIGHT) / 4),
			0);
		window_fit(kompass_frame);

		/* Initialisierung der oben genannten Struktur */
		kompass_fenster.daten = &blickrichtung;
		kompass_fenster.X_zeichne = X_zeichne_kompass;
		kompass_fenster.X_init = X_fenster_init;
		kompass_fenster.X_freigeben = X_fenster_freigeben;

		/* initialisiert Daten fuer X11-Grafik in kompass_fenster */
		fenster_init(kompass_canvas);

		/* Funktionen fuer Neuzeichen setzen */
		xv_set(kompass_canvas,
			CANVAS_REPAINT_PROC, repaint_fenster,
			CANVAS_RESIZE_PROC, resize_fenster,
			0);
	}


	{
		Canvas punkte_canvas;

		/* initialisiert das Punkte-Fenster als Unterfenster */
		punkte_frame = xv_create(hauptframe, FRAME_CMD,
			XV_X, defaults_get_integer("iMaze.scores.x",
				"IMaze.Scores.X", xv_get(blickfeld_frame, XV_X) +
				xv_get(blickfeld_frame, XV_WIDTH) + 12),
			XV_Y, defaults_get_integer("iMaze.scores.y",
				"IMaze.Scores.Y", xv_get(blickfeld_frame, XV_Y)),
			FRAME_LABEL, "iMaze: Scores",
			FRAME_CMD_PUSHPIN_IN, TRUE,
			FRAME_SHOW_RESIZE_CORNER, TRUE, /* Groesse veraendern erlaubt */
			0);

		/* kein Panel-Bereich erwuenscht */
		xv_set(xv_get(punkte_frame, FRAME_CMD_PANEL),
			XV_SHOW, FALSE,
			0);

		/* Zeichenfeld initialisieren */
		punkte_canvas = xv_create(punkte_frame, CANVAS,
			CANVAS_X_PAINT_WINDOW, TRUE,
			CANVAS_FIXED_IMAGE, FALSE,
			CANVAS_RETAINED, FALSE,
			/* Anhaengen von Struktur mit Daten und Funktion, die beim
			   Neuzeichnen erforderlich sind */
			XV_Y, 0,
			XV_KEY_DATA, 1, &punkte_fenster,
			0);

		/* Standard-Fenstergroesse setzen */
		xv_set(punkte_canvas,
			XV_WIDTH, defaults_get_integer("iMaze.scores.width",
				"IMaze.Scores.Width", 82),
			XV_HEIGHT, defaults_get_integer("iMaze.scores.height",
				"IMaze.Scores.Height", 302),
			0);

		window_fit(punkte_frame);

		/* Initialisierung der oben genannten Struktur */
		punkte_fenster.daten = &punkte;
		punkte_fenster.X_zeichne = X_zeichne_punkte;
		punkte_fenster.X_init = X_fenster_init;
		punkte_fenster.X_freigeben = X_fenster_freigeben;

		/* initialisiert Daten fuer X11-Grafik in kompass_fenster */
		fenster_init(punkte_canvas);

		/* Funktionen fuer Neuzeichen setzen */
		xv_set(punkte_canvas,
			CANVAS_REPAINT_PROC, repaint_fenster,
			CANVAS_RESIZE_PROC, resize_fenster,
			0);
	}


	/* erzeugt die Panel-Leiste */
	hauptpanel = xv_create(hauptframe, PANEL,
		PANEL_LAYOUT, PANEL_HORIZONTAL, /* Ausrichtung horizontal */
		0);

	/* erzeugt das "Open"-Menue */
	xv_create(hauptpanel, PANEL_BUTTON,
		PANEL_LABEL_STRING, "Open",
		PANEL_ITEM_MENU, xv_create(XV_NULL, MENU,
			MENU_ITEM,
				MENU_STRING, "Front View",
				MENU_ACTION_PROC, fenster_knopf,
				/* zu oeffnender Frame */
				XV_KEY_DATA, 0, blickfeld_frame,
				/* Anhaengen von Struktur mit Daten und Funktion, die beim
				   Neuzeichnen erforderlich sind */
				XV_KEY_DATA, 1, &blickfeld_fenster,
				0,
			MENU_ITEM,
				MENU_STRING, "Rear View",
				MENU_ACTION_PROC, fenster_knopf,
				/* zu oeffnender Frame */
				XV_KEY_DATA, 0, rueckspiegel_frame,
				/* Anhaengen von Struktur mit Daten und Funktion, die beim
				   Neuzeichnen erforderlich sind */
				XV_KEY_DATA, 1, &rueckspiegel_fenster,
				0,
			MENU_ITEM,
				MENU_STRING, "Map",
				MENU_ACTION_PROC, fenster_knopf,
				/* zu oeffnender Frame */
				XV_KEY_DATA, 0, karte_frame,
				/* Anhaengen von Struktur mit Daten und Funktion, die beim
				   Neuzeichnen erforderlich sind */
				XV_KEY_DATA, 1, &karte_fenster,
				0,
			MENU_ITEM,
				MENU_STRING, "Compass",
				MENU_ACTION_PROC, fenster_knopf,
				/* zu oeffnender Frame */
				XV_KEY_DATA, 0, kompass_frame,
				/* Anhaengen von Struktur mit Daten und Funktion, die beim
				   Neuzeichnen erforderlich sind */
				XV_KEY_DATA, 1, &kompass_fenster,
				0,
			MENU_ITEM,
				MENU_STRING, "Scores",
				MENU_ACTION_PROC, fenster_knopf,
				/* zu oeffnender Frame */
				XV_KEY_DATA, 0, punkte_frame,
				/* Anhaengen von Struktur mit Daten und Funktion, die beim
				   Neuzeichnen erforderlich sind */
				XV_KEY_DATA, 1, &punkte_fenster,
				0,
			0),
		0);

	{
		Frame properties_frame; /* Properties-Fenster */
		Panel karte_properties; /* Properties fuer Karte */

		/* Properties-Fenster mit Panel erzeugen */
		properties_frame = xv_create(karte_frame, FRAME_PROPS,
			XV_X, xv_get(karte_frame, XV_WIDTH) + 12,
			XV_Y, xv_get(rueckspiegel_frame, XV_HEIGHT) + 32,
			FRAME_LABEL, "iMaze: Properties",
			0);

		/* Properties-Button erzeugen */
		xv_create(hauptpanel, PANEL_BUTTON,
			PANEL_LABEL_STRING, "Properties",
			/* auszufuehrende Funktion: */
			PANEL_NOTIFY_PROC, fenster_oeffnen_knopf,
			XV_KEY_DATA, 0, properties_frame, /* zu oeffnender Frame */
			0);

		/* Properties-Panel von Frame abfragen */
		karte_properties = xv_get(properties_frame, FRAME_PROPS_PANEL);

		{
			xv_set(karte_properties,
				PANEL_LAYOUT, PANEL_VERTICAL,
				0);

			/* Checkbox erzeugen */
			karte_scrollbar_checkbox = xv_create(karte_properties,
					PANEL_CHECK_BOX,
				PANEL_LABEL_STRING, "  Map scrolling: ",
				PANEL_CHOICE_STRING, 0, "with scrollbars",
				PANEL_VALUE, karte_hat_scrollbars,
				0);

			/* Checkbox erzeugen */
			karte_zentriert_checkbox = xv_create(karte_properties,
					PANEL_CHECK_BOX,
				PANEL_NEXT_ROW, 0,
				PANEL_LABEL_STRING, " ",
				PANEL_CHOICE_STRING, 0, "player is centered",
				PANEL_VALUE, karte_ist_zentriert,
				0);

			/* Slider erzeugen */
			zentrier_schritt_slider = xv_create(karte_properties,
					PANEL_SLIDER,
				PANEL_LABEL_STRING, "  Step size (% of window size): ",
				PANEL_INACTIVE, karte_ist_zentriert ? FALSE : TRUE,
				PANEL_VALUE, zentrier_schritt,
				PANEL_MAX_VALUE, 50,
				0);

			/* Slider-aktivieren-Knopf einbinden */
			xv_set(karte_zentriert_checkbox,
				PANEL_NOTIFY_PROC, slider_aktivieren_knopf,
				XV_KEY_DATA, 0, zentrier_schritt_slider,
				0);
		}

		{
			Panel_item item;                /* ein Item des Teilpanels */
			Panel_button_item apply_button; /* Objekt Apply-Knopf */
			Panel_button_item reset_button; /* Objekt Reset-Knopf */
			int max_label_breite;           /* Breite der laengsten
			                                   Beschriftung */

			/* Teilpanel direkt unter dem Properties-Panel angeordnet */
			xv_set(karte_properties,
				XV_SHOW, TRUE,
				PANEL_LAYOUT, PANEL_VERTICAL,
				0);

			/* Apply-Knopf erzeugen */
			apply_button = xv_create(karte_properties, PANEL_BUTTON,
				PANEL_NEXT_ROW, 3 * xv_get(karte_properties, PANEL_ITEM_X_GAP),
				PANEL_LABEL_STRING, "Apply",
				PANEL_NOTIFY_PROC, properties_apply_knopf,
				0);

			/* Reset-Knopf neben dem Apply-Knopf erzeugen */
			xv_set(karte_properties,
				PANEL_DEFAULT_ITEM, apply_button,
				PANEL_LAYOUT, PANEL_HORIZONTAL,
				0);
			reset_button = xv_create(karte_properties, PANEL_BUTTON,
				PANEL_LABEL_STRING, "Reset",
				PANEL_NOTIFY_PROC, properties_reset_knopf,
				0);

			/* Breite der laengsten Beschriftung feststellen */
			max_label_breite = 0;
			PANEL_EACH_ITEM(karte_properties, item)
				if (item != apply_button && item != reset_button &&
					xv_get(item, PANEL_LABEL_WIDTH) > max_label_breite)
					max_label_breite = xv_get(item, PANEL_LABEL_WIDTH);
			PANEL_END_EACH

			/* Beschriftungen ausser fuer Apply-/Reset-Button
			   rechtsbuendig anordnen */
			PANEL_EACH_ITEM(karte_properties, item)
				if (item != apply_button && item != reset_button)
					xv_set(item,
						XV_X, max_label_breite -
							xv_get(item, PANEL_LABEL_WIDTH),
						0);
			PANEL_END_EACH

			/* Groesse des Teilpanels anpassen */
			window_fit(karte_properties);

			/* Position des Apply-/Reset-Buttons anpassen */
			xv_set(apply_button,
				XV_X, (xv_get(karte_properties, XV_WIDTH) -
					xv_get(apply_button, PANEL_LABEL_WIDTH) -
					xv_get(reset_button, PANEL_LABEL_WIDTH)) / 3,
				0);
			xv_set(reset_button,
				XV_X, (xv_get(karte_properties, XV_WIDTH) -
					xv_get(apply_button, PANEL_LABEL_WIDTH) -
					xv_get(reset_button, PANEL_LABEL_WIDTH)) * 2 / 3 +
					xv_get(apply_button, PANEL_LABEL_WIDTH),
				0);

			window_fit(properties_frame);
		}
	}

	{
		int max_label_breite; /* Breite der laengsten Beschriftung
		                         eines Eingabefeldes */
		Panel_item item;      /* ein Eingabefeld */
		char *default_spruch; /* Puffer fuer default-Spruch */
		char *server_name;
		int override;

		/* connect/disconnect-Knopf erzeugen; zuerst mit "Disconnect"
		   beschriften, weil das mehr Platz braucht */
		verbindung_aufbauen_knopf = xv_create(hauptpanel, PANEL_BUTTON,
			PANEL_LABEL_STRING, "Disconnect",
			0);

		/* pause/resume-Knopf erzeugen; zuerst mit "Resume"
		   beschriften, weil das mehr Platz braucht */
		pause_umschalten_knopf = xv_create(hauptpanel, PANEL_BUTTON,
			PANEL_LABEL_STRING, "Resume",
			0);

		server_name = get_server_name(&override);
		if (!override)
			server_name = defaults_get_string("iMaze.server",
				"IMaze.Server", server_name);

		/* Eingabefeld fuer Servername erzeugen */
		server_text = xv_create(hauptpanel, PANEL_TEXT,
			PANEL_NEXT_ROW, -1,
			PANEL_VALUE_DISPLAY_LENGTH, 32,
			PANEL_VALUE_STORED_LENGTH, EINGABE_TEXT_LAENGE,
			PANEL_LABEL_STRING, "  Server:",
			PANEL_VALUE, server_name,
			0);

		/* aus dem Benutzernamen einen default-Spruch bauen */
		if ((default_spruch = benutzer_name()) == NULL)
		{
			speicher_belegen((void **)&default_spruch, 10);
			strcpy(default_spruch, "Gotcha!");
		}
		else
		{
			speicher_vergroessern((void **)&default_spruch,
				strlen(default_spruch) + 20);
			strcat(default_spruch, " shouts: Gotcha!");
		}

		/* Eingabefeld fuer Spruch erzeugen */
		spruch_text = xv_create(hauptpanel, PANEL_TEXT,
			PANEL_NEXT_ROW, -1,
			PANEL_VALUE_DISPLAY_LENGTH, 32,
			PANEL_VALUE_STORED_LENGTH, EINGABE_TEXT_LAENGE,
			PANEL_LABEL_STRING, "  Message:",
			PANEL_VALUE, defaults_get_string("iMaze.message",
				"IMaze.Message", default_spruch),
			0);

		/* Speicher fuer default-Spruch wieder freigeben */
		speicher_freigeben((void **)&default_spruch);

		/* Breite der laengsten Beschriftung feststellen */
		max_label_breite = 0;
		PANEL_EACH_ITEM(hauptpanel, item)
			if ((item == server_text || item == spruch_text) &&
				xv_get(item, PANEL_LABEL_WIDTH) > max_label_breite)
				max_label_breite = xv_get(item, PANEL_LABEL_WIDTH);
		PANEL_END_EACH

		/* Beschriftungen rechtsbuendig anordnen */
		PANEL_EACH_ITEM(hauptpanel, item)
			if (item == server_text || item == spruch_text)
				xv_set(item,
					XV_X, max_label_breite -
						xv_get(item, PANEL_LABEL_WIDTH),
					0);
		PANEL_END_EACH

		/* Checkbox fuer den Kamera-Modus */
		camera_mode_checkbox = xv_create(hauptpanel, PANEL_CHECK_BOX,
			PANEL_NEXT_ROW, -1,
			PANEL_LABEL_STRING, "",
			PANEL_CHOICE_STRING, 0, "Camera mode",
			XV_X, max_label_breite,
			0);

		/* Fenstergroesse anpassen */
		window_fit(hauptpanel);
		window_fit(hauptframe);

		/* Knopf mit "Connect" beschriften und aktivieren */
		xv_set(verbindung_aufbauen_knopf,
			PANEL_LABEL_STRING, "Connect",
			/* auszufuehrende Funktion: */
			PANEL_NOTIFY_PROC, connect_knopf,
			0);

		/* Knopf mit "Pause" beschriften und aktivieren */
		xv_set(pause_umschalten_knopf,
			PANEL_LABEL_STRING, "Pause",
			/* auszufuehrende Funktion: */
			PANEL_NOTIFY_PROC, pause_knopf,
			0);
	}

	/* audio-visuelle Untermalung von Ereignissen initialisieren */
	if (xv_ereignisse_init(pause_handler, hauptframe, hauptpanel))
		return 1;

	/* Initialisierung der Eingabe-Abfrage */
	if (xv_eingabe_init(hauptframe, hauptpanel))
		return 1;

	/* bei der Initialisierung ist kein Fehler aufgetreten */
	return 0;
}


/*
** grafik_ende
**  gibt Datenstrukturen frei
*/
void grafik_ende(void)
{
	/* Eingabe-Routine beenden */
	xv_eingabe_ende();

	/* audio-visuelle Untermalung von Ereignissen beenden */
	xv_ereignisse_ende();

	/* falls es noch einen Haupframe gibt */
	if (hauptframe_gueltig)
	{
		/* Verbindung zum Server beenden, falls noetig */
		ende_ausloesen();

		/* Standard-xview-Funktion fuer Programmende */
		xv_destroy_safe(hauptframe);
	}
}


/*
** disconnect_abfrage
**  macht eine Sicherheitsabfrage und beendet bei Zustimmung die Verbindung
*/
void disconnect_abfrage(void)
{
	static char *meldung[] = { "Do you really want to disconnect?", NULL };

	if (!rueckfrage(meldung, "Sure", "Play on"))
		return;

	/* zur Zeit noch Programmende, spaeter nur disconnect */
	ende_ausloesen();
}


/*
** ueblen_fehler_anzeigen
**  oeffnet eine Hinweisbox mit einer Fehlermeldung und wartet auf Druecken
**  eines Knopfes
**
** Parameter:
**  meldung: Feld von Strings, die die Zeilen des auszugebenden Textes
**           beinhalten
**  knopf: Text der auf dem Knopf stehen soll;
**         falls NULL, so wird ein Standardtext verwendet
*/
void ueblen_fehler_anzeigen(char **meldung, char *knopf)
{
	int schliessen;

	schliessen = 0;

	/* xview schon aktiv? */
	if (hauptframe_gueltig)
	{
		/* mit Server synchronisieren */
		xv_set(hauptframe,
			SERVER_SYNC_AND_PROCESS_EVENTS,
			0);

		if (verbindung_aktiv)
		{
			schliessen = 1;

			/* falls es einen Spiel-Deskriptor gibt,
			   Handler loeschen */
			if (network_proc_active &&
				network_proc_descriptor >= 0)
				notify_set_input_func(hauptframe, NULL,
					network_proc_descriptor);
		}
	}

	/* bei ueblem Fehler kann sowieso nicht mehr kommuniziert werden */
	verbindung_aktiv = 0;

	/* Aufruf eines Notice;
	   falls Texte fuer Knopf=NULL, Standardaufschrift einsetzen */
	if (notice_oeffnen(attr_create_list(
		NOTICE_MESSAGE_STRINGS_ARRAY_PTR, meldung,
		NOTICE_BUTTON_YES, knopf == NULL ? "Bad Luck" : knopf,
		0)) == NOTICE_FAILED)
	{
		int i; /* Zaehlvariable fuer Ausgeben der Meldung */

		/* falls Oeffnen des Notice-Fensters fehlgeschlagen,
		   Ausgabe der Meldung auf standard-error */
		for (i = 0; meldung[i] != NULL; i++)
			fprintf(stderr, "iMaze: %s\n", meldung[i]);
	}

	if (schliessen)
		/* Handler ist schon weg; Fenster schliessen */
		stop_network_proc();
}


/*
** rueckfrage
**  oeffnet eine Abfragebox mit einer Rueckfrage und zwei Knoepfen zur
**  Auswahl
**
** Parameter:
**  meldung: Feld von Strings, die die Zeilen des auszugebenden Textes
**           beinhalten
**  weiter_knopf: Text der auf dem positiv-Knopf stehen soll (default);
**                falls NULL, so wird ein Standardtext verwendet
**  abbruch_knopf: Text der auf dem negativ-Knopf stehen soll;
**                 falls NULL, so wird ein Standardtext verwendet
**
**  Rueckgabewert:
**   true, falls positiv-Knopf gewaehlt
**   false, falls negativ-Knopf gewaehlt
*/
int rueckfrage(char **meldung, char *weiter_knopf, char *abbruch_knopf)
{
	int wahl; /* Nummer des gewaehlten Knopfes */

	/* Erzeugen eines Notice-Fensters;
	   falls Texte fuer Knoepfe = NULL, Standardaufschrift einsetzen */
	wahl = notice_oeffnen(attr_create_list(
		NOTICE_MESSAGE_STRINGS_ARRAY_PTR, meldung,
		NOTICE_BUTTON_YES, weiter_knopf == NULL ?
			"Continue" : weiter_knopf,
		NOTICE_BUTTON_NO, abbruch_knopf == NULL ?
			"Quit" : abbruch_knopf,
		0));

	/* Oeffnen des Notice-Fensters fehlgeschlagen? */
	if (wahl == NOTICE_FAILED)
	{
		static char *meldung[] = { "iMaze - Fatal Error", "",
			"Couldn't open notice window.", NULL };

		uebler_fehler(meldung, NULL);
	}

	/* Rueckgabe: positiv-Knopf = true; negativ-Knopf = false */
	return wahl == NOTICE_YES;
}
