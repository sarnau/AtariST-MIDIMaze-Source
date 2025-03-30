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
** Datei: bewegung.c
**
** Kommentar:
**  Bewegt die Spieler und Schuesse durch das Labyrinth und stellt
**  die Pakete fuer die Clients zusammen
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "argv.h"
#include "global.h"
#include "speicher.h"
#include "labyrinth.h"
#include "spieler.h"
#include "signale.h"
#include "ereignisse.h"
#include "protokoll.h"
#include "bewegung.h"
#include "server.h"

static char sccsid[] = "@(#)bewegung.c	3.25 12/05/01";


/* Schrittweite der Spieler (Default: Bruchteil der Breite eines Feldes) */
#ifndef SCHRITT
#define SCHRITT (RASPT / 10)
#endif

/* Schrittweite der Schuesse (Default: doppelt so schnell wie Spieler) */
#ifndef SCHUSSSCHRITT
#define SCHUSSSCHRITT (2 * SCHRITT)
#endif

/* Zeit, die ein Spieler tot bleibt, wenn er getroffen wurde */
#ifndef SPIELER_TOT_ZEIT
#define SPIELER_TOT_ZEIT 30
#endif

/* erwuenschter Minimalabstand von Gegnern beim Beamen */
#ifndef MIN_GEGNER_NAEHE
#define MIN_GEGNER_NAEHE 5
#endif

/* erwuenschter Minimalabstand von Schuessen beim Beamen */
#ifndef MIN_SCHUSS_NAEHE
#define MIN_SCHUSS_NAEHE 3
#endif


/* Ergebnisse von bewege_schuss: */
#define SCHUSS_WEITER    0 /* Schuss fliegt weiter */
#define SCHUSS_DANEBEN   1 /* Schuss hat nur eine Wand getroffen */
#define SCHUSS_TREFFER   2 /* Schuss hat einen Gegner getroffen */
#define SCHUSS_PRALLT_AB 3 /* Schuss prallt an einer Wand ab */

/* Bahnkorrektur-Typ: */
#define TYP_NICHTS 0 /* keine Korrektur */
#define TYP_GERADE 1 /* Korrekturbahn ist gerade */
#define TYP_KURVE  2 /* Korrekturbahn ist eine Kreisbahn */

#define MAX_PUNKTE 999 /* maximale Punktezahl, die ein Spieler haben kann */

#define SPIELER_NICHT_AKTIV 1023 /* Punktezahl eines Spielers, der
                                    nicht aktiv ist */

/* Flags, ob der Punktestand gesendet werden soll */
#define NICHT_SENDEN 0 /* nicht senden */
#define NACHSENDEN   1 /* senden, falls das urspruengliche
                          Paket nicht bestaetigt wurde */
#define NEU_SENDEN   2 /* auf jeden Fall senden */


/* Struktur, die die Parameter fuer eine gerade Korrekturbahn enthaelt */
struct korrektur_gerade
{
	int winkel;  /* Winkel, in dem die Korrekturbahn verlaeuft
	                (0 = nach Norden) */
	int strecke; /* Laenge der Korrekturbahn */
};

/* Struktur, die die Parameter fuer eine kreisfoermige
   Korrekturbahn enthaelt */
struct korrektur_kurve
{
	int radius;  /* Kreisradius */
	int winkel1; /* Startwinkel auf dem Kreis */
	int winkel2; /* Zielwinkel auf dem Kreis */
};

/* Struktur, die die Parameter fuer eine beliebige Korrekturbahn enthaelt */
struct korrektur
{
	int typ; /* Typ der Korrektur (TYP_NICHTS, TYP_GERADE oder TYP_KURVE) */
	union
	{
		struct korrektur_gerade gerade; /* Parameter fuer gerade Bahn */
		struct korrektur_kurve kurve;   /* Parameter fuer Kreisbahn */
	} daten;
};


/* Positionen, Blickrichtungen und Farben aller Spieler */
/* die Positionen und Farben muessen fuer autoanswer erhalten bleiben */
static struct spieler alle_spieler[SPIELERANZ];

/* Kamera-Flags */
static int ist_kamera[SPIELERANZ];

/* gekaufte Optionen aller Spieler */
static int opt_faceless[SPIELERANZ];
static int opt_colorless[SPIELERANZ];
static int opt_reflective[SPIELERANZ];
static int opt_quickturn[SPIELERANZ];
static int opt_autoanswer[SPIELERANZ];
static int opt_invisible[SPIELERANZ];

/* Vorbelegung der Optionen */
static int default_faceless = 0;
static int default_colorless = 0;
static int default_reflective = 0;
static int default_quickturn = 0;
static int default_autoanswer = 0;
static int default_invisible = 0;


struct arg_option feature_opts[] =
{
	{ Arg_Simple, "F", &default_faceless, "enable faceless players" },
	{ Arg_Simple, "C", &default_colorless, "enable colorless players" },
	{ Arg_Simple, "R", &default_reflective, "enable reflective shots" },
	{ Arg_Simple, "Q", &default_quickturn, "enable quickturn" },
	{ Arg_Simple, "A", &default_autoanswer, "enable autoanswer" },
	{ Arg_Simple, "I", &default_invisible, "enable invisiblity" },
	{ Arg_End }
};


/* Positionen und Farben aller Schuesse */
static struct schuss alle_schuesse[SPIELERANZ];

/* Flugrichtungen der Schuesse */
static int schuss_blick[SPIELERANZ];

/* Flags, welche Schuesse unterwegs sind */
static int schuss_fliegt[SPIELERANZ];

/* Flags, welche Schuesse gestartet werden muessen, */
/* wird in einer Runde immer nur erhoeht, um Wiederholungen zu vermeiden */
static int autoanswer_ausgeloest[SPIELERANZ];

/* Flugrichtungen der zu startenden Schuesse */
static int autoanswer_blick[SPIELERANZ];

/* Flags, welche Ereignisse in dieser Runde aufgetreten sind */
static char ereignisse[SPIELERANZ][ERG_ANZ];

/* Flags, ob ein Ereignis noch nicht vom Client bestaetigt wurde */
static char alte_ereignisse[SPIELERANZ][ERG_ANZ];

/* Paket, in dem zuletzt das Ereignis gesendet wurde */
static int ereignis_paket_nr[SPIELERANZ][ERG_ANZ];

/* Flags, welche Spieler existieren */
static int spieler_aktiv[SPIELERANZ];

/* Zeit, bis ein Spieler wieder in das Spiel kommt, 0 fuer Spieler
   im Spiel, -1 fuer Spieler, die eine Pause einlegen (bis WEITERSIGNAL) */
static int spieler_tot[SPIELERANZ];

/* Nummer des Spielers, von dem ein Spieler abgeschossen wurde,
   -1, wenn er wieder lebendig ist */
static int abgeschossen_durch[SPIELERANZ];

/* zugehoeriger Spruch: */
static int abgeschossen_spruch_laenge[SPIELERANZ]; /* Laenge */
static char *abgeschossen_spruch[SPIELERANZ];      /* Inhalt */

/* Spruch der Spieler: */
static int spieler_spruch_laenge[SPIELERANZ]; /* Laenge */
static char *spieler_spruch[SPIELERANZ];      /* Inhalt */

static int spieler_punkte[SPIELERANZ]; /* Punkte der Spieler */

/* Flags, welche Punktestaende an die Clients gesendet werden muessen */
static int punkte_senden[SPIELERANZ][SPIELERANZ];

/* Paket, in dem zuletzt der Punktestand gesendet wurde */
static int punkte_paket_nr[SPIELERANZ][SPIELERANZ];

/* Feld, in dem gespeichert wird, wie nahe die Gegner und Schuesse sind;
   fuer spieler_beamen; wird jedesmal vor Gebrauch neu berechnet */
static char **gegner_naehe;


/*
** atan
**  berechnet den Arkustangens von x/y unter Beruecksichtigung des
**  Quadranten;
**
** Parameter:
**  x: Zaehler
**  y: Nenner
**
** Rueckgabewert:
**  -1 fuer x=y=0, sonst atan(x/y) als Winkel von 0 bis TRIGANZ; speziell:
**  0         fuer x=0, y=1;  TRIGANZ/4   fuer x=1,  y=0;
**  TRIGANZ/2 fuer x=0, y=-1; TRIGANZ*3/4 fuer x=-1, y=0
*/
static int atan(int x, int y)
{
	int korrektur; /* Korrekturwinkel fuer Quadranten */

	/* 0/0 ist nicht zulaessig */
	if (x == 0 && y == 0)
		return -1;

	/* Korrektur initialisieren */
	korrektur = 0;

	/* Position um TRIGANZ/4 drehen, bis y > abs(x) */

	if (y < x)
	{
		/* (x,y) um TRIGANZ/2 drehen, Korrektur umgekehrt */
		korrektur = TRIGANZ / 2;
		x = -x;
		y = -y;
	}

	if (y < -x)
	{
		int tmp; /* Zwischenspeicher */

		/* (x,y) um TRIGANZ/4 drehen, Korrektur umgekehrt */
		korrektur += 3 * TRIGANZ / 4;
		tmp = x;
		x = -y;
		y = tmp;
	}

	/* atan aus Tabelle auslesen, Korrekturwinkel wieder hinzufuegen */
	return (atantab[x * (TRIGANZ / 2) / y + TRIGANZ / 2] + korrektur) %
		TRIGANZ;
}


/*
** punkt_trifft_wand
**  prueft, ob ein Punkt, der sich von (x,y) in Richtung winkel
**  um die Strecke strecke bewegt, mit einer Wand, die von (0,0) bis
**  (RASPT,0) verlaeuft, kollidiert; berechnet eine Korrekturbahn
**
** Parameter:
**  x: x-Koordinate des Punktes
**  y: y-Koordinate des Punktes
**  winkel: Richtung, in die sich der Punkt bewegt
**  strecke: Strecke, um die sich der Punkt bewegt
**  korrektur: Zeiger auf eine Struktur, in der die Parameter
**             der Korrekturbahn abgelegt werden
**
** Rueckgabewert:
**  -1, falls der Punkt nicht mit der Wand kollidiert;
**  sonst die Laenge der Strecke, die er vorher zuruecklegen kann
**
** Seiteneffekte:
**  *korrektur wird gesetzt
*/
static int punkt_trifft_wand(int x, int y, int winkel, int strecke,
	struct korrektur_gerade *korrektur)
{
	int dx, dy; /* Positionsdifferenz bei einem Schritt um 1,
	               skaliert mit TRIGFAKTOR */

	/* Korrekturstrecke auf Null initialisieren */
	korrektur->strecke = 0;

	/* Korrekturwinkel: westlich entlang der Wand */
	korrektur->winkel = TRIGANZ / 4;

	/* falls sich der Punkt ueberhaupt nicht bewegen soll,
	   keine Kollision */
	if (strecke == 0)
		return -1;

	/* bewegt sich der Punkt nach Sueden oder Osten, die Szene
	   um TRIGANZ/2 drehen */
	if (winkel >= TRIGANZ / 2)
	{
		/* Position des Punktes drehen */
		x = RASPT - x;
		y = -y;

		/* Richtung des Punktes drehen */
		winkel -= TRIGANZ / 2;

		/* Richtung der Korrekturbahn drehen */
		korrektur->winkel += TRIGANZ / 2;
	}

	/* bewegt sich der Punkt nicht nach Nordwesten, sondern nach
	   Suedwesten, die Szene an der x-Achse spiegeln */
	if (winkel > TRIGANZ / 4)
	{
		/* Position des Punktes spiegeln */
		y = -y;

		/* Richtung des Punktes anpassen */
		winkel = TRIGANZ / 2 - winkel;

		/* Richtung der Korrekturbahn anpassen */
		korrektur->winkel = (TRIGANZ / 2 - winkel + TRIGANZ) % TRIGANZ;
	}

	/* bewegt sich der Punkt genau nach Westen, so kann er sich entweder
	   gar nicht bewegen, falls er in der Wand ist, oder bis zur Wand;
	   eine Korrektur ist nicht moeglich: korrektur->strecke bleibt 0 */
	if (winkel == TRIGANZ / 4)
		return x <= 0 || x >= RASPT + strecke ? -1 :
			x >= RASPT ? x - RASPT : 0;

	/* ist der Punkt schon an der Wand vorbei, so kann er sich unbehindert
	   bewegen */
	if (x <= 0 || y <= 0)
		return -1;

	/* Positionsaenderung bei einem Schritt um 1 feststellen */
	dx = sintab[winkel];
	dy = costab[winkel];

	/* ueberschreitet der Punkt nach der gewuenschten Strecke noch
	   nicht die x-Achse, so kann er sich unbehindert bewegen */
	if (strecke * dy <= y * TRIGFAKTOR)
		return -1;

	/* verfehlt der Punkt die Wand auf der x-Achse, so kann er
	   sich unbehindert bewegen */
	if (y * dx >= x * dy || y * dx <= (x - RASPT) * dy)
		return -1;

	/* die Laenge der Korrekturbahn ist der Abstand des Kollisionspunktes
	   vom Ende der Wand */
	korrektur->strecke = x - y * dx / dy;

	/* die Strecke berechnen, um die sich der Punkt ohne Kollision
	   bewegen kann */
	return y * TRIGFAKTOR / dy;
}


/*
** punkt_trifft_wand
**  prueft, ob ein Punkt, der sich von (x,y) in Richtung winkel
**  um die Strecke strecke bewegt, mit einem Kreis mit Mittelpunkt
**  (0,0) und Radius radius kollidiert; berechnet eine Korrekturbahn
**
** Parameter:
**  x: x-Koordinate des Punktes
**  y: y-Koordinate des Punktes
**  winkel: Richtung, in die sich der Punkt bewegt
**  strecke: Strecke, um die sich der Punkt bewegt
**  radius: Radius des Kreises
**  korrektur: Zeiger auf eine Struktur, in der die Parameter
**             der Korrekturbahn abgelegt werden
**
** Rueckgabewert:
**  -1, falls der Punkt nicht mit der Kreis kollidiert;
**  sonst die Laenge der Strecke, die er vorher zuruecklegen kann
**
** Seiteneffekte:
**  *korrektur wird gesetzt
*/
static int punkt_trifft_kreis(int x, int y, int winkel, int strecke,
	int radius, struct korrektur_kurve *korrektur)
{
	int kwinkel;       /* Winkel, um den die Korrekturbahn hinterher
	                      gedreht werden muss */
	int kfaktor;       /* Vorzeichen, mit dem der Winkel der Korrekturbahn
	                      multipliziert werden muss */
	int diskriminante; /* Diskriminante der quadratischen Gleichung, die
	                      die Schnittpunkte der Bahn des Punktes mit dem
	                      Kreisumfang bestimmt */
	int w;             /* Wurzel aus der Diskriminante */
	int mitte;         /* Strecke, die der Punkt zuruecklegen muss, bis
	                      er den Kreis halb durchquert hat */

	/* initialisieren */
	korrektur->radius = 0;
	kfaktor = 1;
	kwinkel = 0;

	/* falls sich der Punkt ueberhaupt nicht bewegen soll,
	   keine Kollision */
	if (strecke == 0)
		return -1;

	/* bewegt sich der Punkt nach Sueden oder Osten, die Szene
	   um TRIGANZ/2 drehen */
	if (winkel >= TRIGANZ / 2)
	{
		/* Position des Punktes drehen */
		x = -x;
		y = -y;

		/* Richtung des Punktes drehen */
		winkel -= TRIGANZ / 2;

		/* spaeter Richtung der Korrekturbahn drehen */
		kwinkel += TRIGANZ / 2;
	}

	/* bewegt sich der Punkt nicht nach Nordwesten, sondern nach
	   Suedwesten, die Szene um -TRIGANZ/4 drehen */
	if (winkel >= TRIGANZ / 4)
	{
		int tmp; /* Zwischenspeicher */

		/* Position des Punktes drehen */
		tmp = x;
		x = -y;
		y = tmp;

		/* Richtung des Punktes drehen */
		winkel -= TRIGANZ / 4;

		/* spaeter Richtung der Korrekturbahn drehen */
		kwinkel += TRIGANZ / 4;
	}

	/* bewegt sich der Punkt nicht nach Nordnordwest, sondern nach
	   Westnordwest, die Szene an der Nordwest-Suedost-Achse spiegeln */
	if (winkel > TRIGANZ / 8)
	{
		int tmp; /* Zwischenspeicher */

		/* Position des Punktes spiegeln */
		tmp = x;
		x = y;
		y = tmp;

		/* Richtung des Punktes anpassen */
		winkel = TRIGANZ / 4 - winkel;

		/* spaeter Richtung der Korrekturbahn anpassen */
		kwinkel = (kwinkel + TRIGANZ / 4) % TRIGANZ + TRIGANZ;
		kfaktor = -1;
	}

	/* verfehlt der Punkt offensichtlich den Kreis (zu weit westlich,
	   noerdlich, nordoestlich oder suedwestlich), fertig */
	if (x <= -radius || y <= -radius ||
		x >= radius + strecke || y >= radius + strecke)
		return -1;

	/* befindet sich der Punkte innerhalb der Kreisflaeche, ist
	   ueberhaupt keine Bewegung mehr moeglich */
	if (sqr(x) + sqr(y) < sqr(radius))
		return 0;

	{
		int tmp; /* Zwischenspeicher */

		/* Diskriminante berechnen */
		tmp = (x * costab[winkel] - y * sintab[winkel]) / TRIGFAKTOR;
		diskriminante = sqr(radius) - sqr(tmp);
	}

	/* hat die quadratische Gleichung keine zwei Loesungen, d.h. schneidet
	   die Bahn des Punktes den Kreis nicht zweimal? */
	if (diskriminante <= 0)

		/* liegt durch einen Rundungsfehler das Ziel des Punktes
		   innerhalb des Kreises? */
		if (sqr(x - strecke * sintab[winkel] / TRIGFAKTOR) +
			sqr(y - strecke * costab[winkel] / TRIGFAKTOR) <
			sqr(radius))
		{
			/* die moegliche zurueckgelegte Strecke soweit verkuerzen,
			   bis das Ziel ausserhalb des Kreises landet; keine
			   Korrektur berechnen, da eine Kollision eigentlich nicht
			   moeglich war; totaler Pfusch! */
			do
				if (--strecke == 0)
					return 0;
			while (sqr(x - strecke * sintab[winkel] / TRIGFAKTOR) +
				sqr(y - strecke * costab[winkel] / TRIGFAKTOR) <
				sqr(radius));

			return strecke;
		}
		else
			/* alles in Ordnung, keine Kollision */
			return -1;

	/* die Strecke, die bis zur Kollision zurueckzulegen ist,
	   ist mitte +- wurzel(diskriminante) */
	mitte = (x * sintab[winkel] + y * costab[winkel]) / TRIGFAKTOR;
	w = wurzel(diskriminante, W_AUFRUNDEN);

	/* hat der Punkt den Kreis schon hinter sich gelassen? */
	if (mitte < 0)
		/* keine Kollision */
		return -1;

	/* wird der Punkt den Kreis nicht erreichen? */
	if (mitte - w >= strecke)
		/* keine Kollision */
		return -1;

	/* Strecke, die der Punkte bis zur Kollision zuruecklegen kann */
	strecke = mitte - w;

	/* ist die Strecke durch einen Rundungsfehler kleiner als Null,
	   korrigieren */
	if (strecke < 0)
		strecke = 0;

	{
		int sx, sy; /* Koordinaten des Kollisionspunktes */

		/* Kollisionspunkt berechnen */
		sx = x - strecke * sintab[winkel] / TRIGFAKTOR;
		sy = y - strecke * costab[winkel] / TRIGFAKTOR;

		/* Winkel, an dem der Kollisionspunkt auf dem Kreis liegt */
		korrektur->winkel1 = (atan(sx, sy) + TRIGANZ / 2) % TRIGANZ;

		/* nach links abgleiten? */
		if ((winkel + 3 * TRIGANZ / 2 - korrektur->winkel1) %
			TRIGANZ > TRIGANZ / 2)
		{
			/* Winkel, an dem eine zur Bahn des Punktes parallele
			   Tangente den Kreis trifft */
			korrektur->winkel2 = (winkel + 3 * TRIGANZ / 4) % TRIGANZ;

			/* winkel1 und winkel2 in der richtigen Reihenfolge? */
			if ((korrektur->winkel1 - korrektur->winkel2 + TRIGANZ) %
				TRIGANZ > TRIGANZ / 2)
				korrektur->radius = wurzel(sqr(sx) + sqr(sy), W_AUFRUNDEN);
		}
		/* nach rechts abgleiten? */
		else if ((korrektur->winkel1 + 3 * TRIGANZ / 2 - winkel) %
			TRIGANZ > TRIGANZ / 2)
		{
			/* Winkel, an dem eine zur Bahn des Punktes parallele
			   Tangente den Kreis trifft */
			korrektur->winkel2 = (winkel + TRIGANZ / 4) % TRIGANZ;

			/* winkel2 und winkel1 in der richtigen Reihenfolge? */
			if ((korrektur->winkel2 - korrektur->winkel1 + TRIGANZ) %
				TRIGANZ > TRIGANZ / 2)
				korrektur->radius = wurzel(sqr(sx) + sqr(sy), W_AUFRUNDEN);
		}

		/* wurde eine Korrekturbahn gefunden? */
		if (korrektur->radius > 0)
		{
			/* Richtung der Korrekturbahn anpassen */
			korrektur->winkel1 = (kfaktor * korrektur->winkel1 + kwinkel) %
				TRIGANZ;
			korrektur->winkel2 = (kfaktor * korrektur->winkel2 + kwinkel) %
				TRIGANZ;
		}
	}

	/* solange das Ziel des Punktes durch einen Rundungsfehler innerhalb
	   des Kreises liegt, die moegliche zurueckgelegte Strecke; Pfusch! */
	while (sqr(x - strecke * sintab[winkel] / TRIGFAKTOR) +
		sqr(y - strecke * costab[winkel] / TRIGFAKTOR) <
		sqr(radius))
		if (--strecke <= 0)
			return 0;

	/* verbleibende freie Strecke zurueckgeben */
	return strecke;
}


/*
** kreis_schneidet_wand
**  prueft, ob ein Kreis um (x,y) mit Radius radius eine Wand, die
**  von (0,0) bis (RASPT,0) verlaeuft, schneidet (nicht nur beruehrt)
**
** Parameter:
**  x: x-Koordinate des Kreismittelpunktes
**  y: y-Koordinate des Kreismittelpunktes
**  radius: Radius des Kreises
**
** Rueckgabewert:
**  1 fuer ja, 0 fuer nein
*/
static int kreis_schneidet_wand(int x, int y, int radius)
{
	/* bei x-Koordinate <= 0 am anderen Ende der Wand rechnen */
	if (x <= 0)
		x = RASPT - x;

	/* falls allein die x-Koordinate oder die y-Koordinate des
	   Mittelpunkts den Schnitt mit der Wand ausschliessen, fertig */
	if (x >= radius + RASPT || y <= -radius || y >= radius)
		return 0;

	/* bei einer x-Koordinate im Bereich der x-Koordinaten der
	   Wand und einem Abstand (y-Koordinate) von der Wand
	   kleiner oder gleich dem Radius schneidet der Kreis die Wand */
	if (x <= RASPT)
		return 1;

	/* sonst muss der Endpunkt der Wand auf der Kreisflaeche liegen */
	return sqr(x - RASPT) + sqr(y) < sqr(radius);
}


/*
** kreis_trifft_wand
**  prueft, ob ein Kreis mit Radius radius, der sich von (x,y) in Richtung
**  winkel um die Strecke strecke bewegt, mit einer Wand, die von (0,0) bis
**  (RASPT,0) verlaeuft, kollidiert; berechnet eine Korrekturbahn
**
** Parameter:
**  x: x-Koordinate des Kreismittelpunktes
**  y: y-Koordinate des Kreismittelpunktes
**  radius: Radius des Kreises
**  winkel: Richtung, in die sich der Kreis bewegt
**  strecke: Strecke, um die sich der Kreis bewegt
**  korrektur: Zeiger auf eine Struktur, in der die Parameter
**             der Korrekturbahn abgelegt werden
**
** Rueckgabewert:
**  -1, falls der Kreis nicht mit der Wand kollidiert;
**  sonst die Laenge der Strecke, die er vorher zuruecklegen kann
**
** Seiteneffekte:
**  *korrektur wird gesetzt
*/
static int kreis_trifft_wand(int x, int y, int radius, int winkel, int strecke,
	struct korrektur *korrektur)
{
	int kwinkel;     /* Winkel, um den die von punkt_trifft_wand
	                    ermittelte Korrekturbahn gedreht werden muss */
	int kfaktor;     /* Vorzeichen, mit dem der Winkel der Korrekturbahn
	                    multipliziert werden muss */
	int strecke_neu; /* Strecke, um die sich der Kreis ohne Kollision
	                    bewegen kann (Rueckgabewert) */

	/* initialisieren */
	korrektur->typ = TYP_NICHTS;
	kwinkel = 0;
	kfaktor = 1;

	/* falls sich der Kreis ueberhaupt nicht bewegen soll,
	   keine Kollision */
	if (strecke == 0)
		return -1;

	/* bewegt sich der Kreis nach Sueden oder Osten, die Szene
	   um TRIGANZ/2 drehen */
	if (winkel >= TRIGANZ / 2)
	{
		/* Position des Mittelpunktes drehen */
		x = RASPT - x;
		y = -y;

		/* Richtung des Kreises drehen */
		winkel = (winkel + TRIGANZ / 2) % TRIGANZ;

		/* spaeter Richtung der Korrekturbahn drehen */
		kwinkel = TRIGANZ / 2;
	}

	/* bewegt sich der Kreis nicht nach Nordwesten, sondern nach
	   Suedwesten, die Szene an der x-Achse spiegeln */
	if (winkel > TRIGANZ / 4)
	{
		/* Position des Mittelpunktes spiegeln */
		y = -y;

		/* Richtung des Kreises anpassen */
		winkel = TRIGANZ / 2 - winkel;

		/* spaeter Richtung der Korrekturbahn anpassen */
		kwinkel = (kwinkel + TRIGANZ / 2) % TRIGANZ + TRIGANZ;
		kfaktor = -1;
	}

	/* bewegt sich der Kreis nicht genau laengs der Wand? */
	if (winkel != TRIGANZ / 4)
	{
		/* beruehrt der Kreis eine Seite der Wand? */
		if (x >= 0 && x <= RASPT && y == radius)
		{
			/* dann verlaeuft die Korrekturbahn an der Wand entlang ... */
			korrektur->typ = TYP_GERADE;

			/* ... in Richtung TRIGANZ/4 (Westen), korrigiert mit
			   kfaktor und kwinkel */
			korrektur->daten.gerade.winkel = (kfaktor * (TRIGANZ / 4) +
				kwinkel) % TRIGANZ;

			/* Korrekturbahn bis zum Ende der Wand, aber mindestens
			   einen Schritt, um eine Endlosschleife durch Rundungsfehler
			   zu vermeiden */
			if (x > 0)
				korrektur->daten.gerade.strecke = x;
			else
				korrektur->daten.gerade.strecke = 1;

			/* da der Kreis die Wand bereits beruehrt, ist vor der
			   Korrektur keine Bewegung moeglich */
			return 0;
		}

		/* wenn sich der Kreis auf die Wand zubewegen, trifft er sie
		   dann seitwaerts? */
		strecke_neu = punkt_trifft_wand(x, y - radius, winkel, strecke,
			&korrektur->daten.gerade);

		if (strecke_neu >= 0)
		{
			/* der Kreis trifft die Wand */

			/* gibt es eine Korrekturbahn? */
			if (korrektur->daten.gerade.strecke > 0)
			{
				/* Typ setzen */
				korrektur->typ = TYP_GERADE;

				/* Richtung der Korrekturbahn anpassen */
				korrektur->daten.gerade.winkel = (kfaktor *
					korrektur->daten.gerade.winkel + kwinkel) % TRIGANZ;
			}

			/* verbleibende freie Strecke zurueckgeben */
			return strecke_neu;
		}
	}

	/* schneidet der Kreis bereits jetzt die Wand, ist weder Bewegung
	   noch Korrektur moeglich */
	if (kreis_schneidet_wand(x, y, radius))
		return 0;

	/* trifft der Kreis bei seiner Bewegung die Ecke (RASPT,0) der Wand? */
	strecke_neu = punkt_trifft_kreis(RASPT - x, -y,
		(winkel + TRIGANZ / 2) % TRIGANZ, strecke, radius,
		&korrektur->daten.kurve);

	if (strecke_neu >= 0)
	{
		/* der Kreis trifft die Ecke */

		/* gibt es eine Korrekturbahn? */
		if (korrektur->daten.kurve.radius > 0)
		{
			/* Typ setzen */
			korrektur->typ = TYP_KURVE;

			/* Richtung der Korrekturbahn anpassen */
			korrektur->daten.kurve.winkel1 = (kfaktor *
				korrektur->daten.kurve.winkel1 + kwinkel) % TRIGANZ;
			korrektur->daten.kurve.winkel2 = (kfaktor *
				korrektur->daten.kurve.winkel2 + kwinkel) % TRIGANZ;
		}

		/* verbleibende freie Strecke zurueckgeben */
		return strecke_neu;
	}

	/* trifft der Kreis bei seiner Bewegung die Ecke (0,0) der Wand? */
	strecke_neu = punkt_trifft_kreis(-x, -y,
		(winkel + TRIGANZ / 2) % TRIGANZ, strecke, radius,
		&korrektur->daten.kurve);

	if (strecke_neu >= 0)
	{
		/* der Kreis trifft die Ecke */

		/* gibt es eine Korrekturbahn? */
		if (korrektur->daten.kurve.radius > 0)
		{
			/* Typ setzen */
			korrektur->typ = TYP_KURVE;

			/* Richtung der Korrekturbahn anpassen */
			korrektur->daten.kurve.winkel1 = (kfaktor *
				korrektur->daten.kurve.winkel1 + kwinkel) % TRIGANZ;
			korrektur->daten.kurve.winkel2 = (kfaktor *
				korrektur->daten.kurve.winkel2 + kwinkel) % TRIGANZ;
		}

		/* verbleibende freie Strecke zurueckgeben */
		return strecke_neu;
	}

	/* der Kreis trifft die Wand weder von der Seite, noch an der
	   Ecke, kann sich also ungehindert bewegen */
	return -1;
}


/*
** ist_wand
**  prueft, ob vom Feld (x,y) aus in Richtung himricht eine Wand ist
**
** Parameter:
**  x: x-Koordinate des Feldes
**  y: y-Koordinate des Feldes
**  himricht: Himmelsrichtung (NORD, WEST, SUED, OST)
**  spieler: 1, falls unbegehbar geprueft werden soll;
**           0, falls schusssicher geprueft werden soll
**  kamera: 1, falls nur der Rand geprueft werden soll
**
** Rueckgabewert:
**  1 fuer ja, 0 fuer nein
*/
static int ist_wand(int x, int y, int himricht, int spieler, int kamera)
{
	/* falls x oder y ausserhalb des Spielfeldes liegt oder in
	   der angegebenen Richtung eine Aussenwand ist, "ja" zurueckgeben */
	if (x < 0 || x >= feldbreite || y < 0 || y >= feldlaenge ||
		y == 0 && himricht == NORD ||
		x == 0 && himricht == WEST ||
		y == feldlaenge - 1 && himricht == SUED ||
		x == feldbreite - 1 && himricht == OST)
		return 1;

	/* Kameras sind von inneren Waenden unbeeinflusst */
	if (kamera)
		return 0;

	/* sonst das entsprechende Flag zurueckgeben */
	return spieler ? spielfeld[x][y][himricht].unbegehbar :
		spielfeld[x][y][himricht].schusssicher;
}


/*
** kreis_kollidiert
**  berechnet, wie weit sich ein Kreis mit Radius radius von (x,y) aus
**  in Richtung winkel durch das Labyrinth bewegen kann, bis er
**  kollidiert, und berechnet eine Korrekturbahn
**
** Parameter:
**  x: x-Koordinate des Kreismittelpunktes
**  y: y-Koordinate des Kreismittelpunktes
**  radius: Radius des Kreises
**  winkel: Richtung, in der sich der Kreis bewegen soll
**  strecke: Strecke, um die sich der Kreis maximal bewegen soll
**  spieler: Flag, ob der Kreis ein Spieler ist (und durch Tueren
**           gehen kann)
**  nr: Nummer des Spielers/Schusses (mit Spielern/Schuessen der
**      gleichen Farbe ist keine Kollision moeglich)
**  mit_spieler: Zeiger auf die Nummer des Spielers, mit dem
**               kollidiert wurde; wird -1, falls nicht;
**               Kollisionen mit Spielern werden nicht geprueft,
**               wenn der Zeiger NULL ist
**  mit_schuss: Zeiger auf die Nummer des Schusses, mit dem
**              kollidiert wurde; wird -1, falls nicht;
**              Kollisionen mit Schuessen werden nicht geprueft,
**              wenn der Zeiger NULL ist
**  korrektur: Zeiger auf eine Struktur, in der die Parameter
**             der Korrekturbahn gespeichert werden sollen
**  kamera: Flag, ob der Kreis eine Kamera ist
**
** Rueckgabewert:
**  Strecke, nach der der Kreis kollidiert; -1, falls nicht
**
** Seiteneffekte:
**  mit_spieler, mit_schuss und korrektur werden gesetzt
*/
static int kreis_kollidiert(int x, int y, int radius, int winkel, int strecke,
	int spieler, int nr, int *mit_spieler, int *mit_schuss,
	struct korrektur *korrektur, int kamera)
{
	int xgrob, ygrob;   /* x- und y-Koordinate des Feldes, in dem sich
	                       der hintere rechte Punkt eines den Kreis
	                       umgebenden Quadrates befindet */
	int dx, dy;         /* Koordinatendifferenz zum naechsten in
	                       Bewegungsrichtung liegenden Feld */
	int himricht;       /* grobe Himmelsrichtung, in die sich der Kreis
	                       bewegt; NORD fuer Nord bis West, WEST fuer
	                       West bis Sued usw. */
	int strecke_alt;    /* Zwischenspeicher fuer die uebergebene Strecke */
	int x_alt, y_alt;   /* Zwischenspeicher fuer die uebergebenen
	                       Koordinaten */
	int i;              /* genereller Index */
	int winkel_alt;     /* Zwischenspeicher fuer die uebergebene Richtung */
	int letzte_strecke; /* maximal erlaubte Strecke, die im letztem
	                       Schleifendurchlauf berechnet wurde */

	/* noch keine Korrekturbahn berechnet */
	korrektur->typ = TYP_NICHTS;

	/* noch mit keinem Spieler oder Schuss kollidiert */
	if (mit_spieler != NULL)
		*mit_spieler = -1;
	if (mit_schuss != NULL)
		*mit_schuss = -1;

	/* eine Strecke der Laenge Null kann immer ohne Kollision
	   zurueckgelegt werden */
	if (strecke == 0)
		return -1;

	/* strecke, x, y und winkel retten */
	strecke_alt = strecke;
	x_alt = x;
	y_alt = y;
	winkel_alt = winkel;

	/* grobe Himmelsrichtung feststellen; xgrob, ygrob, dx und dy
	   berechnen; (x,y) nach Norden drehen und (xgrob,ygrob) abziehen */
	switch (himricht = winkel / (TRIGANZ / 4))
	{
		case NORD:
			xgrob = (x + radius - 1) / RASPT;
			ygrob = (y + radius - 1) / RASPT;
			x -= xgrob * RASPT;
			y -= ygrob * RASPT;
			dx = 0;
			dy = -1;
			break;

		case WEST:
			xgrob = (x + radius - 1) / RASPT;
			ygrob = (y - radius) / RASPT;
			{
				int tmp;

				tmp = x;
				x = (ygrob + 1) * RASPT - y;
				y = tmp - xgrob * RASPT;
			}
			dx = -1;
			dy = 0;
			break;

		case SUED:
			xgrob = (x - radius) / RASPT;
			ygrob = (y - radius) / RASPT;
			x = (xgrob + 1) * RASPT - x;
			y = (ygrob + 1) * RASPT - y;
			dx = 0;
			dy = 1;
			break;

		case OST:
			xgrob = (x - radius) / RASPT;
			ygrob = (y + radius - 1) / RASPT;
			{
				int tmp;

				tmp = x;
				x = y - ygrob * RASPT;
				y = (xgrob + 1) * RASPT - tmp;
			}
			dx = 1;
			dy = 0;
			break;
	}

	/* Blickrichtung entsprechend drehen */
	winkel %= TRIGANZ / 4;

	/* Schleife, bis sich die berechnete maximal moegliche Strecke
	   nicht mehr aendert */
	do
	{
		int nullstrecke_gefunden; /* mit TYP_KURVE */
		struct korrektur korrektur_versuch;

		/* Strecke merken */
		letzte_strecke = strecke;

		nullstrecke_gefunden = 0;

		/* vier Waende pruefen */
		for (i = 0; i < 4; i++)
		{
			int pruefen; /* Wand ist undurchlaessig,
			                auf Kollision pruefen */
			int x0, y0;  /* relative Koordinaten des Kreises im
			                Verhaeltnis zur Wand */
			int kwinkel; /* Korrekturwinkel */

			switch (i)
			{
				case 0: /* Wand am aktuellen Feld des Kreises,
				           grob laengs in Bewegungsrichtung */

					/* Kollision moeglich, wenn der Kreis sich schraeg
					   auf die Wand zubewegt oder sie von beiden
					   Seiten undurchlaessig ist */
					pruefen = ist_wand(xgrob, ygrob, (himricht + 1) % 4,
						spieler, kamera) && (winkel > 0 || ist_wand(xgrob + dy,
						ygrob - dx, (himricht + 3) % 4, spieler, kamera));

					/* Koordinaten und Winkel anpassen */
					x0 = RASPT - y;
					y0 = x;
					kwinkel = TRIGANZ / 4;

					break;

				case 1: /* Wand rechts vor dem Kreis,
				           grob quer zur Bewegungsrichtung */

					/* Kollision moeglich, wenn die Wand von dieser
					   Seite undurchlaessig ist */
					pruefen = ist_wand(xgrob, ygrob, himricht, spieler, kamera);

					/* Koordinaten und Winkel anpassen */
					x0 = x;
					y0 = y;
					kwinkel = 0;

					break;

				case 2: /* Wand links vor dem Kreis,
				           grob quer zur Bewegungsrichtung */

					/* Kollision moeglich, wenn die Wand von dieser
					   Seite undurchlaessig ist */
					pruefen = ist_wand(xgrob + dy, ygrob - dx, himricht,
						spieler, kamera);

					/* Koordinaten und Winkel anpassen */
					x0 = x + RASPT;
					y0 = y;
					kwinkel = 0;

					break;

				case 3: /* Wand am naechsten Feld,
				           grob laengs in Bewegungsrichtung */

					/* Kollision moeglich, wenn der Kreis sich schraeg
					   auf die Wand zubewegt oder sie von beiden
					   Seiten undurchlaessig ist */
					pruefen = ist_wand(xgrob + dx, ygrob + dy,
						(himricht + 1) % 4, spieler, kamera) && (winkel > 0 ||
						ist_wand(xgrob + dx + dy, ygrob + dy - dx,
						(himricht + 3) % 4, spieler, kamera));

					/* Koordinaten und Winkel anpassen */
					x0 = -y;
					y0 = x;
					kwinkel = TRIGANZ / 4;

					break;
			}

			/* Kollision mit der Wand moeglich? */
			if (pruefen)
			{
				int strecke_neu;                /* Strecke bis zur
				                                   Kollision */
				struct korrektur korrektur_neu; /* Parameter fuer die
				                                   Bahnkorrektur */

				/* Kollision testen */
				strecke_neu = kreis_trifft_wand(x0, y0, radius,
					((winkel + TRIGANZ - kwinkel) % TRIGANZ),
					strecke, &korrektur_neu);

				/* Kollision? */
				if (strecke_neu >= 0)
				{
					/* Bahnkorrektur uebernehmen */
					memcpy(korrektur, &korrektur_neu,
						sizeof(struct korrektur));

					/* Richtung der Bahnkorrektur anpassen */
					if (korrektur->typ == TYP_GERADE)
						korrektur->daten.gerade.winkel =
							(korrektur->daten.gerade.winkel + kwinkel +
							winkel_alt - winkel) % TRIGANZ;
					else if (korrektur->typ == TYP_KURVE)
					{
						korrektur->daten.kurve.winkel1 =
							(korrektur->daten.kurve.winkel1 + kwinkel +
							winkel_alt - winkel) % TRIGANZ;
						korrektur->daten.kurve.winkel2 =
							(korrektur->daten.kurve.winkel2 + kwinkel +
							winkel_alt - winkel) % TRIGANZ;
					}

					if (strecke_neu == 0 &&
						korrektur->typ == TYP_KURVE)
					{
						/* Bahnkorrektur merken */
						memcpy(&korrektur_versuch,
							korrektur,
							sizeof(struct
								korrektur));

						nullstrecke_gefunden = 1;

						/* weitersuchen nach
						   TYP_GERADE */
						continue;
					}

					/* Strecke uebernehmen */
					strecke = strecke_neu;

					/* keine Bewegung bis zur Kollision? */
					if (strecke == 0)
						/* dann fertig */
						return strecke;
				}
			}
		}

		if (nullstrecke_gefunden)
		{
			/* Bahnkorrektur restaurieren */
			memcpy(korrektur, &korrektur_versuch,
				sizeof(struct korrektur));

			return 0;
		}

		/* alle Spieler und Schuesse pruefen, ausser denen mit der Nummer
		   des Kreises */
		for (i = 0; i < SPIELERANZ; i++)
		{
			int strecke_neu;                      /* Strecke bis zur
			                                         Kollision */
			struct korrektur_kurve korrektur_neu; /* Parameter fuer die
			                                         Bahnkorrektur */

			/* Spieler/Schuesse mit Nummer des Kreises und nicht
			   existierende Spieler und Kameras ignorieren */
			if (i == nr || !spieler_aktiv[i] ||
				ist_kamera[i] || kamera)
				continue;

			/* Kollisionen mit Spielern testen und Spieler nicht tot? */
			if (mit_spieler != NULL && !spieler_tot[i])
			{
				/* Kollision testen */
				strecke_neu = punkt_trifft_kreis(x_alt -
					XPOS(alle_spieler[i].pos),
					y_alt - YPOS(alle_spieler[i].pos), winkel_alt,
					strecke, radius + KUGELRAD, &korrektur_neu);

				/* Kollision? */
				if (strecke_neu >= 0)
				{
					/* falls Bahnkorrektur berechnet, Bahnkorrektur
					   uebernehmen, Typ setzen und Richtung der
					   Bahnkorrektur anpassen */
					if (korrektur_neu.radius > 0)
					{
						korrektur->typ = TYP_KURVE;
						korrektur->daten.kurve.winkel1 =
							(korrektur_neu.winkel1 + TRIGANZ / 2) % TRIGANZ;
						korrektur->daten.kurve.winkel2 =
							(korrektur_neu.winkel2 + TRIGANZ / 2) % TRIGANZ;
						korrektur->daten.kurve.radius =
							korrektur_neu.radius;
					}
					else
						/* keine Bahnkorrektur */
						korrektur->typ = TYP_NICHTS;

					/* mit Spieler i kollidiert */
					*mit_spieler = i;

					/* mit keinem Schuss kollidiert */
					if (mit_schuss != NULL)
						*mit_schuss = -1;

					/* Strecke uebernehmen */
					strecke = strecke_neu;

					/* keine Bewegung bis zur Kollision? */
					if (strecke == 0)
						/* dann fertig */
						return strecke;
				}
			}

			/* Kollisionen mit Schuessen testen und Schuss unterwegs? */
			if (mit_schuss != NULL && schuss_fliegt[i])
			{
				/* Kollision testen */
				strecke_neu = punkt_trifft_kreis(x_alt -
					XPOS(alle_schuesse[i].pos),
					y_alt - YPOS(alle_schuesse[i].pos), winkel_alt,
					strecke, radius + SCHUSSRAD, &korrektur_neu);

				/* Kollision? */
				if (strecke_neu >= 0)
				{
					/* falls Bahnkorrektur berechnet, Bahnkorrektur
					   uebernehmen, Typ setzen und Richtung der
					   Bahnkorrektur anpassen */
					if (korrektur_neu.radius > 0)
					{
						korrektur->typ = TYP_KURVE;
						korrektur->daten.kurve.winkel1 =
							(korrektur_neu.winkel1 + TRIGANZ / 2) % TRIGANZ;
						korrektur->daten.kurve.winkel2 =
							(korrektur_neu.winkel2 + TRIGANZ / 2) % TRIGANZ;
						korrektur->daten.kurve.radius =
							korrektur_neu.radius;
					}
					else
						/* keine Bahnkorrektur */
						korrektur->typ = TYP_NICHTS;

					/* mit Schusss i kollidiert */
					*mit_schuss = i;

					/* mit keinem Spieler kollidiert */
					if (mit_spieler != NULL)
						*mit_spieler = -1;

					/* Strecke uebernehmen */
					strecke = strecke_neu;

					/* keine Bewegung bis zur Kollision? */
					if (strecke == 0)
						/* dann fertig */
						return strecke;
				}
			}
		}

		/* neue Strecke laenger als im vorigen Durchlauf? */
		if (strecke > letzte_strecke)
		{
			/* Rundungsfehler, korrigieren */
			strecke = letzte_strecke - 1;

			/* keine Bewegung bis zur Kollision? */
			if (strecke <= 0)
				/* dann fertig */
				return 0;
		}
	}
	while (strecke != letzte_strecke);

	/* keine Kollision? dann -1 zurueckgeben, sonst die Strecke */
	return strecke == strecke_alt ? -1 : strecke;
}


/*
** punkte_erhoehen
**  erhoeht (reduziert) die Punkte eines Spielers
**
** Parameter:
**  spieler_nr: Nummer des Spielers
**  differenz: Wert, um die die Punkte erhoeht werden sollen (auch < 0 erlaubt)
**
** Seiteneffekte:
**  spieler_punkte wird veraendert
*/
static void punkte_erhoehen(int spieler_nr, int differenz)
{
	int punkte; /* neuer Punktestand */

	/* neuen Punktestand berechnen */
	punkte = spieler_punkte[spieler_nr] + differenz;

	/* Ueberlauf abfangen */
	if (punkte < 0)
		punkte = 0;
	else if (punkte > MAX_PUNKTE)
		punkte = MAX_PUNKTE;

	/* hat sich der Punktestand veraendert? */
	if (spieler_punkte[spieler_nr] != punkte)
	{
		int i; /* Index fuer Spieler */

		/* Punktestand speichern */
		spieler_punkte[spieler_nr] = punkte;

		/* fuer alle aktiven Spieler diesen Punktestand uebertragen */
		for (i = 0; i < SPIELERANZ; i++)
			if (spieler_aktiv[i])
				punkte_senden[i][spieler_nr] = NEU_SENDEN;
	}
}


static void spruch_kopieren(int spieler_nr, int gegner_nr)
{
	int laenge;

	laenge = abgeschossen_spruch_laenge[spieler_nr] =
		spieler_spruch_laenge[gegner_nr];

	if (laenge)
	{
		speicher_belegen((void **)&abgeschossen_spruch[spieler_nr],
			laenge);

		memcpy(abgeschossen_spruch[spieler_nr],
			spieler_spruch[gegner_nr], laenge);
	}
}


/*
** spieler_schritt
**  bewegt einen Spieler einen Schritt vorwaerts
**
** Parameter:
**  spieler_nr: Nummer des Spielers
**  pos: Zeiger auf die Position des Spielers
**  winkel: Richtung des Schrittes
**  gesamt_schritt: gewuenschte Laenge des Schrittes
**  korrektur: Zeiger auf eine Struktur, in der die Parameter
**             einer Korrekturbahn abgelegt werden
**
** Rueckgabewert:
**  -1, falls der Spieler getoetet wurde; sonst die Laenge
**  des tatsaechlich zurueckgelegten Weges
**
** Seiteneffekte:
**  *pos wird veraendert, *korrektur wird gesetzt
*/
static int spieler_schritt(int spieler_nr, struct position *pos, int winkel,
	int gesamt_schritt, struct korrektur *korrektur)
{
	int schritt;     /* Laenge der Strecke, um die sich der Spieler
	                    bis zu einer Kollision bewegen kann */
	int mit_spieler; /* Nummer des Spielers, mit dem der Spieler
	                    kollidiert ist; -1 falls nicht */
	int mit_schuss;  /* Nummer des Schusses, mit dem der Spieler
	                    kollidiert ist; -1 falls nicht */

	/* berechnen, wie weit sich der Spieler bis zur Kollision
	   bewegen kann; -1 fuer "keine Kollision" */
	schritt = kreis_kollidiert(XPOS(*pos), YPOS(*pos),
		KUGELRAD, winkel, gesamt_schritt, 1, spieler_nr, &mit_spieler,
		&mit_schuss, korrektur, ist_kamera[spieler_nr]);

	/* mit einem Schuss kollidiert? */
	if (mit_schuss >= 0)
	{
		/* der Spieler wurde durch den Besitzer des
		   Schusses abgeschossen */
		ereignisse[spieler_nr][ABGESCHOSSEN_ERG] = 1;
		spieler_tot[spieler_nr] = SPIELER_TOT_ZEIT;
		abgeschossen_durch[spieler_nr] = mit_schuss;
		spruch_kopieren(spieler_nr, mit_schuss);

		/* einen Punkt fuer den Besitzer des Schusses */
		punkte_erhoehen(mit_schuss, 1);

		/* der Schuss fliegt nicht mehr */
		schuss_fliegt[mit_schuss] = 0;

		/* Ereignis merken */
		ereignisse[mit_schuss][ABSCHUSS_ERG] = 1;

		/* autoanswer? */
		if (opt_autoanswer[spieler_nr])
		{
			autoanswer_ausgeloest[spieler_nr]++;
			autoanswer_blick[spieler_nr] =
				(schuss_blick[mit_schuss] + WINKANZ / 2) %
					WINKANZ;
		}

		/* Spieler ist tot */
		return -1;
	}

	/* nicht kollidiert? */
	if (schritt < 0)
		/* die gesamte Strecke kann gelaufen werden */
		schritt = gesamt_schritt;

	/* laufen? */
	if (schritt >= 1)
	{
		/* Position veraendern */
		SUBX(*pos, sintab[winkel] * schritt / TRIGFAKTOR);
		SUBY(*pos, costab[winkel] * schritt / TRIGFAKTOR);
	}

	/* gelaufene Strecke zurueckgeben */
	return schritt;
}


/*
** bewege_spieler
**  bewegt einen Spieler um einen Schritt; der Spieler gleitet
**  von Waenden und anderen Spielern ab
**
** Parameter:
**  spieler_nr: Nummer des Spielers
**  spieler: Zeiger auf die Position des Spielers
**  vor: 1, falls sich der Spieler vorwaerts bewegen will, sonst 0
**  zurueck: 1, falls sich der Spieler rueckwaerts bewegen will, sonst 0
**  links: 1, falls sich der Spieler nach links drehen will, sonst 0
**  rechts: 1, falls sich der Spieler nach rechts drehen will, sonst 0
**
** Seiteneffekte:
**  *spieler wird veraendert
*/
static void bewege_spieler(int spieler_nr, struct spieler *spieler, int vor,
	int zurueck, int links, int rechts)
{
	int rest_schritt;   /* Strecke, die der Spieler noch
	                       zuruecklegen darf */
	int winkel;         /* Richtung, in die der Spieler laufen will */
	int schritt;        /* Laenge der Strecke, die der Spieler laufen
	                       will bzw. kann */
	int gelaufen;       /* Flag, ob seit dem letzten Versuch in der
	                       eigentlich gewuenschten Richtung eine Bewegung
	                       ausgefuehrt wurde */
	int probiert;       /* Flag, ob im letzten Schleifendurchlauf
	                       spieler_schritt aufgerufen wurde */
	int letzter_winkel; /* Richtung, in der im letzten Schleifendurchlauf
	                       eine Bewegung probiert wurde */
	int winkel_neu;     /* Richtung, in der in diesem Schleifendurchlauf
	                       gelaufen werden soll */
	int coswert;        /* Anteil der tatsaechlich zu laufenden Strecke */
	struct korrektur korrektur; /* Parameter der Bahnkorrektur */

	/* erst drehen */
	spieler->blick = (spieler->blick + links - rechts + WINKANZ) % WINKANZ;

	/* weder vor noch zurueck? */
	if (vor == zurueck)
		/* dann fertig */
		return;

	/* Richtung feststellen, in die der Spieler laufen will */
	if (vor > zurueck)
		winkel = spieler->blick * (TRIGANZ / WINKANZ);
	else
		winkel = (spieler->blick * (TRIGANZ / WINKANZ) +
			TRIGANZ / 2) % TRIGANZ;

	/* rest_schritt initialisieren */
	rest_schritt = SCHRITT;

	/* zuerst gerade laufen */
	probiert = 0;
	gelaufen = 1;
	letzter_winkel = -1;

	/* Schleife, bis Bewegungskontingent verbraucht ist;
	   wird auch bei probiert=gelaufen=0 verlassen */
	while (rest_schritt > 0)
	{
		/* wurden die letzten beiden Schritte in zwei entgegengesetzte
		   Richtungen probiert, so zaehlt das als nicht probiert (um
		   dann mit probiert=gelaufen=0 abzubrechen) */
		if (probiert && letzter_winkel >= 0 &&
			((winkel_neu - winkel + TRIGANZ) % TRIGANZ <= TRIGANZ / 2 &&
			(letzter_winkel - winkel_neu + TRIGANZ) % TRIGANZ >=
			(winkel_neu - winkel + TRIGANZ) % TRIGANZ ||
			(winkel_neu - winkel + TRIGANZ) % TRIGANZ >= TRIGANZ / 2 &&
			(letzter_winkel - winkel_neu + TRIGANZ) % TRIGANZ <=
			(winkel_neu - winkel + TRIGANZ) % TRIGANZ))
			probiert = 0;

		/* nicht probiert zu laufen? */
		if (!probiert)
			/* aber davor gelaufen? */
			if (gelaufen)
			{
				/* dann geradeaus neu probieren */
				korrektur.typ = TYP_GERADE;
				korrektur.daten.gerade.winkel = winkel;
				korrektur.daten.gerade.strecke = rest_schritt;

				/* noch nicht gelaufen */
				gelaufen = 0;
				letzter_winkel = -1;
			}
			else
				/* auch nicht, also beenden */
				break;
		else
			/* letzte probierte Richtung merken */
			letzter_winkel = winkel_neu;

		/* in diesem Durchlauf noch nicht probiert */
		probiert = 0;

		/* geradeaus probieren? */
		if (korrektur.typ == TYP_GERADE)
		{
			/* Laenge der Korrekturbahn Null? */
			if (korrektur.daten.gerade.strecke < 1)
				continue;

			/* in Richtung des Bahnkorrektur laufen */
			winkel_neu = korrektur.daten.gerade.winkel;
		}
		/* um die Kurve laufen? */
		else if (korrektur.typ == TYP_KURVE)
		{
			int sgn;        /* 1 fuer "nach links", -1 fuer "nach rechts" */
			int dwinkel;    /* Winkel, um den sich der Spieler
			                   in diesem Schritt bewegen soll */
			struct korrektur_kurve *kurve; /* temporaerer Zeiger auf die
			                                  Parameter der Korrekturbahn */

			/* temporaeren Zeiger initialisieren */
			kurve = &korrektur.daten.kurve;

			/* sind Start- und Zielwinkel oder Startwinkel und
			   Laufrichtung genau gegenueber, oder ist der Radius
			   der Korrekturbahn zu klein? */
			if (kurve->winkel1 % (TRIGANZ / 2) ==
				kurve->winkel2 % (TRIGANZ / 2) ||
				kurve->winkel1 % (TRIGANZ / 2) ==
				winkel % (TRIGANZ / 2) || kurve->radius < 2)
				/* dann nicht bewegen */
				continue;

			/* nach links oder nach rechts laufen? */
			sgn = (kurve->winkel2 - kurve->winkel1 + TRIGANZ) % TRIGANZ >
				TRIGANZ / 2 ? 1 : -1;

			/* liegt der Startwinkel auf der falschen Seite der
			   Kreisbahn? (d.h. rechts vom Spieler, obwohl er nach
			   links laufen wird; oder umgekehrt) */
			if (sgn > 0 && (kurve->winkel1 - winkel + TRIGANZ) % TRIGANZ <
				3 * TRIGANZ / 4 ||
				sgn < 0 && (winkel - kurve->winkel1 + TRIGANZ) % TRIGANZ <
				3 * TRIGANZ / 4)
				continue;

			/* geben die Korrekturparameter an, das der Spieler bis
			   hinter dem Kreis auf der Kreisbahn bleiben soll? */
			if (sgn < 0 && (winkel - kurve->winkel2 + TRIGANZ) % TRIGANZ >
				TRIGANZ / 2 ||
				sgn > 0 && (kurve->winkel2 - winkel + TRIGANZ) % TRIGANZ >
				TRIGANZ / 2)
				/* dann nur solange auf der Kreisbahn bleiben, bis
				   wieder freie Fahrt ist */
				kurve->winkel2 = winkel;

			/* um welchen Winkel muss sich der Spieler bewegen? */
			dwinkel = ((kurve->winkel2 - kurve->winkel1) * sgn +
				TRIGANZ) % TRIGANZ;

			/* hat der Winkel keinen sinnvollen Wert? */
			if (dwinkel < 1 || dwinkel >= TRIGANZ / 2)
				continue;

			/* Richtung in der der Spieler laufen soll (Tangente
			   an den Kreis) */
			winkel_neu = (korrektur.daten.kurve.winkel1 +
				sgn * (TRIGANZ / 4) + TRIGANZ) % TRIGANZ;
		}
		else
			/* sonst (falls keine Korrekturbahn berechnet
			   wurde) nicht bewegen */
			continue;

		/* Bewegung fuer sowohl TYP_GERADE als auch TYP_KURVE
		   durchfuehren */

		/* Strecken aufeinander projezieren */
		coswert = costab[(winkel_neu - winkel + TRIGANZ) % TRIGANZ];

		/* ist der zu laufende Anteil nicht positiv? */
		if (coswert <= 0)
			continue;

		/* maximal erlaubte Strecke berechnen */
		schritt = rest_schritt * coswert / TRIGFAKTOR;

		/* bei gerader Korrekturbahn darf die Strecke
		   die Laenge der Korrekturbahn nicht ueberschreiten */
		if (korrektur.typ == TYP_GERADE &&
			schritt > korrektur.daten.gerade.strecke)
			schritt = korrektur.daten.gerade.strecke;

		/* kein Schritt erlaubt? */
		if (schritt < 1)
			continue;

		/* versuchen, zu laufen; bei -1 (Spieler tot) abbrechen */
		if ((schritt = spieler_schritt(spieler_nr, &spieler->pos,
			winkel_neu, schritt, &korrektur)) < 0)
			break;

		/* spieler_schritt wurde aufgerufen */
		probiert = 1;

		/* hat sich der Spieler nicht bewegt? */
		if (schritt == 0)
			continue;

		/* Strecke vom Kontingent abziehen */
		rest_schritt -= (schritt * TRIGFAKTOR + coswert - 1) / coswert;

		/* in dieser Runde wurde der Spieler bewegt */
		gelaufen = 1;
	}
}


/*
** bewege_schuss
**  bewegt einen Schuss einen Schritt weiter; setzt bei einem Treffer
**  bei dem getroffenen Spieler die entsprechenden Flags
**
** Parameter:
**  besitzer: die Nummer des Spieler, der diesen Schuss abgefeuert hat
**  schuss: Zeiger auf die Position des Schusses
**  blick: Zeiger auf die Richtung des Schusses
**
** Rueckgabewert:
**  SCHUSS_TREFFER fuer einen Treffer; SCHUSS_DANEBEN, falls der
**  Schuss an einer Wand haengengeblieben ist; SCHUSS_WEITER, wenn
**  er weiterfliegen kann
**
** Seiteneffekte:
**  *schuss, ereignisse, spieler_tot und abgeschossen_durch
**  werden veraendert
*/
static int bewege_schuss(int besitzer, struct schuss *schuss, int *blick)
{
	int mit_spieler;            /* Nummer des Spielers, mit dem der Schuss
	                               kollidiert ist; -1 falls nicht */
	struct korrektur korrektur; /* Parameter der Bahnkorrektur
	                               (findet allerdings nicht statt) */
	int schritt, rest, frei;
	int neublick, ergebnis;

	rest = schritt = SCHUSSSCHRITT;
	ergebnis = SCHUSS_WEITER;

again:
	neublick = *blick;

	/* kollidiert der Schuss auf dieser Bahn mit einem Spieler oder
	   einer Wand? */
	if ((frei = kreis_kollidiert(XPOS(schuss->pos), YPOS(schuss->pos),
		SCHUSSRAD, *blick * (TRIGANZ / WINKANZ), schritt, 0,
		besitzer, &mit_spieler, NULL, &korrektur, 0)) >= 0)

		/* kollidiert er mit einem Spieler? */
		if (mit_spieler >= 0)
		{
			/* der Spieler wurde durch den Besitzer dieses
			   Schusses abgeschossen */
			ereignisse[mit_spieler][ABGESCHOSSEN_ERG] = 1;
			spieler_tot[mit_spieler] = SPIELER_TOT_ZEIT;
			abgeschossen_durch[mit_spieler] = besitzer;
			spruch_kopieren(mit_spieler, besitzer);

			/* autoanswer? */
			if (opt_autoanswer[mit_spieler])
			{
				autoanswer_ausgeloest[mit_spieler]++;
				autoanswer_blick[mit_spieler] =
					(schuss_blick[besitzer] +
						WINKANZ / 2) % WINKANZ;
			}

			/* einen Punkt fuer den Besitzer des Schusses */
			punkte_erhoehen(besitzer, 1);

			/* Schuss hat getroffen */
			return SCHUSS_TREFFER;
		}
		else if (opt_reflective[besitzer] &&
			korrektur.typ == TYP_GERADE &&
			korrektur.daten.gerade.winkel % (TRIGANZ / 4) == 0)
		{
			int dx, dy;

			dx = sintab[*blick * (TRIGANZ / WINKANZ)];
			dy = costab[*blick * (TRIGANZ / WINKANZ)];

			if (korrektur.daten.gerade.winkel /
				(TRIGANZ / 4) % 2 == 0)
				/* nach Norden oder Sueden abgleiten */
				dx = -dx;
			else
				/* nach Osten oder Westen abgleiten */
				dy = -dy;

			neublick = atan(dx, dy) / (TRIGANZ / WINKANZ);
			schritt = frei; /* kann auch 0 sein */

			ergebnis = SCHUSS_PRALLT_AB;

			/* Schuss weiter bewegen */
		}
		else
			/* Schuss hat eine Wand getroffen */
			return SCHUSS_DANEBEN;

	/* Schuss bewegen */
	SUBX(schuss->pos, sintab[*blick * (TRIGANZ / WINKANZ)] *
		schritt / TRIGFAKTOR);
	SUBY(schuss->pos, costab[*blick * (TRIGANZ / WINKANZ)] *
		schritt / TRIGFAKTOR);

	*blick = neublick;

	rest -= schritt;
	if (rest > 0)
	{
		schritt = rest;
		goto again;
	}

	/* Schuss existiert noch */
	return ergebnis;
}


/*
** naehe_setzen
**  berechnet die Werte fuer gegner_naehe in einem Quadrat
**  um das angegebene Feld neu; mit jedem Schritt Entfernung
**  wird der Wert um 1 kleiner; ist vorher schon ein groesserer
**  Wert eingetragen, dann nicht veraendern
**
** Parameter:
**  x: x-Koordinate des Feldes, um das herum neu gerechnet werden soll
**  y: y-Koordinate des Feldes, um das herum neu gerechnet werden soll
**  wert: Wert, auf den das Feld gesetzt werden soll
**
** Seiteneffekte:
**  veraendert gegner_naehe
*/
static void naehe_setzen(int x, int y, int wert)
{
	/* ist der bisherige Wert in gegner_naehe schon gross genug? */
	if (wert <= gegner_naehe[x][y])
		/* fertig */
		return;

	/* gegner_naehe setzen */
	gegner_naehe[x][y] = wert;

	/* die benachbarten Felder rekursiv auf den naechst kleineren
	   Wert setzen; bei Wert 0 fertig */
	if (--wert < 1)
		return;

	/* falls das Feld im Norden von diesem erreichbar ist oder
	   umgekehrt, gegner_naehe dort neu setzen */
	if (y > 0 && (!spielfeld[x][y][NORD].unbegehbar ||
		!spielfeld[x][y - 1][SUED].unbegehbar))
		naehe_setzen(x, y - 1, wert);

	/* falls das Feld im Westen von diesem erreichbar ist oder
	   umgekehrt, gegner_naehe dort neu setzen */
	if (x > 0 && (!spielfeld[x][y][WEST].unbegehbar ||
		!spielfeld[x - 1][y][OST].unbegehbar))
		naehe_setzen(x - 1, y, wert);

	/* falls das Feld im Sueden von diesem erreichbar ist oder
	   umgekehrt, gegner_naehe dort neu setzen */
	if (y < feldlaenge - 1 && (!spielfeld[x][y][SUED].unbegehbar ||
		!spielfeld[x][y + 1][NORD].unbegehbar))
		naehe_setzen(x, y + 1, wert);

	/* falls das Feld im Osten von diesem erreichbar ist oder
	   umgekehrt, gegner_naehe dort neu setzen */
	if (x < feldbreite - 1 && (!spielfeld[x][y][OST].unbegehbar ||
		!spielfeld[x + 1][y][WEST].unbegehbar))
		naehe_setzen(x + 1, y, wert);
}


/*
** spieler_beamen
**  berechnet fuer einen Spieler, der in das Spiel kommt, eine
**  zufaellige Startposition, die nicht zu nahe an den Positionen
**  der Gegner ist
**
** Parameter:
**  spieler_nr: Nummer des Spielers
**
** Rueckgabewert:
**  1, falls es kein einziges freies Feld im Labyrinth gibt;
**  0, falls der Spieler gebeamt werden konnte
**
** Seiteneffekte:
**  gegner_naehe wird ueberschrieben, alle_spieler[spieler_nr] wird gesetzt
*/
static int spieler_beamen(int spieler_nr)
{
	int x, y;    /* Indizes fuer das Spielfeld */
	int minimum; /* geringster "Naehe"-Wert, der erreicht werden kann */
	int anzahl;  /* Anzahl der Felder mit dem "Naehe"-Wert minimum */
	int i;       /* Index fuer die Spieler/allgemeiner Zaehler */

	/* gegner_naehe initialisieren; erstmal auf "kein Gegner in der Naehe"
	   setzen */
	for (x = 0; x < feldbreite; x++)
		for (y = 0; y < feldlaenge; y++)
			gegner_naehe[x][y] = 0;

	/* gegner_naehe fuer die Felder in der Naehe von allen
	   Spielern und Schuessen setzen */
	for (i = 0; i < SPIELERANZ; i++)
		/* existiert der Spieler? */
		if (spieler_aktiv[i])
		{
			/* ist der Spieler im Spiel? */
			if (!spieler_tot[i] && !ist_kamera[i])
				/* gegner_naehe anpassen */
				naehe_setzen(alle_spieler[i].pos.xgrob,
					alle_spieler[i].pos.ygrob, MIN_GEGNER_NAEHE);

			/* ist der Schuss unterwegs? */
			if (schuss_fliegt[i])
				/* gegner_naehe anpassen */
				naehe_setzen(alle_schuesse[i].pos.xgrob,
					alle_schuesse[i].pos.ygrob, MIN_SCHUSS_NAEHE);
		}

	/* minimalen Wert fuer in gegner_naehe suchen und Haeufigkeit
	   des Auftretens zaehlen */
	minimum = max(MIN_GEGNER_NAEHE, MIN_SCHUSS_NAEHE) + 1;
	for (x = 0; x < feldbreite; x++)
		for (y = 0; y < feldlaenge; y++)
			if (gegner_naehe[x][y] < minimum)
			{
				/* neues Minimum gefunden, Anzahl bisher 1 */
				minimum = gegner_naehe[x][y];
				anzahl = 1;
			}
			else if (gegner_naehe[x][y] == minimum)
				/* Minimum nochmal gefunden */
				anzahl++;

	/* kein Wert unter MIN_GEGNER_NAEHE gefunden? d.h. kein
	   Feld, auf dem kein Gegner steht */
	if (minimum >= MIN_GEGNER_NAEHE)
		/* nicht beamen */
		return 1;

	/* wuerfeln, welches der anzahl Felder mit minimalem
	   gegner_naehe-Wert genommen wird */
	i = zufall() % anzahl;

	/* Feld suchen */
	for (x = 0; x < feldbreite; x++)
	{
		for (y = 0; y < feldlaenge; y++)
			if (gegner_naehe[x][y] == minimum)
				if (--i < 0)
					/* Werte fuer x und y beibehalten */
					break;

		if (i < 0)
			break;
	}

	/* Position des Spielers setzen, Blickrichtung zufaellig */
	alle_spieler[spieler_nr].pos.xgrob = x;
	alle_spieler[spieler_nr].pos.ygrob = y;
	alle_spieler[spieler_nr].pos.xfein =
		alle_spieler[spieler_nr].pos.yfein = RASPT / 2;
	alle_spieler[spieler_nr].blick = zufall() % WINKANZ;

	/* Spieler wurde nicht abgeschossen */
	if (abgeschossen_durch[spieler_nr] >= 0 &&
		abgeschossen_spruch_laenge[spieler_nr])
		speicher_freigeben((void **)&abgeschossen_spruch[spieler_nr]);

	abgeschossen_durch[spieler_nr] = -1;

	/* Beamen war erfolgreich */
	return 0;
}


/*
** paket_verlaengern
**  haengt ein Teilpaket an ein Spiel-Paket
**
** Parameter:
**  paket_laenge: Zeiger auf die Laenge des Pakets
**  paket: Zeiger auf das Paket
**  kennung: Kennung des Teilpakets (siehe protokoll.h)
**  daten_laenge: Laenge der Daten des Teilpakets
**  daten: Daten des Teilpakets
**
** Seiteneffekte:
**  paket_laenge und paket werden veraendert
*/
static void paket_verlaengern(int *paket_laenge, void **paket, int kennung,
	int daten_laenge, void *daten)
{
	int laenge; /* Bytes, um die das Paket verlaengert wird */

	/* je nach Kennung entweder nur die Kennung (ein Byte) oder
	   Kennung, Laenge der daten und Daten anhaengen
	   (daten_laenge + 2 Bytes) */
	laenge = DATENLAENGE_VARIABEL(kennung) ? daten_laenge + 2 : 1;

	/* falls das Paket bisher Laenge Null hat, neu anlegen,
	   sonst vergroessern */
	if (*paket_laenge)
		speicher_vergroessern(paket, *paket_laenge + laenge);
	else
		speicher_belegen(paket, *paket_laenge + laenge);

	/* Kennung anhaengen */
	((u_char *)*paket)[(*paket_laenge)++] = kennung;

	/* auch Daten anhaengen? */
	if (DATENLAENGE_VARIABEL(kennung))
	{
		/* Laenge der Daten anhaengen */
		((u_char *)*paket)[(*paket_laenge)++] = daten_laenge;

		/* Daten anhaengen */
		if (daten_laenge)
		{
			memcpy((char *)*paket + *paket_laenge, daten, daten_laenge);
			*paket_laenge += daten_laenge;
		}
	}
}


/* fuer qsort */
static int vergleich_gegner(const void *g1, const void *g2)
{
	return memcmp(g1, g2, 6);
}


/* fuer qsort */
static int vergleich_schuesse(const void *s1, const void *s2)
{
	return memcmp(s1, s2, 5);
}


/*
** schuss_ausloesen
**  startet einen Schuss
**
** Parameter:
**  spieler_nr: Nummer des Besitzers
**  blick: Schussrichtung
*/
static void schuss_ausloesen(int spieler_nr, int blick)
{
	/* Schuss jetzt unterwegs */
	schuss_fliegt[spieler_nr] = 1;

	/* Position und Blickrichtung vom Spieler kopieren */
	schuss_blick[spieler_nr] = blick;
	memcpy(&alle_schuesse[spieler_nr].pos,
		&alle_spieler[spieler_nr].pos, sizeof(struct position));

	/* Schuss zur Nase bewegen */
	SUBX(alle_schuesse[spieler_nr].pos,
		sintab[schuss_blick[spieler_nr] * (TRIGANZ / WINKANZ)] *
		(KUGELRAD - SCHUSSRAD - 1) / TRIGFAKTOR);
	SUBY(alle_schuesse[spieler_nr].pos,
		costab[schuss_blick[spieler_nr] * (TRIGANZ / WINKANZ)] *
		(KUGELRAD - SCHUSSRAD - 1) / TRIGFAKTOR);

	/* Ereignis merken */
	ereignisse[spieler_nr][SCHUSS_LOS_ERG] = 1;
}


/*
** einen_schuss_bewegen
**  bewegt einen Schuss
**
** Parameter:
**  besitzer: Nummer des Besitzers
**
** Seiteneffekte:
**  alle_schuesse, schuss_fliegt und ereignisse werden veraendert
*/
static void einen_schuss_bewegen(int besitzer)
{
	/* alle_schuesse[besitzer] neu berechnen */
	switch (bewege_schuss(besitzer, &alle_schuesse[besitzer],
		&schuss_blick[besitzer]))
	{
		case SCHUSS_DANEBEN: /* Wand getroffen */

			/* Schuss ist weg */
			schuss_fliegt[besitzer] = 0;

			/* Ereignis merken */
			ereignisse[besitzer][SCHUSS_WEG_ERG] = 1;

			break;

		case SCHUSS_PRALLT_AB: /* prallt an Wand ab */

			/* Ereignis merken */
			ereignisse[besitzer][SCHUSS_PRALL_ERG] = 1;

			break;

		case SCHUSS_TREFFER: /* Gegner getroffen */

			/* Schuss ist weg */
			schuss_fliegt[besitzer] = 0;

			/* Ereignis merken */
			ereignisse[besitzer][ABSCHUSS_ERG] = 1;

			break;
	}
}


/* bis hier lokaler Teil                       */
/***********************************************/
/* ab hier globaler Teil                       */


/*
** init_bewegung
**  initialisiert den Bewegungsteil
**
** Seiteneffekte:
**  spieler_aktiv wird gesetzt und Speicher fuer gegner_naehe wird belegt
*/
void init_bewegung(void)
{
	int i; /* Index fuer Spieler und Zeilen von gegner_naehe */

	/* Zufallsgenerator initialisieren (fuer spieler_beamen) */
	zufall_init(time(NULL));

	/* alle Spieler auf noch nicht aktiv setzen */
	for (i = 0; i < SPIELERANZ; i++)
		spieler_aktiv[i] = 0;

	/* Speicher fuer gegner_naehe belegen (2-dimensional) */
	speicher_belegen((void **)&gegner_naehe, feldbreite * sizeof(char *));

	for (i = 0; i < feldbreite; i++)
		speicher_belegen((void **)&gegner_naehe[i],
			feldlaenge * sizeof(char));
}


/*
** spieler_ende
**  streicht einen Spieler aus der Liste der existierenden Spieler
**
** Parameter:
**  spieler_nr: Nummer des Spielers
**
** Seiteneffekte:
**  spieler_aktiv wird gesetzt
*/
void spieler_ende(int spieler_nr)
{
	int i; /* Index fuer Spieler */

	/* Spieler nicht mehr aktiv? */
	if (!spieler_aktiv[spieler_nr])

		/* dann ignorieren */
		return;

	/* Spieler ist nicht mehr aktiv */
	spieler_aktiv[spieler_nr] = 0;

	/* hatte Gegner des Spielers einen Spruch? */
	if (abgeschossen_durch[spieler_nr] >= 0 &&
		abgeschossen_spruch_laenge[spieler_nr])

		/* dann Speicher freigeben */
		speicher_freigeben((void **)&abgeschossen_spruch[spieler_nr]);

	/* hatte Spieler einen Spruch? */
	if (spieler_spruch[spieler_nr] != NULL)

		/* dann Speicher freigeben */
		speicher_freigeben((void **)&spieler_spruch[spieler_nr]);

	/* Punkte loeschen */
	spieler_punkte[spieler_nr] = SPIELER_NICHT_AKTIV;

	/* Punktestand an alle Spieler senden */
	for (i = 0; i < SPIELERANZ; i++)
		if (spieler_aktiv[i])
			punkte_senden[i][spieler_nr] = NEU_SENDEN;
}


/*
** spieler_start
**  initialisiert einen neuen Spieler
**
** Parameter:
**  spieler_nr: Nummer des Spielers
**  spruch_laenge: Laenge des Spruchs, 0 falls unbekannt
**  spruch: Spruch des Spielers, NULL falls unbekannt
**  kamera: Flag, ob der Spieler nur zusehen will
**
** Seiteneffekte:
**  spieler_aktiv, spieler_tot, abgeschossen_durch, schuss_fliegt,
**  alle_spieler, alle_schuesse, spieler_spruch_laenge, spieler_spruch,
**  spieler_punkte, alte_ereignisse und punkte_senden werden initialisiert
*/
void spieler_start(int spieler_nr, int spruch_laenge, char *spruch, int kamera)
{
	int i; /* Index fuer die Ereignisse und Spieler */

	/* Spieler existiert, aber muss die uebliche Zeit warten,
	   bis er in das Spiel kommt */
	spieler_aktiv[spieler_nr] = 1;
	spieler_tot[spieler_nr] = SPIELER_TOT_ZEIT;

	/* Kameras muessen nicht warten */
	if (kamera)
		spieler_tot[spieler_nr] = 1;

	/* Spieler wurde nicht abgeschossen und hat noch nicht selbst
	   geschossen */
	abgeschossen_durch[spieler_nr] = -1;
	schuss_fliegt[spieler_nr] = 0;

	/* Farbe des Spielers und seines Schusses setzen */
	alle_spieler[spieler_nr].farbe =
		alle_schuesse[spieler_nr].farbe = spieler_nr + 1;

	/* gekaufte Optionen hardcoden */
	opt_faceless[spieler_nr] = default_faceless;
	opt_colorless[spieler_nr] = default_colorless;
	opt_reflective[spieler_nr] = default_reflective;
	opt_quickturn[spieler_nr] = default_quickturn;
	opt_autoanswer[spieler_nr] = default_autoanswer;
	opt_invisible[spieler_nr] = default_invisible;

	ist_kamera[spieler_nr] = kamera;

	/* Spruch und Laenge des Spruchs merken */
	spieler_spruch_laenge[spieler_nr] = spruch_laenge;
	spieler_spruch[spieler_nr] = spruch;

	/* Punkte initialisieren */
	spieler_punkte[spieler_nr] = 0;

	/* alte_ereignisse loeschen */
	for (i = 0; i < ERG_ANZ; i++)
		alte_ereignisse[spieler_nr][i] = 0;

	/* Punktestand dieses Spielers an alle Spieler und aller aktiven
	   Spieler an diesen Spieler senden, ausser bei Kameras */
	for (i = 0; i < SPIELERANZ; i++)
		if (spieler_aktiv[i])
		{
			punkte_senden[spieler_nr][i] =
				ist_kamera[i] ? NICHT_SENDEN : NEU_SENDEN;

			punkte_senden[i][spieler_nr] =
				kamera ? NICHT_SENDEN : NEU_SENDEN;
		}
		else
			punkte_senden[spieler_nr][i] = NICHT_SENDEN;
}


/*
** spieler_suspendieren
**  suspendiert einen Spieler, von dem keine Daten mehr gekommen sind;
**  muss fuer einen Spieler jedesmal nach init_runde erneut aufgerufen
**  werden
**
** Parameter:
**  spieler_nr: Nummer des Spielers
**
** Seiteneffekte:
**  spieler_tot und ereignisse werden gesetzt
*/
void spieler_suspendieren(int spieler_nr)
{
	/* macht der Spieler gerade freiwillig Pause? */
	if (spieler_tot[spieler_nr] < 0)
		return;

	/* Spieler nach der ueblichen Zeit wieder ins Spiel nehmen,
	   sowie er wieder von sich hoeren laesst */
	spieler_tot[spieler_nr] = SPIELER_TOT_ZEIT;

	/* Ereignis merken */
	ereignisse[spieler_nr][SUSPENDIERT_ERG] = 1;
}


/*
** init_runde
**  initialisiert die Daten fuer einen Bewegungsschritt
**  (bewegung, schuss_bewegung und paket_bauen)
**
** Seiteneffekte:
**  ereignisse und autoanswer_ausgeloest werden gesetzt, spieler_tot veraendert
*/
void init_runde(void)
{
	int i; /* Index fuer die Spieler */

	/* alle Spieler initialisieren */
	for (i = 0; i < SPIELERANZ; i++)
		/* existiert der Spieler? */
		if (spieler_aktiv[i])
		{
			int j; /* Index fuer die Ereignisse */

			autoanswer_ausgeloest[i] = 0;

			/* noch keine Ereignisse bekannt */
			for (j = 0; j < ERG_ANZ; j++)
				ereignisse[i][j] = 0;

			/* wenn der Spieler tot ist, die verbleibende Zeit
			   herunterzaehlen */
			if (spieler_tot[i] > 0 && !--spieler_tot[i])
			{
				/* verbleibende Zeit ist Null, Spieler wieder in
				   das Spiel bringen */
				if (!spieler_beamen(i))
					/* Spieler ist wieder im Spiel */
					ereignisse[i][ERWACHT_ERG] = 1;
				else
					/* fehlgeschlagen, in der naechsten Runde wieder
					   probieren */
					spieler_tot[i] = 1;
			}
		}
}


/*
** bewegung
**  wertet ein Paket von einem Client aus
**
** Parameter:
**  spieler_nr: Nummer des Spielers
**  paket_laenge: Laenge des Pakets
**  paket: das Paket
**  client_ende: Zeiger auf ein Flag, ob der Client das Spiel verlassen will
**
** Seiteneffekte:
**  *client_ende wird gesetzt; spieler_tot, alle_spieler, schuss_fliegt,
**  alle_schuesse, schuss_blick und ereignisse werden veraendert
*/
void bewegung(int spieler_nr, int paket_laenge, void *paket, int *client_ende)
{
	char signale[SIGNALANZ]; /* Steuersignale */
	u_char *tmp_paket;       /* Kopie des Zeigers paket mit anderem Typ */
	int i;                   /* Index fuer Steuersignale/Bytes im Paket */

	tmp_paket = paket; /* Zeiger kopieren */

	/* Steuersignale initialisieren */
	for (i = 0; i < SIGNALANZ; i++)
		signale[i] = 0;

	/* das ganze Paket durchlaufen */
	for (i = 0; i < paket_laenge; i++)

		/* hat das Teilpaket variable Laenge? */
		if (DATENLAENGE_VARIABEL(tmp_paket[i]))
		{
			/* ist das Teilpaket nicht vollstaendig? */
			if (i == paket_laenge - 1 ||
				i + (int)tmp_paket[i + 1] >= paket_laenge - 1)
				/* dann fertig ausgewertet */
				break;

			/* enthaelt das Teilpaket Steuersignale? */
			if (tmp_paket[i] == BK_SIGNALE)
			{
				u_int j; /* Index fuer die Steuersignale im Teilpaket */

				/* alle Datenbytes auswerten */
				for (j = 0; j < tmp_paket[i + 1]; j++)

					/* gueltiges Steuersignal? */
					if (tmp_paket[i + j + 2] < (u_char)SIGNALANZ)
						/* Steuersignal merken */
						signale[tmp_paket[i + j + 2]] = 1;
			}

			/* dieses Teilpaket ueberspringen */
			i += 1 + tmp_paket[i + 1];
		}
		/* will der Client das Spiel verlassen? */
		else if (tmp_paket[i] == BK_ENDE_SIGNAL)
		{
			/* Flag zurueckgeben */
			*client_ende = 1;

			/* und Steuersignale nicht weiter auswerten */
			return;
		}

	/* will der Spieler eine Pause machen? */
	if (signale[STOPSIGNAL] && spieler_tot[spieler_nr] >= 0)
	{
		/* Spieler macht Pause */
		spieler_tot[spieler_nr] = -1;

		/* Ereignis merken */
		ereignisse[spieler_nr][PAUSE_ERG] = 1;
	}

	/* will der Spieler eine Pause beenden? */
	if (signale[WEITERSIGNAL] && spieler_tot[spieler_nr] < 0)
		/* dann noch die uebliche Zeit warten, ausser bei Kameras */
		spieler_tot[spieler_nr] = ist_kamera[spieler_nr] ? 1 :
			SPIELER_TOT_ZEIT;

	/* ist der Spieler tot? dann weder bewegen, noch schiessen */
	if (spieler_tot[spieler_nr])
		return;

	if (opt_quickturn[spieler_nr] && signale[QUICKTURNSIGNAL])
		alle_spieler[spieler_nr].blick =
			(alle_spieler[spieler_nr].blick +
				WINKANZ / 2) % WINKANZ;

	/* Spieler bewegen */
	bewege_spieler(spieler_nr, &alle_spieler[spieler_nr],
		signale[VORSIGNAL], signale[ZURUECKSIGNAL],
		signale[LINKSSIGNAL], signale[RECHTSSIGNAL]);

	/* Schuss ausloesen?  Kameras duerfen nicht */
	if (!ist_kamera[spieler_nr] && signale[SCHUSSSIGNAL])
		schuss_ausloesen(spieler_nr, alle_spieler[spieler_nr].blick);
}


/*
** paket_bauen
**  stellt ein Datenpaket fuer einen Client zusammen
**
** Parameter:
**  spieler_nr: Nummer des Spielers
**  paket_laenge: Zeiger auf die Laenge des Pakets
**  paket: Zeiger auf das Paket
**  client_ende: Flag, ob der Client ein BK_ENDE_SIGNAL gesendet hat
**  paket_nr: Nummer des Pakets
**
** Seiteneffekte:
**  *paket_laenge und *paket werden gesetzt; fuer *paket wird
**  Speicher belegt; alte_ereignisse und ereignis_paket_nr werden gesetzt
*/
void paket_bauen(int spieler_nr, int *paket_laenge, void **paket,
	int client_ende, int paket_nr)
{
	u_char spieler_pos_daten[6]; /* Puffer fuer die Position des Spielers */
	u_char schuss_pos_daten[5];  /* Puffer fuer die Position
	                                seines Schusses */
	u_char *daten;               /* allgemeiner Puffer */
	int i, j;                    /* allgemeine Indizes */
	int gegneranz;               /* Anzahl der zur Zeit aktiven
	                                gegnerischen Spieler */
	int schussanz;               /* Anzahl der zur Zeit aktiven
	                                gegnerischen Schuesse */
	int erg_anz;                 /* Anzahl der gemerkten Ereignisse */
	int punkte_anz;              /* Anzahl der zu uebertragenden
	                                Punktestaende */

	/* Paketlaenge initialisieren */
	*paket_laenge = 0;

	/* hat der Client BK_ENDE_SIGNAL gesendet? */
	if (client_ende)
	{
		/* mit BK_ENDE_DURCH_CLIENT beantworten */
		paket_verlaengern(paket_laenge, paket, BK_ENDE_DURCH_CLIENT,
			0, NULL);

		/* keine weiteren Daten senden */
		return;
	}

	/* wenn der Spieler nicht tot ist, seine Position senden */
	if (!spieler_tot[spieler_nr])
	{
		spieler_pos_daten[0] = alle_spieler[spieler_nr].pos.xgrob;
		spieler_pos_daten[1] = alle_spieler[spieler_nr].pos.xfein;
		spieler_pos_daten[2] = alle_spieler[spieler_nr].pos.ygrob;
		spieler_pos_daten[3] = alle_spieler[spieler_nr].pos.yfein;
		spieler_pos_daten[4] = alle_spieler[spieler_nr].farbe;
		spieler_pos_daten[5] = alle_spieler[spieler_nr].blick;

		/* spieler_pos_daten mit Typ BK_SPIELER_POS an
		   das Paket anhaengen */
		paket_verlaengern(paket_laenge, paket, BK_SPIELER_POS,
			sizeof spieler_pos_daten, spieler_pos_daten);
	}

	/* Gegner zaehlen */
	for (i = gegneranz = 0; i < SPIELERANZ; i++)
		if (i != spieler_nr && spieler_aktiv[i] &&
			!spieler_tot[i] && !ist_kamera[i] &&
			!opt_invisible[i])
			gegneranz++;

	/* sind Gegner aktiv? */
	if (gegneranz)
	{
		/* Speicher fuer die Positionen belegen */
		speicher_belegen((void **)&daten, 6 * gegneranz);

		/* Positionen aller aktiven Gegner kopieren */
		for (i = j = 0; i < SPIELERANZ; i++)
			if (i != spieler_nr && spieler_aktiv[i] &&
				!spieler_tot[i] && !ist_kamera[i] &&
				!opt_invisible[i])
			{
				int farbe, blick;

				if (opt_faceless[i])
					blick = 255;
				else
					blick = alle_spieler[i].blick;

				if (opt_colorless[i])
					farbe = 0;
				else
					farbe = alle_spieler[i].farbe;

				daten[6 * j] = alle_spieler[i].pos.xgrob;
				daten[6 * j + 1] = alle_spieler[i].pos.xfein;
				daten[6 * j + 2] = alle_spieler[i].pos.ygrob;
				daten[6 * j + 3] = alle_spieler[i].pos.yfein;
				daten[6 * j + 4] = farbe;
				daten[6 * j + 5] = blick;
				j++;
			}

		/* Sortieren, um Mogeln zu verhindern */
		qsort(daten, gegneranz, 6, vergleich_gegner);

		/* Positionsdaten mit Typ BK_GEGNER_POS
		   an das Paket anhaengen */
		paket_verlaengern(paket_laenge, paket, BK_GEGNER_POS,
			6 * gegneranz, daten);

		/* Speicher wieder freigeben */
		speicher_freigeben((void **)&daten);
	}

	/* wenn ein Schuss des Spielers unterwegs ist, die Position senden */
	if (schuss_fliegt[spieler_nr])
	{
		schuss_pos_daten[0] = alle_schuesse[spieler_nr].pos.xgrob;
		schuss_pos_daten[1] = alle_schuesse[spieler_nr].pos.xfein;
		schuss_pos_daten[2] = alle_schuesse[spieler_nr].pos.ygrob;
		schuss_pos_daten[3] = alle_schuesse[spieler_nr].pos.yfein;
		schuss_pos_daten[4] = alle_schuesse[spieler_nr].farbe;

		/* schuss_pos_daten mit Typ BK_SPIELER_SCHUSS_POS an
		   das Paket anhaengen */
		paket_verlaengern(paket_laenge, paket, BK_SPIELER_SCHUSS_POS,
			sizeof schuss_pos_daten, schuss_pos_daten);
	}

	/* gegnerische Schuesse zaehlen */
	for (i = schussanz = 0; i < SPIELERANZ; i++)
		if (i != spieler_nr && spieler_aktiv[i] && schuss_fliegt[i])
			schussanz++;

	/* sind gegnerische Schuesse unterwegs? */
	if (schussanz)
	{
		/* Speicher fuer die Positionen belegen */
		speicher_belegen((void **)&daten, 5 * schussanz);

		/* Positionen aller aktiven gegnerischen Schuesse kopieren */
		for (i = j = 0; i < SPIELERANZ; i++)
			if (i != spieler_nr && spieler_aktiv[i] &&
				schuss_fliegt[i])
			{
				int farbe;

				if (opt_colorless[i])
					farbe = 0;
				else
					farbe = alle_schuesse[i].farbe;

				daten[5 * j] = alle_schuesse[i].pos.xgrob;
				daten[5 * j + 1] = alle_schuesse[i].pos.xfein;
				daten[5 * j + 2] = alle_schuesse[i].pos.ygrob;
				daten[5 * j + 3] = alle_schuesse[i].pos.yfein;
				daten[5 * j + 4] = farbe;
				j++;
			}

		/* Sortieren, um Mogeln zu verhindern */
		qsort(daten, schussanz, 5, vergleich_schuesse);

		/* Positionsdaten mit Typ BK_GEGNER_SCHUSS_POS an
		   das Paket anhaengen */
		paket_verlaengern(paket_laenge, paket, BK_GEGNER_SCHUSS_POS,
			5 * schussanz, daten);

		/* Speicher wieder freigeben */
		speicher_freigeben((void **)&daten);
	}

	/* wenn der Spieler abgeschossen wurde, Farbe des
	   Gegners und Spruch senden */
	if (abgeschossen_durch[spieler_nr] >= 0)
	{
		u_char *daten; /* Puffer fuer Farbe und Spruch */
		int gegner_nr; /* Nummer des Gegners */

		/* Nummer des Gegners feststellen */
		gegner_nr = abgeschossen_durch[spieler_nr];

		/* Speicher fuer Farbe und Spruch belegen */
		speicher_belegen((void **)&daten,
			abgeschossen_spruch_laenge[spieler_nr] + 1);

		/* Farbe in Puffer schreiben */
		daten[0] = alle_spieler[gegner_nr].farbe;

		/* Spruch in Puffer schreiben */
		if (abgeschossen_spruch_laenge[spieler_nr])
			memcpy(daten + 1, abgeschossen_spruch[spieler_nr],
				abgeschossen_spruch_laenge[spieler_nr]);

		/* Farbe und Spruch mit Typ BK_ABGESCHOSSEN_DURCH an
		   das Paket anhaengen */
		paket_verlaengern(paket_laenge, paket, BK_ABGESCHOSSEN_DURCH,
			abgeschossen_spruch_laenge[spieler_nr] + 1, daten);

		/* Speicher fuer Puffer freigeben */
		speicher_freigeben((void **)&daten);
	}

	/* noch nicht bestaetigte alte Ereignisse zaehlen */
	for (i = erg_anz = 0; i < ERG_ANZ; i++)

		/* ist das Ereignis wieder aufgetreten? */
		if (ereignisse[spieler_nr][i])
			/* dann das alte Ereignis vergessen */
			alte_ereignisse[spieler_nr][i] = 0;
		/* war das Ereignis aufgetreten? */
		else if (alte_ereignisse[spieler_nr][i])
			/* ist es mittlerweile bestaetigt? */
			if (paket_bestaetigt(spieler_nr,
				ereignis_paket_nr[spieler_nr][i]))
				/* dann das alte Ereignis vergessen */
				alte_ereignisse[spieler_nr][i] = 0;
			else
				/* sonst das alte Ereignis zaehlen */
				erg_anz++;

	/* gibt es nicht bestaetigte Ereignisse? */
	if (erg_anz)
	{
		/* Speicher fuer die alten Ereignisse belegen */
		speicher_belegen((void **)&daten, erg_anz);

		/* alte Ereignisse in Puffer kopieren */
		for (i = erg_anz = 0; i < ERG_ANZ; i++)
			if (alte_ereignisse[spieler_nr][i])
				daten[erg_anz++] = i;

		/* alte Ereignisse mit Typ BK_ALTE_EREIGNISSE an
		   das Paket anhaengen */
		paket_verlaengern(paket_laenge, paket, BK_ALTE_EREIGNISSE,
			erg_anz, daten);

		/* Speicher wieder freigeben */
		speicher_freigeben((void **)&daten);
	}

	/* neue Ereignisse zaehlen */
	for (i = erg_anz = 0; i < ERG_ANZ; i++)

		/* Ereignis aufgetreten? */
		if (ereignisse[spieler_nr][i])
		{
			/* Ereignis als altes Ereignis merken */
			alte_ereignisse[spieler_nr][i] = 1;

			/* zugehoerige Paketnummer merken (fuer paket_bestaetigt) */
			ereignis_paket_nr[spieler_nr][i] = paket_nr;

			/* Ereignis zaehlen */
			erg_anz++;
		}

	/* sind Ereignisse aufgetreten? */
	if (erg_anz)
	{
		/* Speicher fuer die Ereignisse belegen */
		speicher_belegen((void **)&daten, erg_anz);

		/* Ereignisse in Puffer kopieren */
		for (i = erg_anz = 0; i < ERG_ANZ; i++)
			if (ereignisse[spieler_nr][i])
				daten[erg_anz++] = i;

		/* Ereignisse mit Typ BK_EREIGNISSE an das Paket anhaengen */
		paket_verlaengern(paket_laenge, paket, BK_EREIGNISSE,
			erg_anz, daten);

		/* Speicher wieder freigeben */
		speicher_freigeben((void **)&daten);
	}

	/* neue Punktestaende zaehlen */
	for (i = punkte_anz = 0; i < SPIELERANZ; i++)

		/* Punkte auf jeden Fall senden? */
		if (punkte_senden[spieler_nr][i] == NEU_SENDEN)
		{
			/* Punkte spaeter wieder senden, falls das Paket nicht
			   bestaetigt wird */
			punkte_senden[spieler_nr][i] = NACHSENDEN;
			punkte_paket_nr[spieler_nr][i] = paket_nr;
			punkte_anz++;
		}

		/* Punkte nur senden, wenn noch nicht bestaetigt? */
		else if (punkte_senden[spieler_nr][i] == NACHSENDEN)

			/* Paket bestaetigt? */
			if (paket_bestaetigt(spieler_nr, punkte_paket_nr[spieler_nr][i]))

				/* dann nicht mehr senden */
				punkte_senden[spieler_nr][i] = NICHT_SENDEN;
			else
				/* sonst senden */
				punkte_anz++;

	/* sind Punktestaende zu senden? */
	if (punkte_anz)
	{
		/* Speicher fuer die Punktestaende belegen */
		speicher_belegen((void **)&daten, 2 * punkte_anz);

		/* Punktestaende in Puffer kopieren */
		for (i = punkte_anz = 0; i < SPIELERANZ; i++)
			if (punkte_senden[spieler_nr][i] == NACHSENDEN)
			{
				int punkte; /* Zwischenspeicher fuer Punkte */

				punkte = spieler_punkte[i];
				daten[punkte_anz++] = i | 0xc0 & punkte >> 2;
				daten[punkte_anz++] = punkte;
			}

		/* Puffer mit Typ BK_PUNKTESTAND an Paket anhaengen */
		paket_verlaengern(paket_laenge, paket, BK_PUNKTESTAND,
			punkte_anz, daten);

		/* Speicher wieder freigeben */
		speicher_freigeben((void **)&daten);
	}
}


/*
** schuss_bewegung
**  verwaltet die Bewegung aller zur Zeit aktiven Schuesse
**
** Seiteneffekte:
**  alle_schuesse, schuss_fliegt und ereignisse werden veraendert
*/
void schuss_bewegung(void)
{
	int i; /* Index fuer die Spieler */

	/* fuer jeden Spieler den Schuss bewegen */
	for (i = 0; i < SPIELERANZ; i++)
		/* existiert der Spieler und ist der Schuss unterwegs? */
		if (spieler_aktiv[i] && schuss_fliegt[i])
			einen_schuss_bewegen(i);

	for (;;)
	{
		int fertig;

		fertig = 1;
		for (i = 0; i < SPIELERANZ; i++)
			if (spieler_aktiv[i] && autoanswer_ausgeloest[i] == 1)
			{
				fertig = 0;
				autoanswer_ausgeloest[i]++;
				schuss_ausloesen(i, autoanswer_blick[i]);
				einen_schuss_bewegen(i);
			}

		if (fertig)
			break;
	}
}
