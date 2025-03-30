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
** Datei: netzwerk.c
**
** Kommentar:
**  wertet die Pakete aus, die ueber das Netz vom Server kommen und stellt
**  die Antworten zusammen. Dieser Teil ist von der Art des Netzwerks
**  unabhaengig. Das Senden und Empfangen auf unterster Ebene uebernimmt
**  ein anderer Programmteil.
*/


#include <stdio.h>
#include <string.h>

#include "argv.h"
#include "global.h"
#include "fehler.h"
#include "speicher.h"
#include "labyrinth.h"
#include "spieler.h"
#include "signale.h"
#include "ereignisse.h"
#include "protokoll.h"
#include "netzwerk.h"
#include "spiel.h"
#include "system.h"

static char sccsid[] = "@(#)netzwerk.c	3.12 12/3/01";


/* Zeit in sec, nach der die Verbindung zum Server hart beendet wird, auch
   ohne, dass dieser ein Verlassen des Spiels bestaetigt hat */
#ifndef SPIEL_VERLASSEN_TIMEOUT
#define SPIEL_VERLASSEN_TIMEOUT 3
#endif

#define ERG_WAR_DA 1 /* Bit 0 wird gesetzt, wenn ein Ereignis im vorigen
                        Paket mitgeschickt wurde */
#define ERG_IST_DA 2 /* Bit 1 wird gesetzt, wenn das aktuelle Paket dieses
                        Ereignis enthaelt */

/* Feld mit allen Ereignissen, in dem obige Flags gesetzt werden */
static char ereignis_flags[ERG_ANZ];

/* Feld fuer bisherigen Punktestand; Punkte werden nur inkrementell
   uebertragen; -1 bedeutet: Spieler ist unbekannt */
static int alte_punkte[SPIELERANZ];

/* Flag: soll mit dem naechsten Paket dem Server das Verlassen des Spiels
   mitgeteilt werden?
   0 = Nicht verlassen, 1 = Server das Ende mitteilen,
   2 = Ende durch Server bestaetigt */
static int endesignal_senden = 0;


/*
** spielfeld_kopieren
**  speichert die vom Server uebertragenen Labyrinth-Informationen in die
**  interne Darstellung
**
** Parameter:
**  spielfeld: Zeiger auf das Spielfeld (hier werden die Daten abgelegt)
**  feldbreite: Breite des Spielfeldes
**  feldlaenge: Laenge des Spielfeldes
**  daten: Liste mit den Daten
**
** Seiteneffekte:
**  spielfeld wird gesetzt
*/
static void spielfeld_kopieren(block ***spielfeld, int feldbreite,
	int feldlaenge, u_char *daten)
{
	int x, y; /* Index fuer das Spielfeld */
	int i;    /* Index fuer 4 Himmelsrichtungen */

	/* Feld in passender Groesse anlegen */
	speicher_belegen((void **)spielfeld, feldbreite * sizeof(block *));

	for (x = 0; x < feldbreite; x++)
		speicher_belegen((void **)&(*spielfeld)[x],
			feldlaenge * sizeof(block));

	/* Daten kopieren */
	for (y = 0; y < feldlaenge; y++)
		for (x = 0; x < feldbreite; x++)
			for (i = 0; i < 4; i++)
			{
				struct wand *wand;

				wand = &(*spielfeld)[x][y][i];
				wand->farbe = *daten++;
				/* ob eine Wand als Tuer angezeigt wird, wird an der
				   Farbe entschieden */
				wand->tuer = IST_TUER(wand->farbe);
				wand->tueroffen = 0;
			}
}


/*
** protokoll_fehler
**  ruft die Fehlerbox mit einer einheitlichen Meldung fuer Verletzungen
**  des iMaze-Protokolls auf (dient nur der Uebersichtlichkeit)
**
** Parameter
**  ursache: String mit Text
*/
static void protokoll_fehler(char *ursache)
{
	/* Stringfeld mit dem Meldungstext */
	static char *meldung[] = { "iMaze - Protocol Error", "", NULL, NULL };
	u_char kennung; /* Kennung fuer Mitteilung an den Server */

	/* Mitteilung an den Server schicken; Fehler ignorieren */
	kennung = PROL_PAKET_KENNUNG(PAKET_IGNORIERT, PT_ABBRUCH);
	prolog_paket_senden(1, &kennung);

	/* Fehlermeldung ausgeben */
	meldung[2] = ursache;
	uebler_fehler(meldung, NULL);
}


/* bis hier lokaler Teil                       */
/***********************************************/
/* ab hier globaler Teil                       */


/*
** spielparameter_empfangen
**  tauscht mit dem Server einmalig zu uebertragene Informationen ueber
**  den Aufbau der Spielrunde und das Spielfeld aus
**
** Parameter:
**  programmname: String zur Identifikation des eigenen Programms
**  spielername: Name des Spielers
**  spielerspruch: Spruch, der abgeschossenen Gegnern angezeigt wird
**  kamera: Flag, ob der Spieler nur zusehen will
**  feldbreite: Zeiger auf die Breite des Spielfeldes
**  feldlaenge: Zeiger auf die Laenge des Spielfeldes
**  spielfeld: Zeiger auf das Spielfeld
**  server_status: Zeiger auf die Server-Statusmeldung
**  wirklich_starten: Flag, ob gespielt oder nur der Status
**                    abgefragt werden soll
**
** Rueckgabewert:
**  1 fuer Fehler, sonst 0
**
** Seiteneffekte:
**  *feldbreite, *feldlaenge, *spielfeld und server_status werden gefuellt,
**  falls nicht NULL; ereignis_flags und alte_punkte werden initialisiert
*/
int spielparameter_empfangen(char *programmname, char *spielername,
	char *spielerspruch, int kamera, u_int *feldbreite, u_int *feldlaenge,
	block ***spielfeld, char **server_status, int wirklich_starten)
{
	/* Flags: */
	int labyrinth_fehlt;           /* der Client hat das Spielfeld
	                                  noch nicht erhalten */
	int status_fehlt;              /* der Client hat den Status
	                                  noch nicht erhalten */
	int programmname_uebertragen;  /* der Client hat sich bereits
	                                  beim Server identifiziert */
	int spielername_uebertragen;   /* der Client hat den Spielernamen
	                                  uebertragen */
	int spielerspruch_uebertragen; /* der Client hat den Spruch uebertragen */
	int kamera_uebertragen;        /* der Client hat den Kamera-Modus
	                                  angefordert */
	int ignoriert;                 /* das aktuelle Paket wird vom Client
	                                  ignoriert (z.B. nicht verstanden) */

	{
		int i;

		/* ereignis_flags initialisieren */
		for (i = 0; i < ERG_ANZ; i++)
			ereignis_flags[i] = 0;

		for (i = 0; i < SPIELERANZ; i++)
			alte_punkte[i] = -1;
	}

	/* falls feldlaenge oder feldbreite = NULL, soll kein Labyrinth gelesen
	   werden */
	labyrinth_fehlt = feldbreite != NULL || feldlaenge != NULL;

	/* falls server_status = NULL, soll er nicht gelesen werden */
	status_fehlt = server_status != NULL;

	/* Spielfeld wurde noch nicht empfangen */
	if (labyrinth_fehlt)
		*spielfeld = NULL;

	/* Status wurde noch nicht empfangen */
	if (server_status != NULL)
		*server_status = NULL;

	/* Identifikations-Text wird nicht uebertragen, wenn NULL */
	programmname_uebertragen = programmname == NULL;

	/* Spielername wird nicht uebertragen, wenn NULL */
	spielername_uebertragen = spielername == NULL;

	/* Spielerspruch wird nicht uebertragen, wenn NULL */
	spielerspruch_uebertragen = spielerspruch == NULL;

	/* wenn nicht Kamera-Modus, dann "bereits uebertragen" annehmen */
	kamera_uebertragen = !kamera;

	/* der Client sendet zuerst, er kann also zuerst kein
	   Paket ignorieren */
	ignoriert = PAKET_NICHT_IGNORIERT;

	/* Schleife, bis alle Parameter-Pakete ausgetauscht sind */
	for (;;)
	{
		int abbruch;      /* Flag: es ist ein Fehler aufgetreten;
		                     Prolog abbrechen */
		int bereit;       /* Flag: der Client hat aus seiner Sicht alle
		                     notwendigen Pakete ausgetauscht und ist bereit
		                     fuer den Beginn des Spiels */
		int paket_laenge; /* Laenge eines empfangenen Pakets */
		u_char kennung;   /* Paket-Kennung des gesendeten Pakets*/
		u_char *paket;    /* Liste mit einem Paket */
		int status_unterwegs; /* Status wurde gerade angefordert */

		/* der Client ist bereit fuer den Beginn des Spiel, wenn er sich
		   beim Server identifiziert und alle Daten erhalten hat */
		bereit = !labyrinth_fehlt && !status_fehlt &&
			programmname_uebertragen && spielername_uebertragen &&
			spielerspruch_uebertragen;

		/* der Client sendet zuerst ein Paket */

		/* Flags initialisieren */
		abbruch = 0;
		status_unterwegs = 0;

		if (bereit && !wirklich_starten)
		{
			/* fertig, abbrechen */
			kennung = PROL_PAKET_KENNUNG(ignoriert, PT_ABBRUCH);
			if (prolog_paket_senden(1, &kennung))
				abbruch = 1;
			else
				/* kein Fehler */
				return 0;
		}
		else if (bereit)
		{
			/* dem Server die Bereitschaft mitteilen */
			kennung = PROL_PAKET_KENNUNG(ignoriert, PT_BEREIT_FUER_SPIEL);
			if (prolog_paket_senden(1, &kennung))
				abbruch = 1;
		}
		else if (!programmname_uebertragen)
		{
			/* falls der Identifikationstext noch nicht uebertragen wurde,
			   wird er in einem Paket an den Server geschickt */
			speicher_belegen((void **)&paket, 1 + strlen(programmname));
			paket[0] = kennung =
				PROL_PAKET_KENNUNG(ignoriert, PT_PROGRAMM_NAME);
			memcpy(&paket[1], programmname, strlen(programmname));
			if (prolog_paket_senden(1 + strlen(programmname), paket))
				abbruch = 1;
			speicher_freigeben((void **)&paket);

			/* das Flag setzen */
			programmname_uebertragen = 1;
		}
		else if (!spielername_uebertragen)
		{
			/* falls der Spielername noch nicht uebertragen wurde,
			   wird er in einem Paket an den Server geschickt */
			speicher_belegen((void **)&paket, 1 + strlen(spielername));
			paket[0] = kennung =
				PROL_PAKET_KENNUNG(ignoriert, PT_SPIELER_NAME);
			memcpy(&paket[1], spielername, strlen(spielername));
			if (prolog_paket_senden(1 + strlen(spielername), paket))
				abbruch = 1;
			speicher_freigeben((void **)&paket);

			/* das Flag setzen */
			spielername_uebertragen = 1;
		}
		else if (!spielerspruch_uebertragen)
		{
			/* falls der Spruch noch nicht uebertragen wurde,
			   wird er in einem Paket an den Server geschickt */
			speicher_belegen((void **)&paket, 1 + strlen(spielerspruch));
			paket[0] = kennung =
				PROL_PAKET_KENNUNG(ignoriert, PT_SPIELER_SPRUCH);
			memcpy(&paket[1], spielerspruch, strlen(spielerspruch));
			if (prolog_paket_senden(1 + strlen(spielerspruch), paket))
				abbruch = 1;
			speicher_freigeben((void **)&paket);

			/* das Flag setzen */
			spielerspruch_uebertragen = 1;
		}
		else if (!kamera_uebertragen)
		{
			/* falls der Kamera-Modus noch nicht uebertragen wurde,
			   aber gewuenschst ist, wird er in einem Paket an
			   den Server geschickt */

			kennung = PROL_PAKET_KENNUNG(ignoriert, PT_KAMERA);

			if (prolog_paket_senden(1, &kennung))
				abbruch = 1;

			/* das Flag setzen */
			kamera_uebertragen = 1;
		}
		else if (labyrinth_fehlt)
		{
			/* falls Spielfeld noch nicht erhalten, vom Server anfordern */
			kennung = PROL_PAKET_KENNUNG(ignoriert, PT_WILL_LABYRINTH);
			if (prolog_paket_senden(1, &kennung))
				abbruch = 1;
		}
		else
		{
			/* falls Status noch nicht erhalten, vom Server anfordern */
			status_unterwegs = 1;
			kennung = PROL_PAKET_KENNUNG(ignoriert, PT_WILL_STATUS);
			if (prolog_paket_senden(1, &kennung))
				abbruch = 1;
		}

		/* Prolog wegen Fehler abbrechen? */
		if (abbruch)
			break;

		/* jetzt wird ein Paket vom Server empfangen */

		if (prolog_paket_empfangen(&paket_laenge, (void **)&paket))
		{
			/* Speicher wieder freigeben */
			if (paket_laenge)
				speicher_freigeben((void **)&paket);

			/* Abbruch wegen Fehler */
			break;
		}

		/* das Server-Paket ist vorerst ungekannt */
		ignoriert = PAKET_IGNORIERT;

		/* das Paket muss mindestens ein Zeichen umfassen, sonst ist das
		   ein Protokoll-Fehler */
		if (!paket_laenge)
			protokoll_fehler("Empty prolog packet received.");

		if (status_unterwegs && PROL_PAKET_IGNORIERT(paket[0]))
			/* Server kennt keine Status-Anfragen */
			status_fehlt = 0;

		/* das erste Zeichen enthaelt die Paket-Kennung; von ihr haengt die
		   weitere Behandlung ab */
		switch (PROL_PAKET_TYP(paket[0]))
		{
			case PT_LEER: /* leeres Paket */
				break;

			case PT_ABBRUCH: /* Abbruch durch den Server */
				{
					/* hier wird eine Fehlerbox ausgegeben und das Programm
					   beendet */
					static char *meldung[] = { "iMaze - Fatal Error", "",
						"Aborted by server.", NULL };

					kennung = PROL_PAKET_KENNUNG(PAKET_IGNORIERT,
						PT_ABBRUCH);
					prolog_paket_senden(1, &kennung);

					uebler_fehler(meldung, NULL);
				}

			case PT_SPIEL_BEGINNT: /* der Server beginnt das Spiel */
				/* hat der Client noch nicht alle Information, wird mit
				   einem Fehler abgebrochen (nach diesem Paket kann auf
				   dieser Protokollebene nicht weiteruebertragen werden) */
				if (!bereit)
					protokoll_fehler(
						"Game started by server without consent.");

				/* sonst ist dieser Protokollteil beendet */
				speicher_freigeben((void **)&paket);

				/* kein Fehler */
				return 0;

			case PT_LABYRINTH: /* der Server schickt das Spielfeld */
				/* wenn feldbreite und feldlaenge = NULL, ist das Spielfeld
				   gar nicht erwuenscht */
				if (feldbreite == NULL && feldlaenge == NULL)
					break;
				/* wenn das Paket nicht einmal die Groessen enthaelt, ist
				   es nicht zu gebrauchen */
				if (paket_laenge < 3)
					break;

				/* feldlaenge und feldbreite auslesen, falls gewuenscht */
				if (feldbreite != NULL)
				{
					*feldbreite = paket[1];
					if (*feldbreite == 0)
						break;
				}
				if (feldlaenge != NULL)
				{
					*feldlaenge = paket[2];
					if (*feldlaenge == 0)
						break;
				}

				/* ist ein Spielfeld nicht erwuenscht, wird abgebrochen */
				if (feldlaenge == NULL || feldbreite == NULL ||
					spielfeld == NULL)
				{
					labyrinth_fehlt = 0;
					ignoriert = PAKET_NICHT_IGNORIERT;
					break;
				}
				/* stimmt die Laenge des Paketes?
				   (kennung + feldbreite + feldlaenge + feldbreite  *
				   feldlaenge * 4 Waende) */
				if (paket_laenge < 4 * *feldbreite * *feldlaenge + 3)
					break;

				/* Flag setzen und Spielfeld in Datenstrukturen kopieren */
				labyrinth_fehlt = 0;
				ignoriert = PAKET_NICHT_IGNORIERT;
				spielfeld_kopieren(spielfeld, *feldbreite, *feldlaenge,
					&paket[3]);
				break;

			case PT_STATUS: /* der Server schickt den Status */
				/* wenn server_status = NULL, ist
				   kein Status erwuenscht */
				if (server_status == NULL)
					break;

				/* ist der Status schon angekommen? */
				if (*server_status != NULL)
					break;

				/* Status (mit abschliessender Null) kopieren */
				speicher_belegen((void **)server_status,
					paket_laenge);

				memcpy(*server_status, paket + 1,
					paket_laenge - 1);
				(*server_status)[paket_laenge - 1] = 0;

				/* Erfolg */
				status_fehlt = 0;
				ignoriert = PAKET_NICHT_IGNORIERT;
				break;
		}

		speicher_freigeben((void **)&paket);
	}

	/* Prolog wurde wegen Fehler abgebrochen */

	/* Beenden der Verbindung zum Server */
	verbindung_beenden();

	/* Speicher fuer Spielfeld freigeben */
	if (spielfeld != NULL && *spielfeld != NULL)
	{
		int i; /* Index */

		for (i = 0; i < *feldbreite; i++)
			speicher_freigeben((void **)&(*spielfeld)[i]);

		speicher_freigeben((void **)spielfeld);
	}

	/* Speicher fuer Status freigeben */
	if (server_status != NULL && *server_status != NULL)
		speicher_freigeben((void **)server_status);

	/* wegen Fehler beendet */
	return 1;
}


/*
** spiel_verlassen
**  beendet die Verbindung zum Server; falls der Server nicht mehr
**  ordnungsgemaess antwortet, wird nach Ablauf einer Zeit beendet
**
** Seiteneffekt:
**  endesignal_senden wird gesetzt
*/
void spiel_verlassen(void)
{
	void *alter_zustand; /* Zwischenspeicher fuer Timer-Zustand */

	alter_zustand = timer_retten();
	/* Zeit fuer maximale Wartezeit setzen */
	timer_starten(SPIEL_VERLASSEN_TIMEOUT * 1000);

	/* in einer Schleife pruefen, ob Bestaetigung durch Server eingetroffen
	   oder Wartezeit abgelaufen */
	for (endesignal_senden = 1; endesignal_senden == 1;)
	{
		int restzeit; /* Timer-Restzeit */

		/* Timer abgelaufen? */
		if ((restzeit = timer_restzeit()) == 0)
			/* dann fertig */
			break;

		/* auf Paket warten */
		if (spiel_paket_erwarten(restzeit))
			/* Fehler? dann fertig */
			break;
	}

	/* Timer zuruecksetzen */
	timer_restaurieren(alter_zustand);

	/* kein Endesignal mehr senden */
	endesignal_senden = 0;

	/* Beenden der Verbindung zum Server */
	verbindung_beenden();
}


/*
** spiel_paket_angekommen
**  wird aufgerufen, wenn ein Paket vom Server ankommt, und
**  wertet dieses aus
**
** Parameter:
**  paket_laenge: Laenge des Pakets
**  paket: Liste mit den Daten
**
** Rueckgabewert:
**  1 fuer Fehler, sonst 0
**
** Seiteneffekte:
**  ereignis_flags und endesignal_senden werden veraendert
*/
int spiel_paket_angekommen(int paket_laenge, void *paket)
{
	u_char *daten;      /* Liste mit den Daten eines Pakets */
	int i;
	int *spieler_aktiv; /* Zeiger auf Flag: ist der Spieler selbst am
	                       Leben? */
	int *schuss_aktiv;  /* Zeiger auf Flag: hat der Spieler gerade einen
	                       Schuss im Spiel? */
	int *gegneranz;     /* Zeiger auf die Anzahl der aktuell sichtbaren
	                       Mitspieler */
	int *schussanz;     /* Zeiger auf die Anzahl der Schuesse der
	                       Mitspieler */
	int *abgeschossen_durch; /* Zeiger auf die Farbe des Spielers, von dem
	                            man ggf. abgeschossen wurde */
	char **abschuss_spruch;  /* Zeiger auf String mit dem Spruch des Gegners,
	                            von dem man abgeschossen wurde */
	char *ereignisse;        /* Liste von Ereignissen */
	struct spieler *spieler; /* Zeiger auf Daten ueber den Spieler selbst */
	struct spieler *gegner;  /* Liste mit den Daten der Mitspieler */
	struct schuss *schuss;   /* Zeiger auf Daten ueber den eigenen Schuss */
	struct schuss *schuesse; /* Liste mit den Daten der Gegner-Schuesse */
	int *punkte;             /* Liste mit den Punktestaenden */

	/* im Feld ereignis_flags fuer jedes Ereignis das Bit fuer "Ereignis
	   wurde mituebertragen" loeschen */
	for (i = 0; i < ERG_ANZ; i++)
		ereignis_flags[i] &= ~ERG_IST_DA;

	/* falls dem Server der Ausstieg aus der Spielrunde mitgeteilt werden
	   soll, sind alle Spiel-Daten uninteressant */
	if (endesignal_senden)
	{
		spieler_aktiv = gegneranz = schuss_aktiv = schussanz = NULL;
		spieler = gegner = NULL;
		schuss = schuesse = NULL;
		abgeschossen_durch = NULL;
		abschuss_spruch = NULL;
		ereignisse = NULL;
		punkte = NULL;
	}
	else
	/* andernfalls werden die Pufferspeicher fuer die Spieldaten zur
	   Verfuegung gestellt */
		spiel_puffer_anlegen(&spieler_aktiv, &spieler, &gegneranz, &gegner,
			&schuss_aktiv, &schuss, &schussanz, &schuesse,
			&abgeschossen_durch, &abschuss_spruch, &ereignisse, &punkte);

	/* die Spiel-Daten, die gewuenscht sind, werden auf sinnvolle Werte
	   initialisiert */
	if (spieler_aktiv != NULL)
		*spieler_aktiv = 0;
	if (schuss_aktiv != NULL)
		*schuss_aktiv = 0;
	if (gegneranz != NULL)
		*gegneranz = 0;
	if (schussanz != NULL)
		*schussanz = 0;
	if (abgeschossen_durch != NULL)
		*abgeschossen_durch = -1;
	if (abschuss_spruch != NULL)
		*abschuss_spruch = NULL;
	if (ereignisse != NULL)
		for (i = 0; i < ERG_ANZ; i++)
			ereignisse[i] = 0;

	daten = paket;

	/* Scheife, in der das Paket der Reihe nach ausgewertet wird */
	while (paket_laenge > 0)
	{
		u_char kennung; /* Paket-Kennung des Unterbereichs im Gesamtpaket */
		int laenge;	    /* Laenge der Daten in den Unterpaketen */

		/* Zeichen 1 ist jeweils die Paketkennung */
		kennung = *daten++;
		paket_laenge--;

		/* hat das Paket Daten angehaengt oder besteht es nur aus der
		   Kennung? */
		if (DATENLAENGE_VARIABEL(kennung))
		{
			/* enthaelt das Gesamtpaket noch Daten? */
			if (!paket_laenge)
				break;

			/* Laenge des Unterpakets auslesen */
			laenge = *daten++;
			paket_laenge--;
		}
		else
			/* keine Laenge, falls Unterpaket nur aus der Kennung besteht */
			laenge = 0;

		/* enthaelt das Gesamtpaket soviel Daten, wie angegeben? */
		if (paket_laenge < laenge)
			break;

		/* Auswerten des Unterpakets */
		switch (kennung)
		{
			case BK_ENDE_DURCH_SERVER: /* der Server beendet die Verbindung
			                              von sich aus */
			case BK_ENDE_DURCH_CLIENT: /* der Server beendet die Verbindung
				                          auf Wunsch des Clients */

				/* falls der Timeout laeuft, im Flag vermerken, dass die
				   Bestaetigung von Server vorliegt; dann wird die Verbindung
				   aus der Funktion 'spiel_verlassen' geschlossen */
				if (endesignal_senden)
					endesignal_senden = 2;
				else
				{
					static char *meldung[] = { "iMaze - Network Error", "",
						"Disconnected by server.", NULL };

					milder_fehler(meldung, NULL);
					verbindung_beenden();

					return 1;
				}
				break;

			case BK_SPIELER_POS: /* Daten ueber den eigenen Spieler */

				/* erwartete Laenge pruefen */
				if (laenge != 6)
					break;

				/* Information unerwuenscht? */
				if (spieler_aktiv == NULL)
					break;

				/* der Spieler ist lebendig */
				*spieler_aktiv = 1;

				/* Daten unerwuenscht? */
				if (spieler == NULL)
					break;

				/* Daten in Struktur schreiben */
				spieler->pos.xgrob = daten[0];
				spieler->pos.xfein = daten[1];
				spieler->pos.ygrob = daten[2];
				spieler->pos.yfein = daten[3];
				spieler->farbe = daten[4];
				spieler->blick = daten[5];
				break;

			case BK_GEGNER_POS: /* Daten ueber die Mitspieler */

				/* Daten nicht erwuenscht? */
				if (gegneranz == NULL)
					break;

				/* erwartete Laenge pruefen (je 6 Zeichen pro Spieler
				   und weniger als die maximal zulaessige Spieleranzahl) */
				if (!laenge || laenge / 6 * 6 != laenge ||
					*gegneranz + laenge / 6 > SPIELERANZ)
					break;

				/* falls nur Anzahl, aber nicht Position der Mitspieler
				   erwuenscht */
				if (gegner == NULL)
				{
					*gegneranz += laenge / 6;
					break;
				}

				/* alle Mitspieler in den Datenstrukturen ablegen */
				for (i = 0; i < laenge / 6; i++)
				{
					int blick;

					gegner[*gegneranz].pos.xgrob = daten[6 * i];
					gegner[*gegneranz].pos.xfein = daten[6 * i + 1];
					gegner[*gegneranz].pos.ygrob = daten[6 * i + 2];
					gegner[*gegneranz].pos.yfein = daten[6 * i + 3];
					gegner[*gegneranz].farbe = daten[6 * i + 4];
					blick = daten[6 * i + 5];
					if (blick >= WINKANZ)
						blick = -1;
					gegner[*gegneranz].blick = blick;
					(*gegneranz)++;
				}
				break;

			case BK_SPIELER_SCHUSS_POS: /* Daten ueber eigenen Schuss */

				/* erwartete Laenge pruefen */
				if (laenge != 5)
					break;

				/* Information erwuenscht? */
				if (schuss_aktiv == NULL)
					break;

				/* ein Schuss ist unterwegs */
				*schuss_aktiv = 1;

				/* Daten erwuenscht? */
				if (schuss == NULL)
					break;

				/* alle Daten in die Struktur uebernehmen */
				schuss->pos.xgrob = daten[0];
				schuss->pos.xfein = daten[1];
				schuss->pos.ygrob = daten[2];
				schuss->pos.yfein = daten[3];
				schuss->farbe = daten[4];
				break;

			case BK_GEGNER_SCHUSS_POS: /* Daten ueber die Schuesse der
			                              Mitspieler */

				/* Information erwuenscht? */
				if (schussanz == NULL)
					break;

				/* erwartete Laenge pruefen (je 5 Zeichen pro Schuss
				   und weniger als die maximal zulaessige Spieleranzahl) */
				if (!laenge || laenge / 5 * 5 != laenge ||
					*schussanz + laenge / 5 > SPIELERANZ)
					break;

				/* falls nur Anzahl, aber nicht Position der Schuesse
				   erwuenscht */
				if (schuesse == NULL)
				{
					*schussanz += laenge / 5;
					break;
				}

				/* alle Schuesse in den Datenstrukturen ablegen */
				for (i = 0; i < laenge / 5; i++)
				{
					schuesse[*schussanz].pos.xgrob = daten[5 * i];
					schuesse[*schussanz].pos.xfein = daten[5 * i + 1];
					schuesse[*schussanz].pos.ygrob = daten[5 * i + 2];
					schuesse[*schussanz].pos.yfein = daten[5 * i + 3];
					schuesse[*schussanz].farbe = daten[5 * i + 4];
					(*schussanz)++;
				}
				break;

			case BK_ABGESCHOSSEN_DURCH: /* Informationen ueber den Schuetzen */

				/* Laenge pruefen */
				if (!laenge)
					break;

				/* Farbe des Schuetzen zuweisen, falls erwuenscht */
				if (abgeschossen_durch != NULL)
					*abgeschossen_durch = daten[0];

				/* Abbruch, falls keine weiteren Daten oder Spruch nicht
				   erwuenscht */
				if (laenge < 1 || abschuss_spruch == NULL)
					break;

				/* den Spruch des Schuetzen in internen String uebernehmen */
				speicher_belegen((void **)abschuss_spruch, laenge);
				memcpy(*abschuss_spruch, &daten[1], laenge - 1);
				(*abschuss_spruch)[laenge - 1] = 0;
				break;

			case BK_EREIGNISSE: /* Liste der aufgetretenen Ereignisse */

				/* Informationen erwuenscht? */
				if (ereignisse == NULL)
					break;

				/* alle Ereignisse in das Feld eintragen */
				for (i = 0; i < laenge; i++)
					if (daten[i] < ERG_ANZ)
					{
						ereignisse[daten[i]] = 1;
						/* doppelte Ereignisse fallen aufeinander */
						ereignis_flags[daten[i]] |= ERG_IST_DA;
					}
				break;

			case BK_ALTE_EREIGNISSE: /* Liste der nachgeschickten Ereignisse
									    (falls im Netz Pakete verloren gehen,
				                        werden die Ereignisse zur Sicherheit
				                        nocheinmal nachgeschickt) */

				/* Informationen erwuenscht? */
				if (ereignisse == NULL)
					break;

				/* alle Ereignisse in das Feld eintragen */
				for (i = 0; i < laenge; i++)
					if (daten[i] < ERG_ANZ)
					{
						/* ist das Ereignis beim letztenmal noch doch richtig
						   uebermittelt worden? */
						if (!(ereignis_flags[daten[i]] & ERG_WAR_DA))
							ereignisse[daten[i]] = 1;
						/* doppelte Ereignisse fallen aufeinander */
						ereignis_flags[daten[i]] |= ERG_IST_DA;
					}
				break;

			case BK_PUNKTESTAND: /* die Punktestaende */

				/* Informationen erwuenscht? */
				if (punkte == NULL)
					break;

				/* alle neuen Punktestaende in das Feld eintragen */
				for (i = 0; i < laenge; i += 2)
				{
					int spieler; /* Nummer des Spielers */
					int punkte;  /* Punkte des Spielers */

					/* spieler und punkte aus Paket auslesen */
					spieler = daten[i] & 0x3f;
					punkte = daten[i] << 2 & 0x300 | daten[i + 1];

					/* Spieler nicht mehr aktiv? */
					if (punkte == 1023)
						punkte = -1;

					/* Spielernummer gueltig? */
					if (spieler >= SPIELERANZ)
						continue;

					/* Punkte merken */
					alte_punkte[spieler] = punkte;
				}
				break;
		}

		/* Gesamtpaket um das jetzt fertig abgearbeitet Unterpaket
		   verkuerzen */
		daten += laenge;
		paket_laenge -= laenge;
	}

	/* Punkte uebergeben? */
	if (punkte != NULL)

		/* kopieren */
		for (i = 0; i < SPIELERANZ; i++)
			punkte[i] = alte_punkte[i];


	/* jetzt: Senden des Antwort-Pakets */


	/* falls dem Server ein Aussteigen aus dem Spiel mitgeteilt werden soll */
	if (endesignal_senden)
	{
		/* noch keine Bestaetigung vom Server eingetroffen */
		if (endesignal_senden == 1)
		{
			u_char antwort; /* Zeichen mit Paketkennung */

			/* Wunsch nach Ausstieg aus dem Spiel senden */
			antwort = BK_ENDE_SIGNAL;
			if (spiel_paket_senden(1, &antwort))
				/* Fehler */
				return 1;
		}
	}
	else
	{
		int signalanz;                 /* Anzahl der ausgeloesten
		                                  Eingabesignale */
		char signale[SIGNALANZ];       /* Feld mit Flags fuer alle Signale */
		u_char antwort[SIGNALANZ + 2]; /* Feld mit den zu uebertragenden
		                                  Signalen */

		/* Signale in Eingabeteil abfragen */
		signale_abfragen(signale);

		/* aus dem Feld mit Flags die Nummer des Signals ermitteln und an
		   Antwort-Paket anhaengen */
		for (i = signalanz = 0; i < SIGNALANZ; i++)
			if (signale[i])
				antwort[signalanz +++ 2] = i; /* Uebung: Was macht das? */

		antwort[0] = BK_SIGNALE; /* Paket-Kennung */
		antwort[1] = signalanz;  /* Zeichen 1 enthaelt die Anzahl der
		                            folgenden Signale */

		/* Paket zum Server senden */
		if (spiel_paket_senden(2 + signalanz, antwort))
			/* Fehler */
			return 1;
	}

	/* in dem Feld der Ereignisse alle neuen Ereignisse als alte Ereignisse
	   fuer naechsten Durchlauf eintragen */
	for (i = 0; i < ERG_ANZ; i++)
		ereignis_flags[i] = ereignis_flags[i] & ERG_IST_DA ? ERG_WAR_DA : 0;

	/* kein Fehler */
	return 0;
}
