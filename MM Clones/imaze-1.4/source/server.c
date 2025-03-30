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
** Datei: server.c
**
** Kommentar:
**  Das Hauptprogramm des Servers
**  inklusive des Startens der anderen Prozesse
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "argv.h"
#include "global.h"
#include "fehler.h"
#include "speicher.h"
#include "system.h"
#include "labyrinth.h"
#include "server_netz.h"
#include "server.h"
#include "init_spieler.h"

static char sccsid[] = "@(#)server.c	3.19 12/04/01";


static int zeittakt_parsen(char *str);


static char *labfile_name = NULL;

struct arg_option argv_opts[] =
{
	{ Arg_Include, NULL, net_opts },
	{ Arg_Include, NULL, feature_opts },
	{ Arg_Callback, "c", (void *)zeittakt_parsen,
		"set cycle time (15-150 ms, default 60)", "time" },
	{ Arg_End, NULL, &labfile_name, NULL, "lab-file" }
};


/* Identifikationstext, mit dem jede Labyrinthdatei beginnt: */
static char MAGIC[] = "iMazeLab1\n";

u_int feldlaenge, feldbreite; /* Feldgroesse */
block **spielfeld;            /* Daten des Spielfeldes */

static void *session_deskriptor; /* Deskriptor zur Kommunikation mit der
                                    einzigen Session, die gestartet wird */
static char *session_status; /* Infostring ueber die einzige Session */

static int zeittakt = 60; /* in Millisekunden */


/*
** zeittakt_parsen
**
** Seiteneffekte:
**  setzt zeittakt
*/
static int zeittakt_parsen(char *str)
{
	long takt;
	char *end;

	takt = strtol(str, &end, 10);
	if (*end != 0 || takt < 15 || takt > 150)
		return 1; /* parse/value error */

	zeittakt = takt;
	return 0; /* ok */
}


/*
** ende_handler
**  wird aufgerufen, wenn der Server vom User beendet wird;
**  schickt dem Session-Prozess die Nachricht; der Server-Prozess wird
**  erst beendet, wenn der Session-Prozess geantwortet hat
**
** Parameter:
**  signum: Dummy-Parameter
*/
static void ende_handler(int signum)
{
	char daten; /* Puffer fuer die Nachricht */

	/* Nachricht NT_SPIEL_BEENDEN an Session-Prozess senden */
	daten = NT_SPIEL_BEENDEN;
	nachricht_senden(session_deskriptor, NULL, sizeof daten, &daten);
}


static void spieler_an_session(void *verbindung, char *progname, char *spruch,
	int kamera)
{
	char *daten; /* Puffer zum Uebertragen des Spruchs
	                an den Sessions-Prozess */
	int progname_laenge, spruch_laenge, daten_laenge;

	if (progname == NULL || progname[0] == 0)
		progname_laenge = 0;
	else
		progname_laenge = strlen(progname) + 1; /* Null mitkopieren */

	/* Laenge begrenzen */
	if (progname_laenge > 255)
		progname_laenge = 255;

	/* Kameras habens keinen Spruch, da sie nicht toeten koennen */
	if (spruch == NULL || spruch[0] == 0 || kamera)
		spruch_laenge = 0;
	else
		spruch_laenge = strlen(spruch) + 1; /* Null mitkopieren */

	/* Speicher fuer den Typ, die Laenge des Programmnamen, den Namen
	   selbst und den Spruch belegen */
	daten_laenge = 2 + progname_laenge + spruch_laenge;
	speicher_belegen((void **)&daten, daten_laenge);

	/* die Nachricht beginnt mit dem Typ und der Laenge des Namen */
	daten[0] = kamera ? NT_NEUE_KAMERA : NT_NEUER_SPIELER;
	daten[1] = progname_laenge;

	/* dann der Programmname */
	if (progname_laenge)
		memcpy(daten + 2, progname, progname_laenge);

	/* und zuletzt der Spruch */
	if (spruch_laenge)
		memcpy(daten + 2 + progname_laenge, spruch, spruch_laenge);

	/* dem Session-Prozess den Verbindungsdeskriptor per Nachricht
	   uebergeben */
	nachricht_senden(session_deskriptor, verbindung, daten_laenge, daten);

	/* allen Speicher wieder freigeben */
	speicher_freigeben((void **)&daten);
	if (progname != NULL)
		speicher_freigeben((void **)&progname);
	if (spruch != NULL)
		speicher_freigeben((void **)&spruch);

	/* der Verbindungsdeskriptor wird vom Server nicht mehr
	   benoetigt, aufraeumen; einen evtl. gestarteten
	   Initialisierungsprozess implizit beenden */
	verbindung_freigeben(verbindung);
}


/*
** server
**  loest die Initialisierung von neuen Clients aus und kommuniziert
**  mit dem Session-Prozess
**
** Seiteneffekte:
**  diese Prozedur ist eine Endlosschleife und wird nur durch Fehler
**  oder Signale per exit verlassen
*/
static void server(void)
{
	/* Status initialisieren */
	speicher_belegen((void **)&session_status, 1);
	session_status[0] = 0;

	/* Endlosschleife, s.o. */
	for (;;)
	{
		/* auf Nachrichten von den anderen Prozessen und neue
		   Verbindungen von Clients warten */
		if (nachricht_oder_verbindung_erwarten(1,
			&session_deskriptor) == NULL)
		/* eine Verbindung steht evtl. bereit */
		{
			void *verbindung; /* Deskriptor fuer die Verbindung zum
			                     neuen Client */
			char *progname;   /* Name des Client-Programms */
			char *spruch;     /* Spruch des neuen Spielers */
			int kamera;       /* Spieler will nur zusehen */
			init_e init_result;

			/* versuchen, die Verbindung entgegenzunehmen */
			if ((verbindung = verbindung_annehmen()) == NULL)
				/* keine Verbindung bekommen oder ein
				   Initialisierungsprozess wurde abgespalten, um die
				   Verbindung anzunehmen */
				continue;

			/* die Verbindung wurde angenommen; evtl. ist dies ein
			   abgespaltener Initialisierungsprozess */

			/* dem Client die Initialisierungsdaten senden
			   und die Verbindung vom Prolog auf das Spiel schalten */
			init_result = init_spieler(verbindung, feldbreite,
				feldlaenge, spielfeld, session_status,
				&progname, &spruch, &kamera);
			if (init_result != Init_Success ||
				verbindung_auf_spiel(session_deskriptor,
					verbindung))
			{
				/* Speicher fuer Programmname und Spruch
				   bei Fehler wieder freigeben */
				if (progname != NULL)
					speicher_freigeben((void **)&progname);

				if (spruch != NULL)
					speicher_freigeben((void **)&spruch);

				/* bei einem Fehler die Verbindung abbrechen und weiter
				   warten; einen evtl. gestarteten Initialisierungsprozess
				   implizit beenden */
				verbindung_abbrechen(verbindung, init_result !=
					Init_Aborted_By_Client /* fehler? */);
				continue;
			}

			/* Spieler an Session uebergeben; einen evtl.
			   gestarteten Initialisierungsprozess implizit
			   beenden */
			spieler_an_session(verbindung, progname,
				spruch, kamera);
		}
		else
		/* eine Nachricht von einer Session liegt evtl. an */
		{
			void *daten;      /* Datenteil der Nachricht */
			int laenge;       /* Laenge des Datenteils */
			void *verbindung; /* Verbindungsdeskriptor, auf den die
			                     Nachricht sich bezieht */
			char typ;         /* Typ der Nachricht; siehe server.h */

			/* Nachricht (Typ und Deskriptor) entgegennehmen */
			if (nachricht_empfangen(session_deskriptor, &verbindung,
				&laenge, &daten))
				/* keine Nachricht bekommen; weiter warten */
				continue;

			/* Session-Status erneuern? */
			if (laenge > 0 && ((char *)daten)[0] == NT_STATUS)
			{
				speicher_freigeben((void **)&session_status);

				/* Typ entfernen, Null anhaengen */
				if (laenge > 1)
					memmove(daten, (char *)daten + 1,
						laenge - 1);

				((char *)daten)[laenge - 1] = 0;

				/* und abspeichern */
				session_status = daten;
				continue;
			}

			/* Datenteil besteht nicht genau aus dem Typ? */
			if (laenge != 1)
			{
				/* dann Speicher wieder freigeben */
				if (laenge)
					speicher_freigeben(&daten);

				/* und Nachricht ignorieren */
				continue;
			}

			typ = *(char *)daten; /* Typ der Nachricht */

			/* Speicher wieder freigeben */
			speicher_freigeben(&daten);

			/* wenn der Session-Prozess sich beendet hat, den
			   Server-Prozess auch beenden */
			if (typ == NT_SPIEL_ENDE)
				exit(0);

			/* alle weiteren Nachrichten ignorieren und einen
			   evtl. mitgesandten Deskriptor wieder freigeben */
			if (verbindung != NULL)
				verbindung_freigeben(verbindung);
		}
	}
}


/*
** aussenwand_korrigieren
**  korrigiert eine Aussenwand des Spielfeldes, falls diese unsichtbar,
**  begehbar oder nicht schusssicher ist
**
** Parameter:
**  wand: Zeiger auf eine Wand im Spielfeld
**
** Rueckgabewert:
**  0, falls nichts veraendert wurde, 1 sonst
**
** Seiteneffekte:
**  *wand wird noetigenfalls korrigiert
*/
static int aussenwand_korrigieren(struct wand *wand)
{
	int veraendert; /* Flag, ob etwas veraendert wurde */

	veraendert = 0;

	/* die Wand muss schusssicher sein */
	if (!wand->schusssicher)
		wand->schusssicher = 1,
		veraendert = 1;

	/* die Wand muss unbegehbar sein */
	if (!wand->unbegehbar)
		wand->unbegehbar = 1,
		veraendert = 1;

	/* die Wand muss die Farbe einer Wand haben */
	if (wand->farbe < 1 || wand->farbe > 7)
		wand->farbe = 1,
		veraendert = 1;

	/* zurueckmelden, ob etwas veraendert wurde */
	return veraendert;
}


/*
** ladelab
**  laedt das Labyrinth aus einer Datei
**
** Parameter:
**  dateiname: der Dateiname
**
** Seiteneffekte:
**  spielfeld, feldbreite und feldlaenge werden initialisiert
*/
static void ladelab(char *dateiname)
{
	FILE *datei;                  /* Deskriptor fuer die Labyrinthdatei */
	char magic[sizeof MAGIC - 1]; /* Identifikationstext in der Datei */
	u_char daten[4];              /* Puffer zum Lesen aus der Datei */
	int x, y, i;                  /* Indizes fuer das Spielfeld */
	int warnung;                  /* Labyrinth enthaelt
	                                 unzulaessige Daten */

	/* falls der Dateiname "-" ist, von stdin lesen, sonst Datei oeffnen */
	if (strcmp(dateiname, "-"))
	{
		/* Datei zum Lesen oeffnen */
		datei = fopen(dateiname, "rb");

		/* bei Fehler, diesen anzeigen und Programm abbrechen */
		if (datei == NULL)
		{
			static char *meldung[] = { "iMaze - Initialization Error", "",
				"Can't read labyrinth file", NULL, NULL };

		lese_fehler:
			/* Fehlermeldung zusammenstellen und ausgeben */
			meldung[3] = dateiname;
			uebler_fehler(meldung, NULL);
		}
	}
	else
		/* stdin benutzen */
		datei = stdin;

	/* Identifikationstext aus der Datei lesen */
	if (fread(magic, 1, sizeof magic, datei) != sizeof magic)
		/* bei Fehler "Initialization Error" anzeigen */
		goto lese_fehler;

	/* Falls der Identifikationstext nicht der erwartete ist,
	   Fehler anzeigen und Programm abbrechen */
	if (memcmp(MAGIC, magic, sizeof magic))
	{
		static char *meldung[] = { "iMaze - Initialization Error", "",
			"Invalid labyrinth file format", NULL, NULL };

	format_fehler:
		/* Fehlermeldung zusammenstellen und ausgeben */
		meldung[3] = dateiname;
		uebler_fehler(meldung, NULL);
	}

	/* Feldgroesse aus der Datei lesen */
	if (fread(daten, 1, 2, datei) != 2)
		/* bei Fehler "Initialization Error" anzeigen */
		goto lese_fehler;
	feldbreite = daten[0];
	feldlaenge = daten[1];

	/* Feldgroesse ueberpruefen */
	if (feldbreite < 1 || feldbreite > MAXFELDBREITE ||
		feldlaenge < 1 || feldlaenge > MAXFELDBREITE)
		/* bei ungueltier Groesse "Initialization Error" anzeigen */
		goto format_fehler;

	/* Speicher fuer das Spielfeld belegen */
	speicher_belegen((void **)&spielfeld, feldbreite * sizeof(block *));

	for (x = 0; x < feldbreite; x++)
		speicher_belegen((void **)&spielfeld[x],
			feldlaenge * sizeof(block));

	/* bei unzulaessigem Spielfeldinhalt Warnung ausgeben,
	   aber weitermachen */
	warnung = 0;

	/* Spielfeld einlesen */
	for (y = 0; y < feldlaenge; y++)
		for (x = 0; x < feldbreite; x++)
		{
			/* einen Block (4 Waende) aus der Datei lesen */
			if (fread(daten, 1, 4, datei) != 4)
				/* bei Fehler "Initialization Error" anzeigen */
				goto lese_fehler;

			/* alle 4 Waende in Spielfeld kopieren */
			for (i = 0; i < 4; i++)
			{
				struct wand *wand; /* Zeiger in das Spielfeld */

				wand = &spielfeld[x][y][i];

				/* Farbe und Flags "schusssicher" und "unbegehbar" aus der
				   Datei uebernehmen */
				wand->farbe = daten[i] & 15;
				wand->schusssicher = daten[i] >> 6 & 1;
				wand->unbegehbar = daten[i] >> 7 & 1;

				/* Daten unzulaessig? */
				warnung |=
					wand->unbegehbar && wand->farbe >= 8 ||
					!wand->unbegehbar && wand->farbe >= 1 &&
					wand->farbe <= 7 ||
					wand->schusssicher ^ (wand->farbe != 0) ||
					(daten[i] >> 4 & 3) != 0;
			}
		}

	/* falls am Rand keine Waende sind, korrigieren */
	for (x = 0; x < feldbreite; x++)
	{
		warnung |= aussenwand_korrigieren(&spielfeld[x][0][NORD]);
		warnung |= aussenwand_korrigieren(&spielfeld[x][feldlaenge - 1][SUED]);
	}
	for (y = 0; y < feldlaenge; y++)
	{
		warnung |= aussenwand_korrigieren(&spielfeld[0][y][WEST]);
		warnung |= aussenwand_korrigieren(&spielfeld[feldbreite - 1][y][OST]);
	}

	/* Datei schliessen, wenn es nicht stdin war */
	if (datei != stdin)
		fclose(datei);

	/* evtl. Warnung ausgeben */
	if (warnung)
	{
		fprintf(stderr, "imazesrv: Invalid labyrinth\n");
		fprintf(stderr, "imazesrv: %s\n", dateiname);
		fprintf(stderr, "imazesrv:\n");
		fprintf(stderr, "imazesrv: Who cares?\n");
	}
}


/* bis hier lokaler Teil                       */
/***********************************************/
/* ab hier globaler Teil                       */


/*
** uebler_fehler
**  zeigt eine Fehlermeldung an und beendet das Programm
**
** Parameter:
**  meldung: Feld von Strings, die die Zeilen des auszugebenden Textes
**           beinhalten
**  knopf: Text der auf einem Bestaetigungs-Knopf stehen sollte;
**         wird ignoriert
**
** Seiteneffekte:
**  Ende des Programms
*/
void uebler_fehler(char **meldung, char *knopf)
{
	int i; /* Zaehlvariable fuer Ausgeben der Meldung */

	/* Ausgabe der Meldung auf standard-error */
	for (i = 0; meldung[i] != NULL; i++)
		fprintf(stderr, "imazesrv: %s\n", meldung[i]);

	/* Programm abbrechen */
	exit(1);
}


/*
** main
**  die Hauptroutine, der Server und eine Session werden gestartet
**
** Parameter:
**  argc: Anzahl der Argumente inklusive Programmname
**  argv: Argumentliste
*/
int main(int argc, char **argv)
{
	/* diese Signale immer ignorieren */
	signal(SIGHUP, SIG_IGN);
	signal(SIGALRM, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);

	/* fuer diese Signale gibt es fuer einen Prozess einen Handler,
	   erstmal ignorieren */
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);

	process_args(&argc, argv);

	/* Labyrinth laden */
	ladelab(labfile_name);

	/* Netzwerkroutinen initialisieren */
	netzwerk_init();

	/* eine Session starten */
	if ((session_deskriptor = session_starten()) != NULL)
	{
		/* dies ist der Server-Prozess */

		/* fuer diese Signale einen Handler installieren, der ein
		   sauberes Programmende erlaubt */
		handle_signal(SIGINT, ende_handler);
		handle_signal(SIGTERM, ende_handler);

		server();
	}
	else
		/* dies ist der Session-Prozess */
		session(zeittakt);

	return 0;
}
