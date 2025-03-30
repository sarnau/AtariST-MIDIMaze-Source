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
** Datei: init_spieler.c
**
** Kommentar:
**  Schickt einem Client die Initialisierung
*/


#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>

#include "argv.h"
#include "global.h"
#include "speicher.h"
#include "labyrinth.h"
#include "protokoll.h"
#include "server_netz.h"
#include "init_spieler.h"

static char sccsid[] = "@(#)init_spieler.c	3.9 12/3/01";


#define PROGRAMM_NAME "iMaze server JC/HUK 1.4" /* Version des Servers */


/* bis hier lokaler Teil                       */
/***********************************************/
/* ab hier globaler Teil                       */


/*
** init_spieler
**  initialisiert einen Client (Prolog), hauptsaechlich werden das
**  Spielfeld gesendet und die Programmversionen ausgetauscht
**
** Parameter:
**  verbindung: Deskriptor fuer die Prolog-Verbindung zum Client
**  feldbreite: Breite des Spielfeldes
**  feldlaenge: Laenge des Spielfeldes
**  spielfeld: das Spielfeld
**  session_info: Infostring, der die (Default-)Session beschreibt
**  progname: Zeiger auf Name des Clientprogramms
**  spruch: Zeiger auf Spruch des Spielers
**  kamera: Zeiger auf Flag fuer Kameramodus
**
** Rueckgabewert:
**  Init_Success, Init_Failed, Init_Aborted_By_Client
**
** Seiteneffekte:
**  *progname, *spruch und *kamera werden gesetzt
*/
init_e init_spieler(void *verbindung, u_int feldbreite, u_int feldlaenge,
	block **spielfeld, char *session_info, char **progname, char **spruch,
	int *kamera)
{
	/* Flags: */
	int programmname_uebertragen; /* der Server hat sich bereits
	                                 identifiziert */
	int spiel_beginnt;            /* PT_SPIEL_BEGINNT gesendet */
	int leer_erlaubt;             /* der Client darf PT_LEER senden */

	/* initialisieren */
	*progname = NULL;
	*spruch = NULL;
	*kamera = 0;

	/* Programmname wurde noch nicht uebertragen, PT_SPIEL_BEGINNT nicht
	   gesendet, der Client darf zu Anfang PT_LEER senden */
	programmname_uebertragen = 0;
	spiel_beginnt = 0;
	leer_erlaubt = 1;

	/* Schleife bis PT_SPIEL_BEGINNT gesendet ist */
	while (!spiel_beginnt)
	{
		int paket_laenge; /* Laenge des Pakets */
		int typ;          /* Typ des zu sendenden Pakets */
		int ignoriert;    /* das aktuelle Paket wird ignoriert (z.B. nicht
		                     verstanden) */
		u_char *paket;    /* zu sendendes oder empfangenes Paket */
		u_char kennung;   /* Kennung aus typ und ignoriert */

		/* ein Paket empfangen (der Client sendet das erste Paket) */
		if (prolog_paket_empfangen(verbindung, &paket_laenge,
			(void **)&paket))
			return Init_Failed;

		/* das Paket darf nicht leer sein, sonst ist das
		   ein Protokoll-Fehler */
		if (!paket_laenge)
			return Init_Failed;

		/* falls nichts mehr zum Senden ansteht, PT_LEER senden */
		typ = PT_LEER;

		/* das Paket gilt erstmal als nicht verstanden */
		ignoriert = PAKET_IGNORIERT;

		/* Paket vom Client auswerten */
		switch (PROL_PAKET_TYP(paket[0]))
		{
			case PT_LEER: /* leeres Paket */
				/* wenn kein PT_LEER vom Client haette kommen duerfen,
				   Verbindung abbrechen */
				if (!leer_erlaubt)
				{
					typ = PT_ABBRUCH;
					break;
				}

				/* keine Anfrage, also muss nicht geantwortet werden */
				break;

			case PT_KAMERA: /* Spieler will nur zusehen */

				ignoriert = PAKET_NICHT_IGNORIERT;
				*kamera = 1;
				break;

			case PT_PROGRAMM_NAME: /* Name des Clientprogramms */
				/* wenn bereits empfangen, ignorieren */
				if (*progname != NULL)
					break;

				ignoriert = PAKET_NICHT_IGNORIERT;

				/* wenn Laenge des Namen 0,
				   dann auch ignorieren */
				if (paket_laenge < 2)
					break;

				/* Speicher fuer Namen belegen und kopieren */
				speicher_belegen((void **)progname,
					paket_laenge);
				memcpy(*progname, paket + 1, paket_laenge - 1);
				(*progname)[paket_laenge - 1] = 0;
				break;

			case PT_SPIELER_SPRUCH: /* Spruch des Spielers */
				/* wenn bereits empfangen, ignorieren */
				if (*spruch != NULL)
					break;

				ignoriert = PAKET_NICHT_IGNORIERT;

				/* wenn Laenge des Spruchs 0, dann auch ignorieren */
				if (paket_laenge < 2)
					break;

				/* Speicher fuer Spruch belegen und kopieren */
				speicher_belegen((void **)spruch, paket_laenge);
				memcpy(*spruch, paket + 1, paket_laenge - 1);
				(*spruch)[paket_laenge - 1] = 0;
				break;

			case PT_WILL_LABYRINTH: /* Client fordert Labyrinth an */
				/* sofort antworten */
				typ = PT_LABYRINTH;
				ignoriert = PAKET_NICHT_IGNORIERT;
				break;

			case PT_WILL_STATUS: /* Client fordert Status an */
				/* sofort antworten */
				typ = PT_STATUS;
				ignoriert = PAKET_NICHT_IGNORIERT;
				break;

			case PT_BEREIT_FUER_SPIEL: /* Client bereit fuer Spiel */
				/* wenn keine Daten mehr zum Senden anstehen,
				   auf das Spiel schalten */
				typ = PT_SPIEL_BEGINNT;
				ignoriert = PAKET_NICHT_IGNORIERT;
				break;

			case PT_ABBRUCH: /* Abbruch durch den Client */
				/* Initialisierung abgebrochen */
				return Init_Aborted_By_Client;
		}

		/* Paket vom Client wird nicht mehr benoetigt */
		speicher_freigeben((void **)&paket);

		/* falls keine Daten anliegen, die sofort uebertragen werden
		   muessen, und der Programmname noch nicht uebertragen
		   wurde, das jetzt tun */
		if ((typ == PT_LEER || typ == PT_SPIEL_BEGINNT) &&
			!programmname_uebertragen)
			typ = PT_PROGRAMM_NAME;

		/* Kennung fuer das Antwort-Paket erzeugen */
		kennung = PROL_PAKET_KENNUNG(ignoriert, typ);

		/* je nach Typ das Antwort-Paket zusammenstellen */
		switch (typ)
		{
			case PT_LABYRINTH: /* Spielfeld senden */
				/* Laenge des Pakets berechnen und Speicher anfordern */
				paket_laenge = 3 + 4 * feldbreite * feldlaenge;
				speicher_belegen((void **)&paket, paket_laenge);

				/* das Paket beginnt immer mit der Kennnung; hier
				   folgen die Dimensionen des Spielfelds */
				paket[0] = kennung;
				paket[1] = feldbreite;
				paket[2] = feldlaenge;

				{
					int x, y, i;   /* Indizes fuer das Spielfeld */
					u_char *daten; /* Zeiger in das Paket */

					/* Inhalt des Spielfeldes nach den Daten fester
					   Laenge anfuegen */
					daten = &paket[3];

					/* den Inhalt des Spielfeldes zeilenweise in das
					   Paket kopieren */
					for (y = 0; y < feldlaenge; y++)
						for (x = 0; x < feldbreite; x++)
							/* die Waende in allen 4 Himmelsrichtungen */
							for (i = 0; i < 4; i++)
								/* nur die Farbe der Wand/Tuer
								   uebertragen */
								*daten++ = spielfeld[x][y][i].farbe;
				}

				/* Paket senden */
				if (prolog_paket_senden(verbindung, paket_laenge, paket))
					/* bei Fehler Initialisierung abbrechen */
					return Init_Failed;

				/* gesandtes Paket wird nicht mehr benoetigt */
				speicher_freigeben((void **)&paket);

				break;

			case PT_STATUS: /* Status senden */
				/* Laenge des Pakets berechnen und
				   Speicher anfordern */
				paket_laenge = 1 + strlen(session_info);
				speicher_belegen((void **)&paket, paket_laenge);

				/* Paket bauen */
				paket[0] = kennung;
				memcpy(paket + 1, session_info,
					paket_laenge - 1);

				/* Paket senden */
				if (prolog_paket_senden(verbindung,
					paket_laenge, paket))
					/* bei Fehler Initialisierung
					   abbrechen */
					return Init_Failed;

				/* gesandtes Paket wird nicht mehr benoetigt */
				speicher_freigeben((void **)&paket);

				break;

			case PT_ABBRUCH: /* Initialisierung abbrechen */
				/* Paket senden, enthaelt nur die Kennung */
				prolog_paket_senden(verbindung, 1, &kennung);

				/* Initialisierung ist fehlgeschlagen */
				return Init_Failed;

			case PT_PROGRAMM_NAME: /* Programmname senden */
				/* Laenge des Pakets berechnen und Speicher anfordern */
				paket_laenge = 1 + strlen(PROGRAMM_NAME);
				speicher_belegen((void **)&paket, paket_laenge);

				/* das Paket beginnt immer mit der Kennnung */
				paket[0] = kennung;

				/* Programmname in das Paket kopieren; ohne
				   abschliessendes Nullbyte */
				memcpy(&paket[1], PROGRAMM_NAME, strlen(PROGRAMM_NAME));

				/* Paket senden */
				if (prolog_paket_senden(verbindung, paket_laenge, paket))
					/* bei Fehler Initialisierung abbrechen */
					return Init_Failed;

				/* gesandtes Paket wird nicht mehr benoetigt */
				speicher_freigeben((void **)&paket);

				/* der Programmname wurde uebertragen */
				programmname_uebertragen = 1;

				break;

			case PT_SPIEL_BEGINNT: /* auf Spiel schalten */
				/* nach dem Senden dieses Pakets ist die Initialisierung
				   beendet */
				spiel_beginnt = 1;

				/* Paket senden: */

			default: /* Paket ohne Daten senden (z.B. PT_LEER) */
				/* Paket senden */
				if (prolog_paket_senden(verbindung, 1, &kennung))
					/* bei Fehler Initialisierung abbrechen */
					return Init_Failed;
		}

		/* PT_LEER vom Client ist nur erlaubt, wenn der Server kein
		   PT_LEER gesendet hat */
		leer_erlaubt = typ != PT_LEER;
	}

	/* Initialisierung ohne Fehler abgeschlossen */
	return Init_Success;
}
