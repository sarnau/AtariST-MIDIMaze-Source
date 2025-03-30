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
** Datei: rechne_karte.c
**
** Kommentar:
**  Berechnet aus den Labyrinth-Daten den aktuellen Inhalt der Karte
**  und legt die zu zeichnenden Grafik-Objekte in den Datenstrukturen
**  ab (in Bildpunkten)
*/


#include "global.h"
#include "speicher.h"
#include "labyrinth.h"
#include "farben.h"
#include "spieler.h"
#include "grafik.h"
#include "rechne.h"

static char sccsid[] = "@(#)rechne_karte.c	3.2 12/3/01";


/*
** skalier_x
**  skaliert Labyrinth-x-Koordinaten im feinen Raster auf Bildpunkte
**
** Parameter:
**  x: x-Koordinate im feinen Raster
**
** Rueckgabewert:
**  x-Koordinate in Bildpunkten (0 ... FENSTER_BREITE)
*/
static int skalier_x(int x)
{
	return x * (FENSTER_BREITE / RASPT) / feldbreite;
}


/*
** skalier_y
**  skaliert Labyrinth-y-Koordinaten im feinen Raster auf Bildpunkte
**
** Parameter:
**  y: y-Koordinate im feinen Raster
**
** Rueckgabewert:
**  y-Koordinate in Bildpunkten (0 ... FENSTER_HOEHE)
*/
static int skalier_y(int y)
{
	return y * (FENSTER_HOEHE / RASPT) / feldlaenge;
}


/*
** rechnelinie
**  skaliert die Koordinaten zweier aneinanderliegender Waende,
**  bestimmt die jeweiligen Typen (siehe grafik.h) und traegt das
**  berechnete Linienpaar in die Linien-Liste ein
**
** Parameter:
**  anzlinien: Zeiger auf die Anzahl der zu zeichnenden Linienpaare
**  linien: Zeiger auf die Liste der zu zeichnenden Linienpaare
**  x: x-Koordinate des linken/oberen Wandendes im groben Raster
**  y: y-Koordinate des linken/oberen Wandendes im groben Raster
**  senkrecht: Flag, ob das Linienpaar senkrecht verlaeuft
**  wand1: Zeiger auf die eine Wand
**  wand2: Zeiger auf die andere Wand
**
** Seiteneffekte:
**  *anzlinien wird erhoeht, *linien um ein Element verlaengert
*/
static void rechnelinie(int *anzlinien, struct kartenlinie **linien, int x,
	int y, u_char senkrecht, struct wand *wand1, struct wand *wand2)
{
	struct kartenlinie *linie; /* Zeiger auf das neue
	                              Linienpaar in *linien */

	/* falls beide Waende transparent sind, kein Linienpaar eintragen */
	if (wand1->farbe == TRANSPARENT && wand2->farbe == TRANSPARENT)
		return;

	liste_verlaengern(anzlinien, (void **)linien,
		sizeof(struct kartenlinie));
	linie = &(*linien)[*anzlinien - 1];

	/* Koordinaten skalieren */
	linie->ecke.x = skalier_x(x * RASPT);
	linie->ecke.y = skalier_y(y * RASPT);
	linie->laenge = senkrecht ?
		skalier_y((y + 1) * RASPT) - skalier_y(y * RASPT) :
		skalier_x((x + 1) * RASPT) - skalier_x(x * RASPT);
	linie->senkrecht = senkrecht;

	/* Tueren haben Typ TUERLINIE, Waende Typ WANDLINIE, transparente
	   Waende Typ KEINELINIE */
	linie->typ1 = wand1->tuer ? TUERLINIE : wand1->farbe == TRANSPARENT ?
		KEINELINIE : WANDLINIE;
	linie->typ2 = wand2->tuer ? TUERLINIE : wand2->farbe == TRANSPARENT ?
		KEINELINIE : WANDLINIE;
}


/* bis hier lokaler Teil                       */
/***********************************************/
/* ab hier globaler Teil                       */


/*
** rechne_karte_linien
**  berechnet alle auf der Karte zu zeichnenden Linienpaare
**
** Parameter:
**  anzlinien: Zeiger auf die Anzahl der zu zeichnenden Linienpaare
**  linien: Zeiger auf die Liste der zu zeichnenden Linienpaare
**
** Seiteneffekte:
**  *anzlinien und *linien werden gesetzt
*/
void rechne_karte_linien(int *anzlinien, struct kartenlinie **linien)
{
	int x, y; /* x/y-Koordinaten im groben Raster */

	/* berechnet alle Nord- und Suedwaende */
	for (x = 0; x < feldbreite; x++)
	{
		/* die noerdlichste Wand ist einzeln und wird deshalb zweimal
		   uebergeben */
		rechnelinie(anzlinien, linien, x, 0, 0,
			&spielfeld[x][0][NORD],
			&spielfeld[x][0][NORD]);

		/* die uebrigen Nord- und Suedwaende sind jeweils Paare */
		for (y = 1; y < feldlaenge; y++)
			rechnelinie(anzlinien, linien, x, y, 0,
				&spielfeld[x][y - 1][SUED],
				&spielfeld[x][y][NORD]);

		/* die suedlichste Wand ist einzeln und wird deshalb zweimal
		   uebergeben */
		rechnelinie(anzlinien, linien, x, feldlaenge, 0,
			&spielfeld[x][feldlaenge - 1][SUED],
			&spielfeld[x][feldlaenge - 1][SUED]);
	}

	/* berechnet alle West- und Ostwaende */
	for (y = 0; y < feldlaenge; y++)
	{
		/* die westlichste Wand ist einzeln und wird deshalb zweimal
		   uebergeben */
		rechnelinie(anzlinien, linien, 0, y, 1,
			&spielfeld[0][y][WEST],
			&spielfeld[0][y][WEST]);

		/* die uebrigen West- und Ostwaende sind jeweils Paare */
		for (x = 1; x < feldbreite; x++)
			rechnelinie(anzlinien, linien, x, y, 1,
				&spielfeld[x - 1][y][OST],
				&spielfeld[x][y][WEST]);

		/* die oestlichste Wand ist einzeln und wird deshalb zweimal
		   uebergeben */
		rechnelinie(anzlinien, linien, feldbreite, y, 1,
			&spielfeld[feldbreite - 1][y][OST],
			&spielfeld[feldbreite - 1][y][OST]);
	}
}


/*
** rechne_karte_kreise
**  berechnet alle auf der Karte zu zeichnenden Kreise (Spieler)
**
** Parameter:
**  anzkreise: Zeiger auf die Anzahl der zu zeichnenden Kreise
**  kreise: Zeiger auf die Liste der zu zeichnenden Kreise
**  anzspieler: Anzahl der Spieler
**  spieler: Liste der Spieler
**
** Seiteneffekte:
**  *anzkreise und *kreise werden gesetzt
*/
void rechne_karte_kreise(int *anzkreise, struct kartenkreis **kreise,
	int anzspieler, struct spieler *spieler)
{
	int i; /* Index fuer Spieler */

	for (i = 0; i < anzspieler; i++)
	{
		liste_verlaengern(anzkreise, (void **)kreise,
			sizeof(struct kartenkreis));

		/* Koordinaten des Spielers skalieren */
		(*kreise)[*anzkreise - 1].mittelpunkt.x =
			skalier_x(XPOS(spieler[i].pos));
		(*kreise)[*anzkreise - 1].mittelpunkt.y =
			skalier_y(YPOS(spieler[i].pos));

		/* Radius des Spielers skalieren */
		(*kreise)[*anzkreise - 1].radiusx = skalier_x(KUGELRAD);
		(*kreise)[*anzkreise - 1].radiusy = skalier_y(KUGELRAD);

		/* Farbe direkt kopieren */
		(*kreise)[*anzkreise - 1].farbe = spieler[i].farbe;
	}
}
