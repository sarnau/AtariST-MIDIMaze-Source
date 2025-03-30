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
** Datei: session.c
**
** Kommentar:
**  Der Session-Prozess; er verwaltet die Adressen und Nummern der Clients,
**  kommuniziert mit den Clients und ruft den Bewegungsteil auf
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "argv.h"
#include "global.h"
#include "speicher.h"
#include "spieler.h"
#include "protokoll.h"
#include "system.h"
#include "server_netz.h"
#include "bewegung.h"
#include "server.h"

static char sccsid[] = "@(#)session.c	3.13 12/05/01";


/* Anzahl der Pakete, die noch ausgewertet werden, selbst wenn sie
   den Server zu spaet erreichen */
#define PAKETANZ 3

/* Zeit in ms bis der Server beim Ausbleiben von Paketen
   vom Client diesen aus dem Spiel nimmt */
/* 1s bis der Spieler suspendiert wird: */
#define ZEIT_BIS_TOT     1000
/* 1min bis die Verbindung ganz abgebrochen wird: */
#define ZEIT_BIS_TIMEOUT (60 * 1000)


/* Makros fuer die Paketnummern: */

/* liest aus einem Paket (Feld von Bytes) ein bestimmtes Byte aus */
#define BYTE_N(paket,n) ((int)((unsigned char *)(paket))[n])

/* liest aus einem Paket vom Client die Nummer des Pakets, das vom Client
   dadurch bestaetigt wird */
#define B_NR(paket) (BYTE_N((paket),0) << 8 | BYTE_N((paket),1))

/* vergleicht zwei Paketnummrn: ist nr1 aelter als nr2? */
#define NR_AELTER(nr1, nr2) ((unsigned short)((unsigned short)(nr1) - \
	(unsigned short)(nr2)) >= 32768)

/* erhoeht die Paketnummer in der Variable nr */
#define NR_ERHOEHEN(nr) ((nr) = (unsigned short)((nr) + 1))


static int aktuelle_paket_nr; /* Nummer des Pakets, das zuletzt zu den
                                 Clients geschickt wurde und auf das jetzt
                                 eine Antwort erwartet wird */
static int client_anz;        /* Anzahl der zur Zeit bekannten Clients */

/* Daten ueber jeden Client: */
static void *client_verbindung[SPIELERANZ]; /* Deskriptor der
                                               Netzwerkverbindung */
static void *client_info[SPIELERANZ];       /* Infozeilen fuer Status */
static int spieler_index[SPIELERANZ];       /* Nummer (Farbe) */
static int letzte_b_nr[SPIELERANZ];         /* Nummer des letzten vom Client
                                               bestaetigten Pakets */
static int client_verschollen[SPIELERANZ];  /* Client hat seit ZEIT_BIS_TOT
                                               Takten nicht mehr
                                               geantwortet */
static int client_ende[SPIELERANZ];         /* Client hat Ende ausgeloest */

/* Pakete, die in einer Runde von den Clients angekommen sind: */
static int paket_anz[SPIELERANZ];               /* Anzahl */
static int paket_laengen[SPIELERANZ][PAKETANZ]; /* Laengen */
static void *pakete[SPIELERANZ][PAKETANZ];      /* Inhalte */


static int vergleich_nummern(const void *n1, const void *n2)
{
	int i1, i2;

	i1 = *(int *)n1;
	i2 = *(int *)n2;

	return spieler_index[i1] - spieler_index[i2];
}


static void status_an_master(void)
{
	int spieler_nummern[SPIELERANZ];
	int i, laenge;
	char *daten;

	laenge = 1; /* fuer den Typ */
	for (i = 0; i < client_anz; i++)
	{
		spieler_nummern[i] = i;
		laenge += strlen(client_info[i]);
	}

	if (client_anz > 1)
		qsort(spieler_nummern, client_anz, sizeof *spieler_nummern,
			vergleich_nummern);

	/* einen Platz lassen fuer ein Null-Byte (faellt danach wieder weg) */
	speicher_belegen((void **)&daten, laenge + 1);
	daten[0] = NT_STATUS;

	daten[1] = 0; /* fuer strcat */
	for (i = 0; i < client_anz; i++)
		strcat(daten + 1, client_info[spieler_nummern[i]]);

	/* Status an den Server-Prozess senden */
	nachricht_senden(NULL, NULL, laenge, daten);
}


static char *client_info_bauen(void *verbindung, int spieler_index,
	char *progname, char *spruch, int kamera)
{
	char puffer[1024], *verb_name, *daten;

	/* eigentlich ueberfluessig */
	memset(puffer, 0, sizeof puffer);


	/* erste Zeile */

	if (kamera)
		strcpy(puffer, "camera");
	else
		sprintf(puffer, "color %d", spieler_index + 1);

	strcat(puffer, " at ");

	verb_name = name_der_verbindung(verbindung);
	strncat(puffer, verb_name, 60);
	speicher_freigeben((void **)&verb_name);

	strcat(puffer, "\n");


	/* zweite Zeile */

	if (progname != NULL)
	{
		strcat(puffer, "  ");
		strncat(puffer, progname, 75);
		strcat(puffer, "\n");
	}


	/* dritte Zeile */

	if (spruch != NULL)
	{
		strcat(puffer, "  ");
		strncat(puffer, spruch, 75);
		strcat(puffer, "\n");
	}


	/* und kopieren */
	speicher_belegen((void **)&daten, strlen(puffer) + 1);
	strcpy(daten, puffer);

	return daten;
}


/*
** bewegung_und_antworten
**  meldet dem Bewegungsteil den Beginn einer neuen Runde und ob in dieser
**  Runde Spieler wegen fehlender Reaktion des Clients suspendiert
**  werden; loest die Bewegung der Spieler und der Schuesse aus;
**  laesst den Bewegungsteil die Informationen fuer die Clients
**  zusammenstellen und versendet diese
**
** Seiteneffekte:
**  Speicher fuer **pakete wird freigegeben, client_ende gesetzt,
**  aktuelle_paket_nummer erhoeht
*/
static void bewegung_und_antworten(void)
{
	int i, j; /* Indizes fuer Clients und Pakete */
	int neue_paket_laengen[SPIELERANZ]; /* Laengen der Pakete an
	                                       die Clients */
	void *neue_pakete[SPIELERANZ];      /* Pakete an die Clients */

	/* dem Bewegungsteil die neue Runde melden */
	init_runde();

	/* verschollene Spieler suspendieren */
	for (i = 0; i < client_anz; i++)
		if (client_verschollen[i])
			spieler_suspendieren(spieler_index[i]);

	/* client_ende initialisieren */
	for (i = 0; i < client_anz; i++)
		client_ende[i] = 0;

	/* alle Pakete zur Auswertung an den Bewegungsteil uebergeben und
	   freigeben; immer reihum von jedem Client ein Paket auswerten */
	for (j = 0; j < PAKETANZ; j++)
		for (i = 0; i < client_anz; i++)
			if (j < paket_anz[i])
			{
				/* Paket ohne Paketnummer an Bewegungsteil uebergeben;
				   client_ende wird evtl. gesetzt */
				bewegung(spieler_index[i], paket_laengen[i][j] - 2,
					(void *)((char *)pakete[i][j] + 2), &client_ende[i]);

				/* Paket freigebenn */
				speicher_freigeben(&pakete[i][j]);
			}

	/* alle Schuesse bewegen */
	schuss_bewegung();

	/* vom Bewegungsteil die Daten fuer die Clients zusammenstellen
	   lassen */
	for (i = 0; i < client_anz; i++)
		paket_bauen(spieler_index[i], &neue_paket_laengen[i],
			&neue_pakete[i], client_ende[i], aktuelle_paket_nr);

	/* Paketnummer erhoehen */
	NR_ERHOEHEN(aktuelle_paket_nr);

	/* die Pakete an alle Clients schicken */
	for (i = 0; i < client_anz; i++)
	{
		void *paket; /* temporaere Kopie des Pakets */

		/* Paket kopieren */
		speicher_belegen(&paket, neue_paket_laengen[i] + 2);
		if (neue_paket_laengen[i])
			memcpy((char *)paket + 2, neue_pakete[i],
				neue_paket_laengen[i]);

		/* Paketnummer an den Anfang des Pakets schreiben */
		((u_char *)paket)[0] = aktuelle_paket_nr / 256;
		((u_char *)paket)[1] = aktuelle_paket_nr % 256;

		/* gesamtes Paket senden */
		spiel_paket_senden(client_verbindung[i],
			neue_paket_laengen[i] + 2, paket);

		/* Speicher wieder freigeben */
		speicher_freigeben(&paket);
		if (neue_paket_laengen[i])
			speicher_freigeben(&neue_pakete[i]);
	}
}


/*
** pakete_empfangen
**  empfaengt alle Pakete, die von den Clients angekommen sind
**
** Seiteneffekte:
**  paket_anz und pakete werden gesetzt
*/
static void pakete_empfangen(void)
{
	int i; /* Index fuer die Clients */

	/* Anzahl der empfangenen Pakete initialisieren */
	for (i = 0; i < client_anz; i++)
		paket_anz[i] = 0;

	/* Schleife durchlaufen, bis keine Pakete mehr anliegen */
	for (;;)
	{
		int paket_laenge; /* Laenge des Pakets */
		void *paket;      /* das Paket */
		int b_nr;         /* Nummer des Pakets, das durch
		                     das angekommene bestaetigt wird */
		int j; /* Index fuer die Pakete eines Clients */

		/* Paket empfangen, zugehoerigen Clientindex in i merken */
		i = spiel_paket_empfangen(client_anz, client_verbindung,
			&paket_laenge, &paket);

		/* es ist kein weiteres Paket zu empfangen */
		if (i < 0)
			/* Routine verlassen */
			break;

		/* enthaelt das Paket nicht einmal die Nummer des dadurch
		   bestaetigten Pakets, ignorieren */
		if (paket_laenge < 2)
		{
			speicher_freigeben(&paket);
			continue;
		}

		/* Bestaetigungsnummer feststellen */
		b_nr = B_NR(paket);

		/* ist die Bestaetigungsnummer neuer als die aktuelle
		   Paketnummer, Paket ignorieren */
		if (NR_AELTER(aktuelle_paket_nr, b_nr))
		{
			speicher_freigeben(&paket);
			continue;
		}

		/* ist die Bestaetigungsnummer aelter oder gleich der
		   letzten Bestaetigungsnummer, Paket ignorieren */
		if (!NR_AELTER(letzte_b_nr[i], b_nr))
		{
			speicher_freigeben(&paket);
			continue;
		}

		/* Position in der Liste der Pakete finden, an der
		   dieses einsortiert werden muss */
		for (j = 0; j < paket_anz[i] &&
			NR_AELTER(B_NR(pakete[i][j]), b_nr); j++);

		/* gibt es ein Paket mit dieser Bestaetigungsnummer schon,
		   Paket ignorieren */
		if (j < paket_anz[i] && b_nr == B_NR(pakete[i][j]))
		{
			speicher_freigeben(&paket);
			continue;
		}

		/* ist die Liste der Pakete noch nicht voll, dieses
		   einsortieren */
		if (paket_anz[i] < PAKETANZ)
		{
			int k; /* Index in die Liste der Pakete */

			/* Pakete verschieben, um an der richtigen Stelle Platz
			   fuer das neue zu schaffen */
			for (k = paket_anz[i]; k > j; k--)
			{
				paket_laengen[i][k] = paket_laengen[i][k - 1];
				pakete[i][k] = pakete[i][k - 1];
			}

			/* Paketanzahl erhoehen */
			paket_anz[i]++;
		}
		/* ist die Liste voll und das neue Paket aelter als alle
		   anderen, Paket ignorieren */
		else if (j == 0)
		{
			speicher_freigeben(&paket);
			continue;
		}
		/* sonst einsortieren */
		else
		{
			int k; /* Index in die Liste der Pakete */

			/* das aelteste Paket freigeben */
			speicher_freigeben(&pakete[i][0]);

			/* dadurch veschiebt sich der Index */
			j--;

			/* Pakete verschieben, um an der richtigen Stelle Platz
			   fuer das neue zu schaffen */
			for (k = 0; k < j; k++)
			{
				paket_laengen[i][k] = paket_laengen[i][k + 1];
				pakete[i][k] = pakete[i][k + 1];
			}
		}

		/* das neue Paket an der jetzt freien Stelle in der Liste
		   einfuegen, Paket dabei kopieren */
		paket_laengen[i][j] = paket_laenge;
		speicher_belegen(&pakete[i][j], paket_laenge);
		memcpy(pakete[i][j], paket, paket_laenge);

		/* den urspruenglichen Speicher wieder freigeben */
		speicher_freigeben(&paket);
	}
}


/*
** client_eintragen
**  haengt einen neuen Client an die Liste der aktiven Clients an und
**  meldet dem Bewegungsteil das Hinzukommen des neuen Clients sowie
**  die Nummer (Farbe) des Clients
**
** Parameter:
**  verbindung: der Deskriptor fuer die Netzwerkverbindung zum Client
**  daten_laenge: Laenge des Clientname und des Spielerspruchs
**  daten: der Clientname und der Spielerspruch
**  kamera: Flag, ob der Spieler nur zusehen will
**
** Rueckgabewert:
**  0, falls der Client eingetragen wurde;
**  1, falls die Liste bereits voll war oder ein interner Fehler auftrat
**
** Seiteneffekte:
**  client_anz wird erhoeht, Eintrag in letzte_b_nr, spieler_index,
**  client_verbindung, client_ende und paket_anz wird initialisiert
*/
static int client_eintragen(void *verbindung, int daten_laenge, char *daten,
	int kamera)
{
	int i; /* Index fuer die Nummer (Farbe) */
	int j; /* Index fuer die Clients */
	int progname_laenge, spruch_laenge;
	char *progname, *spruch; /* Puffer fuer Programmname und Spruch */

	/* Laenge des Programmnamen extrahieren */
	if (daten_laenge < 1)
		return 1; /* ungueltig */
	progname_laenge = ((unsigned char *)daten)[0];
	daten++;
	daten_laenge--;
	if (daten_laenge < progname_laenge)
		return 1; /* ungueltig */

	/* der Rest ist der Spruch */
	spruch_laenge = daten_laenge - progname_laenge;

	/* Liste voll? */
	if (client_anz >= SPIELERANZ)
		return 1;

	/* freie Nummer suchen */
	for (i = 0;; i++)
	{
		/* ist diese Nummer frei? */
		for (j = 0; j < client_anz && spieler_index[j] != i; j++);
		if (j >= client_anz)
			break;
	}

	/* Eintraege fuer diesen Client initialisieren */
	client_verbindung[client_anz] = verbindung;
	spieler_index[client_anz] = i;
	letzte_b_nr[client_anz] = aktuelle_paket_nr;
	client_ende[client_anz] = 0;
	paket_anz[client_anz] = 0;


	/* wurde Programmname uebergeben? */
	if (progname_laenge)
	{
		/* Speicher fuer Programmname belegen und kopieren */
		speicher_belegen((void **)&progname, progname_laenge);
		memcpy(progname, daten, progname_laenge);
	}
	else
		progname = NULL;

	/* wurde Spruch uebergeben? */
	if (spruch_laenge)
	{
		/* Speicher fuer Spruch belegen und kopieren */
		speicher_belegen((void **)&spruch, spruch_laenge);
		memcpy(spruch, daten + progname_laenge, spruch_laenge);
	}
	else
		spruch = NULL;

	client_info[client_anz] = client_info_bauen(verbindung, i,
		progname, spruch, kamera);

	/* Programmname wird nicht mehr gebraucht */
	if (progname != NULL)
		speicher_freigeben((void **)&progname);

	/* dem Bewegungsteil den neuen Spieler melden */
	spieler_start(spieler_index[client_anz], spruch_laenge, spruch, kamera);

	/* Clientanzahl erhoehen */
	client_anz++;

	status_an_master();

	/* Client ist eingetragen */
	return 0;
}


/*
** client_austragen
**  meldet dem Bewegungsteil das Ausscheiden eines Clients und
**  streicht ihn aus der Liste der aktiven Clients
**
** Parameter:
**  client_nr: Index des Clients in der Liste
**
** Seiteneffekte:
**  client_anz wird vermindert, Inhalt von letzte_b_nr, spieler_index,
**  client_verbindung und client_ende wird verschoben
*/
static void client_austragen(int client_nr)
{
	int i; /* Index fuer die Clients */

	/* die Netzwerkverbindung freigeben */
	verbindung_freigeben(client_verbindung[client_nr]);

	speicher_freigeben((void **)&client_info[client_nr]);

	/* dem Bewegungsteil das Ausscheiden des Clients melden */
	spieler_ende(spieler_index[client_nr]);

	/* Clientanzahl anpassen */
	client_anz--;

	/* Listen nach vorne verschieben */
	for (i = client_nr; i < client_anz; i++)
	{
		letzte_b_nr[i] = letzte_b_nr[i + 1];
		client_info[i] = client_info[i + 1];
		spieler_index[i] = spieler_index[i + 1];
		client_verbindung[i] = client_verbindung[i + 1];
		client_ende[i] = client_ende[i + 1];
	}

	status_an_master();
}


/*
** spiel_ende
**  meldet allen Clients das Ende des Spiels, schickt eine Bestaetigung
**  zurueck an den Server-Prozess und beendet den Session-Prozess
**
** Seiteneffekte:
**  Ende des Session-Prozesses
*/
static void spiel_ende(void)
{
	u_char paket[3]; /* Paket fuer die Clients */
	int i;           /* Index fuer die Clients */
	char daten;      /* Puffer fuer die NT_SPIEL_ENDE-Nachricht */

	/* Paketnummer erhoehen */
	NR_ERHOEHEN(aktuelle_paket_nr);

	/* Paket enthaelt nur die Paketnummer und die Ende-Meldung */
	paket[0] = aktuelle_paket_nr / 256;
	paket[1] = aktuelle_paket_nr % 256;
	paket[2] = BK_ENDE_DURCH_SERVER;

	/* Paket an alle Clients senden */
	for (i = 0; i < client_anz; i++)
		spiel_paket_senden(client_verbindung[i], sizeof paket, paket);

	/* Nachricht initialisieren */
	daten = NT_SPIEL_ENDE;

	/* dem Server-Prozess das Ende des Session-Prozesses mitteilen */
	nachricht_senden(NULL, NULL, sizeof daten, &daten);

	/* den Session-Prozess beenden */
	exit(0);
}


/* bis hier lokaler Teil                       */
/***********************************************/
/* ab hier globaler Teil                       */


/*
** paket_bestaetigt
**  stellt fest, ob ein Paket mit Spielinformationen durch den Client
**  bestaetigt wurde
**
** Parameter:
**  spieler_nr: Nummer (Farbe) des Spielers
**  paket_nr: Nummer des fraglichen Pakets
**
** Rueckgabewert:
**  1, falls das Paket bestaetigt wurde, sonst 0
*/
int paket_bestaetigt(int spieler_nr, int paket_nr)
{
	int i; /* Index fuer die Clients */

	/* den zugehoerigen Client suchen */
	for (i = 0; i < client_anz; i++)
		if (spieler_index[i] == spieler_nr)
			/* Client gefunden; zurueckgeben, ob das Paket bestaetigt ist */
			return !NR_AELTER(letzte_b_nr[i], paket_nr);

	/* diese Nummer gibt es nicht; Paket als bestaetigt bezeichnen,
	   damit sich im Bewegungsteil nichts aufstaut */
	return 1;
}


/*
** session
**  kommuniziert mit den Clients und synchronisiert die Bewegungen;
**  tauscht mit dem Server-Prozess Nachrichten aus; nimmt neue
**  Clients (nach init_spieler) in die Session auf und erkennt, ob Clients
**  die Session verlassen
**
** Parameter:
**  zeittakt: Takt in ms
**
** Seiteneffekte:
**  diese Prozedur ist eine Endlosschleife und wird nur durch Fehler
**  oder Nachrichten per exit verlassen
*/
void session(int zeittakt)
{
	client_anz = 0; /* es sind noch keine Clients bekannt */
	aktuelle_paket_nr = 0; /* Paketnummer initialisieren */

	/* Timer initialisieren */
	timer_stoppen();

	/* Bewegungsteil initialisieren */
	init_bewegung();

	/* Endlosschleife; nur durch Fehler oder NT_SPIEL_BEENDEN verlassen */
	for (;;)
	{
		int i; /* genereller Index fuer die Clients */

		if (client_anz)
			/* falls schon Clients bekannt sind, warten bis der Timer, der
			   zum Beginn der letzten Runde gestartet wurde, ablaeuft */
			timer_abwarten();
		else
			/* auf Nachrichten vom Server-Prozess warten */
			nachricht_erwarten(NULL);

		/* Timer zum Synchronisieren der naechsten Runde starten */
		timer_starten(zeittakt);

		/* Verbindung zu den Clients, die sich abgemeldet haben, beenden */
		for (i = 0; i < client_anz; i++)
			if (client_ende[i])
			{
				char daten; /* Puffer fuer NT_SPIELER_ENDE-Nachricht */

				/* Nachricht initialisieren */
				daten = NT_SPIELER_ENDE;

				/* dem Server-Prozess das Ende der Verbindung melden */
				nachricht_senden(NULL, client_verbindung[i],
					sizeof daten, &daten);

				/* den Client aus den Listen austragen */
				client_austragen(i);

				/* Schleifenzaehler korrigieren */
				i--;
				continue;
			}

		/* Nachrichten vom Server-Prozess empfangen und auswerten */
		for (;;)
		{
			char typ;         /* Typ der Nachricht */
			void *verbindung; /* Verbindung, auf die sich die Nachricht
			                     bezieht */
			char *daten;      /* Zeiger auf den Datenteil der Nachricht */
			int laenge;       /* Laenge des Datenteils */

			/* eine Nachricht empfangen */
			if (nachricht_empfangen(NULL, &verbindung,
				&laenge, (void **)&daten))

				/* zur Zeit liegt keine Nachricht an */
				break;

			/* enthaelt der Datenteil nicht einmal den Typ? */
			if (laenge < 1)

				/* dann ignorieren */
				continue;

			typ = *daten; /* Typ der Nachricht feststellen */

			/* Session-Prozess beenden? */
			if (typ == NT_SPIEL_BEENDEN)
			{
				/* Speicher fuer Datenteil freigeben */
				speicher_freigeben((void **)&daten);

				/* Prozess beenden */
				spiel_ende();
			}

			/* falls neuer Client, eintragen */
			if (typ == NT_NEUER_SPIELER && verbindung != NULL &&
				!client_eintragen(verbindung,
					laenge - 1, daten + 1, 0))
			{
				/* Speicher fuer Datenteil freigeben */
				speicher_freigeben((void **)&daten);

				/* erfolgreich eingetragen, Verbindung nicht wieder
				   freigeben */
				continue;
			}

			/* falls neuer Client, eintragen */
			if (typ == NT_NEUE_KAMERA && verbindung != NULL &&
				!client_eintragen(verbindung,
					laenge - 1, daten + 1, 1))
			{
				/* Speicher fuer Datenteil freigeben */
				speicher_freigeben((void **)&daten);

				/* erfolgreich eingetragen, Verbindung nicht wieder
				   freigeben */
				continue;
			}

			/* Speicher fuer Datenteil freigeben */
			speicher_freigeben((void **)&daten);

			/* falls in der Nachricht eine Netzwerkverbindung
			   enthalten war, diese wieder freigeben */
			if (verbindung != NULL)
				verbindung_freigeben(verbindung);
		}

		/* Pakete von den Clients empfangen */
		pakete_empfangen();

		/* fuer alle Clients letzte_b_nr aktualisieren */
		for (i = 0; i < client_anz; i++)
			if (paket_anz[i])
				letzte_b_nr[i] = B_NR(pakete[i][paket_anz[i] - 1]);

		/* Timeouts fuer alle Clients ueberpruefen */
		for (i = 0; i < client_anz; i++)
		{
			/* erstmal annehmen, dass der Spieler nicht suspendiert
			   werden muss */
			client_verschollen[i] = 0;

			/* falls die Pakete schon um ZEIT_BIS_TIMEOUT zurueckliegen
			   und diesmal kein Paket ankam, Verbindung zum Client
			   abbrechen */
			if (NR_AELTER(letzte_b_nr[i] + ZEIT_BIS_TIMEOUT / zeittakt,
				aktuelle_paket_nr) && paket_anz[i] == 0)
			{
				char daten; /* Puffer fuer die NT_SPIELER_TIMEOUT-Nachricht */

				/* Nachricht initialisieren */
				daten = NT_SPIELER_TIMEOUT;

				/* dem Server-Prozess den Abbruch melden */
				nachricht_senden(NULL, client_verbindung[i],
					sizeof daten, &daten);

				/* den Client aus den Listen austragen */
				client_austragen(i);

				/* Schleifenzaehler korrigieren */
				i--;
				continue;
			}

			/* falls die Pakete um ZEIT_BIS_TOT zurueckliegen,
			   den Spieler suspendieren */
			if (NR_AELTER(letzte_b_nr[i] + ZEIT_BIS_TOT / zeittakt,
				aktuelle_paket_nr))
				client_verschollen[i] = 1;
		}

		/* Paketinhalte auswerten, Bewegungen ausfuehren und Antworten
		   an die Clients senden */
		bewegung_und_antworten();
	}
}
