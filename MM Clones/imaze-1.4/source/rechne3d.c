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
** Datei: rechne3d.c
**
** Kommentar:
**  Berechnet aus den Labyrinth-Koordinaten der Spieler die aktuelle
**  3D-Sicht und legt die zu zeichnenden Grafik-Objekte in den
**  Datenstrukturen ab (in Bildpunkten)
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "speicher.h"
#include "labyrinth.h"
#include "farben.h"
#include "spieler.h"
#include "grafik.h"
#include "rechne.h"

static char sccsid[] = "@(#)rechne3d.c	3.7 12/3/01";


/* Werte fuer richtung in Struktur objekt; gibt an, an welcher Stelle beim
   Zeichnen der Waende die Spieler/Schuesse einsortiert werden sollen */
#define VORHER      0 /* vor diesem Durchgang einsortieren */
#define RECHTS_OST  1 /* zwischen den rechts liegenden Ostwaenden */
#define LINKS_WEST  2 /* zwischen den links liegenden Westwaenden */
#define RECHTS_NORD 3 /* zwischen den rechts liegenden Nordwaenden */
#define LINKS_NORD  4 /* zwischen den links liegenden Nordwaenden */

/* Werte fuer soll und Indizes fuer durchgang, richtung und
   index in Struktur objekt */
#define AKTUELL         0 /* aktuelle Werte */
#define FRUEHER         1 /* Werte, falls Kugel frueher einsortiert wird */
#define SPAETER         2 /* Werte, falls Kugel spaeter einsortiert wird */
#define KEINE_KORREKTUR 3 /* Kugel nicht frueher oder spaeter einsortieren */


/* Struktur fuer temporaere Speicherung von Spielern und Schuessen */
struct objekt
{
	struct position pos; /* wie in struct spieler/struct schuss */
	s_char blick;        /* -1 fuer Schuesse, sonst wie in struct spieler */
	u_char farbe;        /* wie in struct spieler/struct schuss */
	u_char radius;       /* KUGELRAD oder SCHUSSRAD */
	int abstandq;        /* Quadrat des Abstandes von 'ich' */

	/* wann soll der Spieler/Schuss gezeichnet werden? */
	int durchgang[3]; /* Nummer des Durchgangs beim Waendezeichnen:
	                     |delta x| + |delta y| */
	int richtung[3];  /* Richtung der Waende: VORHER, RECHTS_OST,
	                     LINKS_WEST, RECHTS_NORD oder LINKS_NORD;
	                     siehe oben */
	int index[3];     /* Nummer der Wand; siehe rechne_alles */
	int kann_frueher; /* Flag: kann Spieler/Schuss auch frueher
	                     gezeichnet werden? */
	int kann_spaeter; /* Flag: kann Spieler/Schuss auch spaeter
	                     gezeichnet werden? */
	int soll;         /* Flag: soll Spieler/Schuss frueher/spaeter
	                     gezeichnet werden? kann FRUEHER, SPAETER oder
	                     KEINE_KORREKTUR sein */
};


/*
** projx
**  berechnet x-Koordinate der Zentralprojektion
**
** Parameter:
**  x: Abstand von 'ich' nach rechts
**  y: Abstand von 'ich' in Blickrichtung
**
** Rueckgabewert:
**  x-Koordinate in Bildpunkten (0 ... FENSTER_BREITE)
*/
static int projx(int x, int y)
{
	return x * (FENSTER_BREITE / 2 + 1) / y + FENSTER_BREITE / 2;
}


/*
** projy
**  berechnet y-Koordinate der Zentralprojektion
**
** Parameter:
**  y: Abstand von 'ich' in Blickrichtung
**  z: Abstand von 'ich' nach oben geteilt durch WANDH/2
**
** Rueckgabewert:
**  y-Koordinate in Bildpunkten (0 ... FENSTER_HOEHE)
*/
static int projy(int y, int z)
{
	return -z * (WANDH / 2) * (FENSTER_HOEHE / 2 + 1) / y +
		FENSTER_HOEHE / 2;
}


/*
** drehx
**  berechnet x-Koordinate des um den Winkel w gedrehten Punktes (x,y)
**
** Parameter:
**  x: alte x-Koordinate
**  y: alte y-Koordinate
**  w: Drehwinkel in WINKANZ Schritten pro Vollkreis
**
** Rueckgabewert:
**  neue x-Koordinate
*/
static int drehx(int x, int y, u_char w)
{
	return (x * costab[(int)w * (TRIGANZ / WINKANZ)] +
		y * sintab[(int)w * (TRIGANZ / WINKANZ)]) / TRIGFAKTOR;
}


/*
** drehy
**  berechnet y-Koordinate des um den Winkel w gedrehten Punktes (x,y)
**
** Parameter:
**  x: alte x-Koordinate
**  y: alte y-Koordinate
**  w: Drehwinkel in WINKANZ Schritten pro Vollkreis
**
** Rueckgabewert:
**  neue y-Koordinate
*/
static int drehy(int x, int y, u_char w)
{
	return (y * costab[(int)w * (TRIGANZ / WINKANZ)] -
		x * sintab[(int)w * (TRIGANZ / WINKANZ)]) / TRIGFAKTOR;
}


/*
** drehblick
**  berechnet die relative Blickrichtung einer Kugel
**
** Parameter:
**  kugelx: relative x-Koordinate der Kugel
**  kugely: relative y-Koordinate der Kugel
**  kugelblick: absolute Blickrichtung der Kugel
**              oder -1, falls Kugel kein Gesicht hat
**  ichblick: absolute Blickrichtung des Spielers
**
** Rueckgabewert:
**  relative Blickrichtung der Kugel oder -1
*/
static int drehblick(int kugelx, int kugely, int kugelblick, int ichblick)
{
	int x;         /* Abstand der Kugel von 'ich' nach rechts */
	int y;         /* Abstand der Kugel von 'ich' in Blickrichtung */
	int korrektur; /* Korrekturwinkel fuer Quadranten */

	/* Position der Kugel drehen */
	x = drehx(kugelx, kugely, ichblick);
	y = drehy(kugelx, kugely, ichblick);

	/* hat Kugel kein Gesicht oder steht er auf demselben Punkt? */
	if (kugelblick < 0 || (x == 0 && y == 0))
		return -1;

	/* Position um TRIGANZ/4 drehen, bis y > abs(x) */
	korrektur = 0;
	if (y < x)
	{
		korrektur = TRIGANZ / 2;
		x = -x;
		y = -y;
	}
	if (y < -x)
	{
		int tmp;

		korrektur += 3 * TRIGANZ / 4;
		tmp = x;
		x = -y;
		y = tmp;
	}

	/* Blickrichtung berechnen */
	return (atantab[x * (TRIGANZ / 2) / y + TRIGANZ / 2] + korrektur +
		(kugelblick - ichblick) * (TRIGANZ / WINKANZ) + TRIGANZ) % TRIGANZ;
}


/*
** ausschnitt_eintragen
**  einen weiteren sichtbaren Ausschnitt einer Kugel in die
**  Clipping-Liste eintragen
**
** Parameter:
**  sichtanz: Zeiger auf die Anzahl der sichtbaren Ausschnitte
**  sichtbar: Zeiger auf die Liste der sichtbaren Ausschnitte
**  von: x-Koordinate des linken Randes des Ausschnitts in Bildpunkten
**  bis: x-Koordinate des rechten Randes des Ausschnitts in Bildpunkten
**
** Seiteneffekte:
**  *sichtanz wird erhoeht, *sichtbar um ein Element verlaengert
*/
static void ausschnitt_eintragen(int *sichtanz, struct ausschnitt **sichtbar,
	int von, int bis)
{
	liste_verlaengern(sichtanz, (void **)sichtbar,
		sizeof(struct ausschnitt));

	(*sichtbar)[*sichtanz - 1].x = von;
	(*sichtbar)[*sichtanz - 1].breite = bis - von;
}


/*
** rechnekugel
**  eine weitere Kugel projezieren, Clipping-Liste berechnen und,
**  sofern sichtbar, an die Kugel-Liste anhaengen
**
** Parameter:
**  anzwaende: Anzahl der sichtbaren Waende
**  waende: Liste der sichtbaren Waende (fuer Clipping)
**  anzkugeln: Zeiger auf die Anzahl der sichtbaren Kugeln
**  kugeln: Zeiger auf die Liste der sichtbaren Kugeln
**  x: Abstand der Kugel von 'ich' nach rechts
**  y: Abstand der Kugel von 'ich' in Blickrichtung
**  blick: Blickrichtung der Kugel relativ zur Blickrichtung von 'ich'
**  farbe: Farbe der Kugel
**  radius: Radius der Kugel im feinen Raster
**
** Seiteneffekte:
**  *anzkugeln wird erhoeht, *kugeln um ein Element verlaengert
*/
static void rechnekugel(int anzwaende, struct flaeche *waende, int *anzkugeln,
	struct kugel **kugeln, int x, int y, int blick, u_char farbe,
	u_char radius)
{
	struct ausschnitt *sichtbar; /* Clipping-Liste */
	int sichtanz;                /* Laenge der Clipping-Liste */
	int mittelpunktx;            /* x-Koordinate des Kugelmittelpunkt in
	                                Bildpunkten */
	int radiusx;                 /* Kugelradius in x-Richtung
	                                in Bildpunkten */

	/* falls Kugel hinter 'ich', nicht zeichnen */
	if (y <= 0)
		return;

	/* falls Kugel nicht im 90-Grad-Blickfeld, nicht zeichnen
	   sqrt(2) und 557 / 408 sind beide 1.41421... */
	if (x - y > (int)radius * 557 / 408 || x + y < -(int)radius * 557 / 408)
		return;

	mittelpunktx = projx(x, y);

	{
		int dist;

		dist = wurzel((long)x * (long)x + (long)y * (long)y, W_RUNDEN);
		radiusx = projx(radius, dist) - projx(0, dist);
	}

	liste_initialisieren(&sichtanz, (void **)&sichtbar);

	{
		int links; /* linker Rand des aktuellen sichtbaren Auschnitts */
		int i;     /* Index fuer Waende */

		/* ganz links anfangen */
		links = 0;

		/* lineare Suche, koennte man durch
		   vorherige binaere Suche optimieren */
		for (i = 0; i < anzwaende &&
			links <= min(mittelpunktx + radiusx, FENSTER_BREITE - 1); i++)
		{
			int rechts; /* rechter Rand des sichtbaren Auschnitts */

			rechts = min(waende[i].ecke[0].x, FENSTER_BREITE);

			/* Ausschnitte, die ganz links der Kugel liegen,
			   nicht eintragen */
			if (waende[i].ecke[0].x > links &&
				rechts >= mittelpunktx - radiusx)
				ausschnitt_eintragen(&sichtanz, &sichtbar, links, rechts);

			/* ab neuem 'links' weitersuchen */
			links = max(waende[i].ecke[2].x, links);
		}

		/* letzten Ausschnitt eintragen */
		if (FENSTER_BREITE > links && links <= mittelpunktx + radiusx)
			ausschnitt_eintragen(&sichtanz, &sichtbar,
				links, FENSTER_BREITE);
	}

	/* falls kein sichtbarer Bereich, Kugel nicht eintragen */
	if (sichtanz == 0)
	{
		liste_freigeben(&sichtanz, (void **)&sichtbar);
		return;
	}

	liste_verlaengern(anzkugeln, (void **)kugeln, sizeof(struct kugel));

	{
		struct kugel *kugel; /* Zeiger auf die neue Kugel in *kugeln */

		kugel = &(*kugeln)[*anzkugeln - 1];

		if (farbe == TRANSPARENT || y <= (int)radius)
			/* transparente Kugeln haben keinen Schatten: */
			kugel->schatteny = kugel->schattenry = 0;
		else
		{
			int schatten_oben, schatten_unten;

			/* Schatten projezieren
			   (Pfusch, in Wirklichkeit keine Ellipse) */
			schatten_oben = projy(y + radius, -1);
			schatten_unten = projy(y - radius, -1);
			kugel->schattenry = (schatten_unten - schatten_oben) / 2;
			kugel->schatteny = schatten_unten - kugel->schattenry;
		}

		kugel->mittelpunkt.x = mittelpunktx;
		kugel->mittelpunkt.y = projy(y, 0);
		kugel->radiusx = radiusx;
		/* Breiten-/Hoehen-Verhaeltnis des Fensters muss 3/2 sein,
		   damit die Kugeln rund werden */
		kugel->radiusy = radiusx * 3 / 2;
		kugel->blick = blick;
		kugel->farbe = farbe;
		kugel->sichtbar = sichtbar;
		kugel->sichtanz = sichtanz;
	}
}


/*
** dreh_rechne_kugel
**  eine Kugel drehen und zum Projezieren an rechnekugel uebergeben
**
** Parameter:
**  anzwaende: Anzahl der sichtbaren Waende
**  waende: Liste der sichtbaren Waende (fuer Clipping)
**  anzkugeln: Zeiger auf die Anzahl der sichtbaren Kugeln
**  kugeln: Zeiger auf die Liste der sichtbaren Kugeln
**  objekt: Zeiger auf das zu projezierende Objekt
**  ichpos: gedrehte Position des Spielers
**  ichblick: gedrehte Blickrichtung des Spielers
**
** Seiteneffekte:
**  *anzkugeln und *kugeln werden veraendert
*/
static void dreh_rechne_kugel(int anzwaende, struct flaeche *waende,
	int *anzkugeln, struct kugel **kugeln, struct objekt *objekt,
	struct position *ichpos, int ichblick)
{
	rechnekugel(anzwaende, waende, anzkugeln, kugeln,
		drehx((XPOS(objekt->pos) - XPOS(*ichpos)),
			(YPOS(*ichpos) - YPOS(objekt->pos)), ichblick),
		drehy((XPOS(objekt->pos) - XPOS(*ichpos)),
			(YPOS(*ichpos) - YPOS(objekt->pos)), ichblick),
		drehblick(XPOS(objekt->pos) - XPOS(*ichpos),
			YPOS(*ichpos) - YPOS(objekt->pos), objekt->blick, ichblick),
		objekt->farbe, objekt->radius);
}


/*
** rechnewand
**  eine weitere Wand projezieren und, sofern sichtbar,
**  in die sortierte Wand-Liste eintragen
**
** Parameter:
**  anzwaende: Zeiger auf die Anzahl der sichtbaren Waende
**  waende: Zeiger auf die Liste der sichtbaren Waende
**  x1: Abstand des einen Wandendes von 'ich' nach rechts
**  y1: Abstand des einen Wandendes von 'ich' in Blickrichtung
**  x2: Abstand des anderen Wandendes von 'ich' nach rechts
**  y2: Abstand des anderen Wandendes von 'ich' in Blickrichtung
**  farbe: Farbe der Wand
**  tuer: Flag, ob Wand eine Tuer ist
**
** Seiteneffekte:
**  *anzwaende wird erhoeht, *waende um ein Element verlaengert
*/
static void rechnewand(int *anzwaende, struct flaeche **waende, int x1, int y1,
	int x2, int y2, u_char farbe, u_char tuer)
{
	struct flaeche wand; /* gesamte Wand, projeziert */

	if (farbe == TRANSPARENT)
		return;

	/* falls Wand rechts des 90-Grad-Blickfeldes, nicht zeichnen */
	if (x1 >= y1 && x2 >= y2)
		return;

	/* falls Wand rechte Grenze des Blickfeldes schneidet, abschneiden */
	if (x1 >= y1 || x2 >= y2)
	{
		int m, e, d;

		e = (x1 * y2 - x2 * y1);
		d = (x1 - x2 + y2 - y1);
		m = (e + d / 2) / d;

		/* erstes oder zweites Wandende veraendern? */
		if (x1 >= y1)
			x1 = y1 = m;
		else
			x2 = y2 = m;
	}

	/* falls Wand links des 90-Grad-Blickfeldes, nicht zeichnen */
	if (x1 <= -y1 && x2 <= -y2)
		return;

	/* falls Wand linke Grenze des Blickfeldes schneidet, abschneiden */
	if (x1 <= -y1 || x2 <= -y2)
	{
		int m, e, d;

		e = (x1 * y2 - x2 * y1);
		d = (x1 - x2 + y1 - y2);
		m = (e + d / 2) / d;

		/* erstes oder zweites Wandende veraendern? */
		if (x1 <= -y1)
			x1 = -m, y1 = m;
		else
			x2 = -m, y2 = m;
	}

	/* falls Wand hinter oder auf Bildebene, korrigieren */
	if (y1 <= 0)
		y1 = 1;
	if (y2 <= 0)
		y2 = 1;

	/* projezieren */
	wand.ecke[0].x = wand.ecke[1].x = projx(x1, y1);
	wand.ecke[0].y = projy(y1, 1);
	wand.ecke[1].y = projy(y1, -1);
	wand.ecke[2].x = wand.ecke[3].x = projx(x2, y2);
	wand.ecke[2].y = projy(y2, -1);
	wand.ecke[3].y = projy(y2, 1);
	wand.farbe = farbe;
	wand.tuer = tuer;

	/* ecke[0] und ecke[1] links, ecke[2] und ecke[3] rechts,
	   sonst tauschen */
	if (wand.ecke[0].x > wand.ecke[2].x)
	{
		int tmp;

		tmp = wand.ecke[0].x;
		wand.ecke[0].x = wand.ecke[1].x = wand.ecke[2].x;
		wand.ecke[2].x = wand.ecke[3].x = tmp;
		tmp = wand.ecke[0].y;
		wand.ecke[0].y = wand.ecke[3].y;
		wand.ecke[3].y = tmp;
		tmp = wand.ecke[1].y;
		wand.ecke[1].y = wand.ecke[2].y;
		wand.ecke[2].y = tmp;
	}

	/* alle sichtbaren Ausschnitte suchen, bei spaeteren Durchlaeufen
	   werden die frueheren Ausschnitte durch sich selbst verdeckt */
	for (;;)
	{
		int i;                     /* Index der Wand, vor der
		                              eingefuegt werden muss */
		struct flaeche ausschnitt; /* ein sichtbarer Ausschnitt der Wand */

		/* lineare Suche, koennte durch binaere Suche optimiert werden,
		   gesucht wird die erste Wand, die weiter rechts
		   anfaengt als 'wand' */
		for (i = 0; i < *anzwaende && (*waende)[i].ecke[0].x <=
			wand.ecke[0].x; i++);

		{
			int links, rechts; /* die Grenzen des sichtbaren Ausschnitts
			                      in Bildpunkten */

			if (i == 0)
				/* falls links von allen Waenden */
				links = wand.ecke[0].x;
			else
			{
				/* erste Luecke in den naeher liegenden Waenden suchen,
				   durch die man 'wand' sehen oder rechts an 'wand'
				   vorbeisehen kann */
				while (i < *anzwaende &&
					(*waende)[i - 1].ecke[2].x == (*waende)[i].ecke[0].x)
					i++;

				/* Luecke ist rechts von 'wand', kein weiterer
				   sichtbarer Ausschnitt */
				if ((*waende)[i - 1].ecke[2].x >= wand.ecke[2].x)
					/* hier verlassen, wer findet schon diese Zeile? */
					return;

				/* Anfang der Luecke */
				links = max(wand.ecke[0].x, (*waende)[i - 1].ecke[2].x);
			}

			if (i >= *anzwaende)
				/* falls rechts von allen Waenden */
				rechts = wand.ecke[2].x;
			else
				rechts = min(wand.ecke[2].x, (*waende)[i].ecke[0].x);

			/* y-Koordinaten und Farbe kopieren */
			memcpy(&ausschnitt, &wand, sizeof(struct flaeche));

			/* x-Koordinaten */
			ausschnitt.ecke[0].x = ausschnitt.ecke[1].x = links;
			ausschnitt.ecke[2].x = ausschnitt.ecke[3].x = rechts;

			/* falls 'wand' nicht nur ein Strich, y-Koordinaten anpassen */
			if (wand.ecke[0].x != wand.ecke[2].x)
			{
				/* linke y-Koordinaten anpassen */
				if (links > wand.ecke[0].x)
				{
					ausschnitt.ecke[0].y =
						(links - wand.ecke[0].x) *
						(wand.ecke[3].y - wand.ecke[0].y) /
						(wand.ecke[2].x - wand.ecke[0].x) + wand.ecke[0].y;
					ausschnitt.ecke[1].y =
						(links - wand.ecke[0].x) *
						(wand.ecke[2].y - wand.ecke[1].y) /
						(wand.ecke[2].x - wand.ecke[0].x) + wand.ecke[1].y;
				}

				/* rechte y-Koordinaten anpassen */
				if (rechts < wand.ecke[2].x)
				{
					ausschnitt.ecke[2].y =
						(rechts - wand.ecke[0].x) *
						(wand.ecke[2].y - wand.ecke[1].y) /
						(wand.ecke[2].x - wand.ecke[0].x) + wand.ecke[1].y;
					ausschnitt.ecke[3].y =
						(rechts - wand.ecke[0].x) *
						(wand.ecke[3].y - wand.ecke[0].y) /
						(wand.ecke[2].x - wand.ecke[0].x) + wand.ecke[0].y;
				}
			}
		}

		liste_verlaengern(anzwaende, (void **)waende,
			sizeof(struct flaeche));

		{
			int j;

			/* weiter rechts liegende Waende in *waende verschieben */
			for (j = *anzwaende - 1; j > i; j--)
				memcpy(&(*waende)[j], &(*waende)[j - 1],
					sizeof(struct flaeche));
		}

		/* Ausschnitt in *waende kopieren */
		memcpy(&(*waende)[i], &ausschnitt, sizeof(struct flaeche));
	}
}


/*
** objekt_vergleich
**  vergleicht zwei Kugelobjekte nach feldx/y,
**  welches bei Blick in Nordrichtung zuerst einsortiert werden muss
**  (wird fuer qsort verwendet)
**
** Parameter:
**  o1: Zeiger auf ein Objekt
**  o2: Zeiger auf anderes Objekt
**
** Rueckgabewert:
**  < 0, falls Objekt 1 vor Objekt 2
**  = 0, falls egal
**  > 0, falls Objekt 1 nach Objekt 2
**
** Seiteneffekte:
**  objekte1->soll und objekt2->soll werden gesetzt
*/
static int objekt_vergleich(const void *o1, const void *o2)
{
	const struct objekt *objekt1 = o1;
	const struct objekt *objekt2 = o2;
	int vergleich; /* Ergebnis des Vergleichs ohne Beruecksichtigung
	                  des Abstands der Objekte vom Spieler */

	/* an welcher Stelle beim Zeichnen der Waende sollen
	   die Objekte einsortiert werden? Objekte danach vergleichen */
	vergleich = objekt1->durchgang[AKTUELL] != objekt2->durchgang[AKTUELL] ?
		objekt1->durchgang[AKTUELL] - objekt2->durchgang[AKTUELL] :
		objekt1->richtung[AKTUELL] != objekt2->richtung[AKTUELL] ?
		objekt1->richtung[AKTUELL] - objekt2->richtung[AKTUELL] :
		objekt1->richtung[AKTUELL] != VORHER &&
		objekt1->index[AKTUELL] != objekt2->index[AKTUELL] ?
		objekt1->index[AKTUELL] - objekt2->index[AKTUELL] : 0;

	/* sollen beide Objekte an derselben Stelle einsortiert werden? */
	if (vergleich == 0)

		/* dann Abstaende der Objekte vom Spieler vergleichen */
		return objekt1->abstandq == objekt2->abstandq ? 0 :
			objekt1->abstandq < objekt2->abstandq ? -1 : 1;

	/* haben beide Objekte denselben Abstand vom Spieler oder sind weit
	   genug (eine Gangbreite) voneinander entfernt, dann fertig */
	if (objekt1->abstandq == objekt2->abstandq ||
		abs(XPOS(objekt1->pos) - XPOS(objekt2->pos)) >= RASPT ||
		abs(YPOS(objekt1->pos) - YPOS(objekt2->pos)) >= RASPT)
		return vergleich;

	/* stimmt die Reihenfolge, die beim Vergleich rausgekommen ist, nicht
	   mit den Abstaenden der Objekte vom Spieler ueberein? */
	if ((vergleich < 0) ^ (objekt1->abstandq < objekt2->abstandq))
	{
		/* dann markieren, dass die Reihenfolge geaendert werden soll,
		   vorsicht: const wird weggecastet */
		((struct objekt *)objekt1)->soll =
			vergleich < 0 ? SPAETER : FRUEHER;
		((struct objekt *)objekt2)->soll =
			vergleich > 0 ? SPAETER : FRUEHER;
	}

	/* fertig */
	return vergleich;
}


/*
** einnorden
**  dreht eine Position um 90 Grad * himricht nach rechts
**
** Parameter:
**  neu: Zeiger auf die neue Position
**  alt: Zeiger auf die alte Position
**  himricht: Winkel um den gedreht werden soll
**            (grobe Blickrichtung von 'ich')
**  feldbreite: Breite des Spielfeldes
**  feldlaenge: Laenge des Spielfeldes
**
** Seiteneffekte:
**  *neu wird auf die neue Position gesetzt
*/
static void einnorden(struct position *neu, struct position *alt,
	u_char himricht, int feldbreite, int feldlaenge)
{
	int xalt, yalt; /* alte Koordinaten im feinen Raster*/
	int xneu, yneu; /* neue Koordinaten im feinen Raster*/

	xalt = XPOS(*alt);
	yalt = YPOS(*alt);

	switch (himricht)
	{
		case 0: /* NORD */
			xneu = xalt;
			yneu = yalt;
			break;

		case 1: /* WEST */
			xneu = RASPT * feldlaenge - yalt;
			yneu = xalt;
			break;

		case 2: /* SUED */
			xneu = RASPT * feldbreite - xalt;
			yneu = RASPT * feldlaenge - yalt;
			break;

		case 3: /* OST */
			xneu = yalt;
			yneu = RASPT * feldbreite - xalt;
			break;
	}

	SETX(*neu, xneu);
	SETY(*neu, yneu);
}


/*
** tueren_offen_waagerecht
**  markiert ein West-/Ost-Tuerenpaar (oder Wandpaar) als offen
**
** Parameter:
**  anzoffen: Zeiger auf die Anzahl der offenen Tueren
**  offen: Zeiger auf die Liste der offenen Tueren
**  feld: Spielfeld in aktuelle Nordrichtung gedreht
**  feldbreite: Breite des Spielfeldes in Bloecken
**  feldlaenge: Laenge des Spielfeldes in Bloecken
**  x: x-Koordinate der zu markierenden West-Tuer
**  y: y-Koordinate der zu markierenden Tueren
**
** Seiteneffekte:
**  *anzoffen wird erhoeht, *offen um ein Element verlaengert,
**  in feld werden die tueroffen-Flags gesetzt
*/
static void tueren_offen_waagerecht(int *anzoffen, u_char (**offen)[3],
	block **feld, int feldbreite, int feldlaenge, int x, int y)
{
	/* falls Koordinaten ausserhalb des Spielfeldes oder alle Tueren/Waende
	   bereits TRANSPARENT, nichts tun */
	if (x < 0 || x > feldbreite || y < 0 || y >= feldlaenge ||
		(x == 0 || feld[x - 1][y][OST].farbe == TRANSPARENT) &&
		(x == feldbreite || feld[x][y][WEST].farbe == TRANSPARENT))
		return;

	if (x > 0)
	{
		/* Osttuer (linke Wand) in Liste eintragen */
		liste_verlaengern(anzoffen, (void **)offen, sizeof(u_char [3]));
		(*offen)[*anzoffen - 1][0] = x - 1;
		(*offen)[*anzoffen - 1][1] = y;
		(*offen)[*anzoffen - 1][2] = OST;

		/* Tuer im Spielfeld markieren */
		feld[x - 1][y][OST].tueroffen = 1;
	}

	if (x < feldbreite)
	{
		/* Westtuer (rechte Wand) in Liste eintragen */
		liste_verlaengern(anzoffen, (void **)offen, sizeof(u_char [3]));
		(*offen)[*anzoffen - 1][0] = x;
		(*offen)[*anzoffen - 1][1] = y;
		(*offen)[*anzoffen - 1][2] = WEST;

		/* Tuer im Spielfeld markieren */
		feld[x][y][WEST].tueroffen = 1;
	}
}


/*
** tueren_offen_senkrecht
**  markiert ein Nord-/Sued-Tuerenpaar (oder Wandpaar) als offen
**
** Parameter:
**  anzoffen: Zeiger auf die Anzahl der offenen Tueren
**  offen: Zeiger auf die Liste der offenen Tueren
**  feld: Spielfeld in aktuelle Nordrichtung gedreht
**  feldbreite: Breite des Spielfeldes in Bloecken
**  feldlaenge: Laenge des Spielfeldes in Bloecken
**  x: x-Koordinate der zu markierenden Tueren
**  y: y-Koordinate der zu markierenden Nord-Tuer
**
** Seiteneffekte:
**  *anzoffen wird erhoeht, *offen um ein Element verlaengert,
**  in feld werden die tueroffen-Flags gesetzt
*/
static void tueren_offen_senkrecht(int *anzoffen, u_char (**offen)[3],
	block **feld, int feldbreite, int feldlaenge, int x, int y)
{
	/* falls Koordinaten ausserhalb des Spielfeldes oder alle Tueren/Waende
	   bereits TRANSPARENT, nichts tun */
	if (x < 0 || x >= feldbreite || y < 0 || y > feldlaenge ||
		(y == 0 || feld[x][y - 1][SUED].farbe == TRANSPARENT) &&
		(y == feldlaenge || feld[x][y][NORD].farbe == TRANSPARENT))
		return;

	if (y > 0)
	{
		/* Suedtuer (obere Wand) in Liste eintragen */
		liste_verlaengern(anzoffen, (void **)offen, sizeof(u_char [3]));
		(*offen)[*anzoffen - 1][0] = x;
		(*offen)[*anzoffen - 1][1] = y - 1;
		(*offen)[*anzoffen - 1][2] = SUED;

		/* Tuer im Spielfeld markieren */
		feld[x][y - 1][SUED].tueroffen = 1;
	}

	if (y < feldlaenge)
	{
		/* Nordtuer (untere Wand) in Liste eintragen */
		liste_verlaengern(anzoffen, (void **)offen, sizeof(u_char [3]));
		(*offen)[*anzoffen - 1][0] = x;
		(*offen)[*anzoffen - 1][1] = y;
		(*offen)[*anzoffen - 1][2] = NORD;

		/* Tuer im Spielfeld markieren */
		feld[x][y][NORD].tueroffen = 1;
	}
}


/*
** tueren_offen_markieren
**  markiert alle Tueren/Waende, die ein Spieler durchdringt als offen
**
** Parameter:
**  anzoffen: Zeiger auf die Anzahl der offenen Tueren
**  offen: Zeiger auf die Liste der offenen Tueren
**  feld: Spielfeld in aktuelle Nordrichtung gedreht
**  feldbreite: Breite des Spielfeldes in Bloecken
**  feldlaenge: Laenge des Spielfeldes in Bloecken
**  pos: Position des Spielers
**  radius: Radius des Spielers
**
** Seiteneffekte:
**  *anzoffen, *offen werden veraendert
**  in feld werden die tueroffen-Flags gesetzt
*/
static void tueren_offen_markieren(int *anzoffen, u_char (**offen)[3],
	block **feld, int feldbreite, int feldlaenge, struct position *pos,
	int radius)
{
	/* ragt Spieler links raus? */
	if ((int)pos->xfein < radius)
		tueren_offen_waagerecht(anzoffen, offen, feld,
			feldbreite, feldlaenge, pos->xgrob, pos->ygrob);

	/* ragt Spieler oben raus? */
	if ((int)pos->yfein < radius)
		tueren_offen_senkrecht(anzoffen, offen, feld,
			feldbreite, feldlaenge, pos->xgrob, pos->ygrob);

	/* ragt Spieler rechts raus? */
	if ((int)pos->xfein > RASPT - radius)
		tueren_offen_waagerecht(anzoffen, offen, feld,
			feldbreite, feldlaenge, pos->xgrob + 1, pos->ygrob);

	/* ragt Spieler unten raus? */
	if ((int)pos->yfein > RASPT - radius)
		tueren_offen_senkrecht(anzoffen, offen, feld,
			feldbreite, feldlaenge, pos->xgrob, pos->ygrob + 1);

	/* ragt Spieler links oben raus? */
	if (sqr((int)pos->xfein) + sqr((int)pos->yfein) < sqr(radius))
	{
		tueren_offen_waagerecht(anzoffen, offen, feld,
			feldbreite, feldlaenge, pos->xgrob, pos->ygrob - 1);
		tueren_offen_senkrecht(anzoffen, offen, feld,
			feldbreite, feldlaenge, pos->xgrob - 1, pos->ygrob);
	}

	/* ragt Spieler links unten raus? */
	if (sqr((int)pos->xfein) + sqr(RASPT - (int)pos->yfein) < sqr(radius))
	{
		tueren_offen_waagerecht(anzoffen, offen, feld,
			feldbreite, feldlaenge, pos->xgrob, pos->ygrob + 1);
		tueren_offen_senkrecht(anzoffen, offen, feld,
			feldbreite, feldlaenge, pos->xgrob - 1, pos->ygrob + 1);
	}

	/* ragt Spieler rechts unten raus? */
	if (sqr(RASPT - (int)pos->xfein) + sqr(RASPT - (int)pos->yfein) <
		sqr(radius))
	{
		tueren_offen_waagerecht(anzoffen, offen, feld,
			feldbreite, feldlaenge, pos->xgrob + 1, pos->ygrob + 1);
		tueren_offen_senkrecht(anzoffen, offen, feld,
			feldbreite, feldlaenge, pos->xgrob + 1, pos->ygrob + 1);
	}

	/* ragt Spieler rechts oben raus? */
	if (sqr(RASPT - (int)pos->xfein) + sqr((int)pos->yfein) < sqr(radius))
	{
		tueren_offen_waagerecht(anzoffen, offen, feld,
			feldbreite, feldlaenge, pos->xgrob + 1, pos->ygrob - 1);
		tueren_offen_senkrecht(anzoffen, offen, feld,
			feldbreite, feldlaenge, pos->xgrob + 1, pos->ygrob);
	}
}


/*
** rechne_nordwand
**  fuehrt fuer eine Nordwand eine Koordinatentransformation relativ zu
**  Position und Blickrichtung des Spielers durch und stellt uebergibt
**  sie, sofern sie nicht offen ist, an rechnewand zur weiteren Bearbeitung
**
** Parameter:
**  anzwaende: Zeiger auf die Anzahl der sichtbaren Waende
**  waende: Zeiger auf die Liste der sichtbaren Waende
**  feld: Spielfeld in aktuelle Nordrichtung gedreht
**  x: gedrehte x-Koordinate des Blocks, zu der die Wand gehoert
**  y: gedrehte y-Koordinate des Blocks, zu der die Wand gehoert
**  ichpos: gedrehte Position des Spielers
**  ichblick: gedrehte Blickrichtung des Spielers
**
** Seiteneffekte:
**  *anzwaende wird erhoeht, *waende verlaengert
*/
static void rechne_nordwand(int *anzwaende, struct flaeche **waende,
	block **feld, int x, int y, struct position *ichpos, int ichblick)
{
	if (!feld[x][y][NORD].tueroffen)
		rechnewand(anzwaende, waende,
			drehx((x * RASPT - XPOS(*ichpos)),
				(YPOS(*ichpos) - y * RASPT), ichblick),
			drehy((x * RASPT - XPOS(*ichpos)),
				(YPOS(*ichpos) - y * RASPT), ichblick),
			drehx(((x + 1) * RASPT - XPOS(*ichpos)),
				(YPOS(*ichpos) - y * RASPT), ichblick),
			drehy(((x + 1) * RASPT - XPOS(*ichpos)),
				(YPOS(*ichpos) - y * RASPT), ichblick),
			feld[x][y][NORD].farbe,
			feld[x][y][NORD].tuer);
}


/*
** rechne_westwand
**  fuehrt fuer eine Westwand eine Koordinatentransformation relativ zu
**  Position und Blickrichtung des Spielers durch und stellt uebergibt
**  sie, sofern sie nicht offen ist, an rechnewand zur weiteren Bearbeitung
**
** Parameter:
**  anzwaende: Zeiger auf die Anzahl der sichtbaren Waende
**  waende: Zeiger auf die Liste der sichtbaren Waende
**  feld: Spielfeld in aktuelle Nordrichtung gedreht
**  x: gedrehte x-Koordinate des Blocks, zu der die Wand gehoert
**  y: gedrehte y-Koordinate des Blocks, zu der die Wand gehoert
**  ichpos: gedrehte Position des Spielers
**  ichblick: gedrehte Blickrichtung des Spielers
**
** Seiteneffekte:
**  *anzwaende wird erhoeht, *waende verlaengert
*/
static void rechne_westwand(int *anzwaende, struct flaeche **waende,
	block **feld, int x, int y, struct position *ichpos, int ichblick)
{
	if (!feld[x][y][WEST].tueroffen)
		rechnewand(anzwaende, waende,
			drehx((x * RASPT - XPOS(*ichpos)),
				(YPOS(*ichpos) - y * RASPT), ichblick),
			drehy((x * RASPT - XPOS(*ichpos)),
				(YPOS(*ichpos) - y * RASPT), ichblick),
			drehx((x * RASPT - XPOS(*ichpos)),
				(YPOS(*ichpos) - (y + 1) * RASPT), ichblick),
			drehy((x * RASPT - XPOS(*ichpos)),
				(YPOS(*ichpos) - (y + 1) * RASPT), ichblick),
			feld[x][y][WEST].farbe,
			feld[x][y][WEST].tuer);
}


/*
** rechne_ostwand
**  fuehrt fuer eine Ostwand eine Koordinatentransformation relativ zu
**  Position und Blickrichtung des Spielers durch und stellt uebergibt
**  sie, sofern sie nicht offen ist, an rechnewand zur weiteren Bearbeitung
**
** Parameter:
**  anzwaende: Zeiger auf die Anzahl der sichtbaren Waende
**  waende: Zeiger auf die Liste der sichtbaren Waende
**  feld: Spielfeld in aktuelle Nordrichtung gedreht
**  x: gedrehte x-Koordinate des Blocks, zu der die Wand gehoert
**  y: gedrehte y-Koordinate des Blocks, zu der die Wand gehoert
**  ichpos: gedrehte Position des Spielers
**  ichblick: gedrehte Blickrichtung des Spielers
**
** Seiteneffekte:
**  *anzwaende wird erhoeht, *waende verlaengert
*/
static void rechne_ostwand(int *anzwaende, struct flaeche **waende,
	block **feld, int x, int y, struct position *ichpos, int ichblick)
{
	if (!feld[x][y][OST].tueroffen)
		rechnewand(anzwaende, waende,
			drehx(((x + 1) * RASPT - XPOS(*ichpos)),
				(YPOS(*ichpos) - y * RASPT), ichblick),
			drehy(((x + 1) * RASPT - XPOS(*ichpos)),
				(YPOS(*ichpos) - y * RASPT), ichblick),
			drehx(((x + 1) * RASPT - XPOS(*ichpos)),
				(YPOS(*ichpos) - (y + 1) * RASPT), ichblick),
			drehy(((x + 1) * RASPT - XPOS(*ichpos)),
				(YPOS(*ichpos) - (y + 1) * RASPT), ichblick),
			feld[x][y][OST].farbe,
			feld[x][y][OST].tuer);
}


/*
** objekte_sortieren_vorberechnung
**  berechnet die Daten, die zum Sortieren in objekte_sortieren
**  benoetigt werden
**
** Parameter:
**  anzobjekte: Anzahl der Objekte
**  objekte: Liste von Positionen, etc. der Objekte
**  ichpos: Zeiger auf die Position des Spielers
**  feld: das Spielfeld
**  feldbreite: Breite des Spielfeldes
**  feldlaenge: Laenge des Spielfeldes
**
** Seiteneffekte:
**  *objekte wird veraendert
*/
static void objekte_sortieren_vorberechnung(int anzobjekte,
	struct objekt *objekte, struct position *ichpos, block **feld,
	int feldbreite, int feldlaenge)
{
	int i; /* Index fuer Objekte */

	/* ueberhaupt Objekte zu sortieren? */
	if (anzobjekte < 1)
		return;

	/* fuer alle Objekte berechnen, an welcher Position sie gezeichnet
	   werden sollen; fuer Objekte, die ganz in einem Block liegen, auch
	   Ausweichpositionen berechnen */
	for (i = 0; i < anzobjekte; i++)
	{
		int links;    /* Flag: ist Objekt links vom Spieler? */
		int radius;   /* Radius des Objekts */
		int schnittx; /* Flag: schneidet Objekt eine (potentielle) Wand
		                 in x-Richtung, d.h. eine Ost- oder Westwand? */
		int schnitty; /* Flag: schneidet Objekt eine (potentielle) Wand
		                 in y-Richtung, d.h. eine Nord- oder Suedwand? */
		int x;        /* x-Koordinate des Blocks, in den der rechte (bei
		                 links liegenden Objekten) bzw. linke (bei rechts
		                 liegenden Objekten) Rand des Objekts faellt */
		int y;        /* y-Koordinate des Blocks, in den der vordere Rand
		                 des Objekts faellt */
		int dx, dy;   /* x, y relativ zum Spieler */
		int ecke_x;   /* x-Koordinate der zum Objekt benachbarten
		                 Blockecke im feinen Raster */
		int ecke_y;   /* y-Koordinate der zum Objekt benachbarten
		                 Blockecke im feinen Raster */

		/* Quadrat des Abstands zwischen Objekt und 'ich' berechnen */
		objekte[i].abstandq = sqr(XPOS(objekte[i].pos) - XPOS(*ichpos)) +
			sqr(YPOS(objekte[i].pos) - YPOS(*ichpos));

		/* Position, an der gezeichnet werden soll, und
		   Ausweichpositionen initialisieren */
		objekte[i].richtung[AKTUELL] = objekte[i].richtung[FRUEHER] =
			objekte[i].richtung[SPAETER] = VORHER;
		objekte[i].index[AKTUELL] = objekte[i].index[FRUEHER] =
			objekte[i].index[SPAETER] = 0;

		/* noch keine Ausweichposition bekannt */
		objekte[i].kann_frueher = objekte[i].kann_spaeter = 0;

		/* liegt das Objekt hinter dem Spieler? */
		if ((int)objekte[i].pos.ygrob > (int)ichpos->ygrob)
		{
			/* dann nicht weiter beachten */
			objekte[i].durchgang[AKTUELL] = -1;
			continue;
		}

		/* Radius des Objekts merken */
		radius = objekte[i].radius;

		/* feststellen, welche (potentiellen) Waende das Objekt schneidet */
		schnittx = (int)objekte[i].pos.xfein < radius ||
			RASPT - (int)objekte[i].pos.xfein < radius;
		schnitty = (int)objekte[i].pos.yfein < radius ||
			RASPT - (int)objekte[i].pos.yfein < radius;

		/* feststellen, ob das Objekt links vom Spieler ist */
		links = XPOS(objekte[i].pos) < RASPT * (int)ichpos->xgrob + RASPT / 2;

		/* bis in welche Bloecke reicht das Objekt? */
		x = (XPOS(objekte[i].pos) + (links ? radius - 1 : -radius)) / RASPT;
		y = (YPOS(objekte[i].pos) + radius - 1) / RASPT;

		/* die Koordinaten noch relativ zum Spieler berechnen */
		dx = x - (int)ichpos->xgrob;
		dy = (int)ichpos->ygrob - y;

		/* in welchem Durchgang muss das Objekt normalerweise
		   gezeichnet werden? */
		objekte[i].durchgang[AKTUELL] = abs(dx) + dy;

		/* befindet sich das Objekt ganz in einem Block? */
		if (!schnittx && !schnitty)
		{
			/* moegliche Korrekturen berechnen */

			/* 1. Fall: Objekt befindet sich links vom Spieler;
			   Test, ob das Objekt auch frueher gezeichnet werden kann */
			if (links && objekte[i].durchgang[AKTUELL] > 0 &&
				x < feldbreite - 1 && y < feldlaenge - 1)
			{
				/* ist vor aber nicht rechts neben dem Objekt eine Wand? */
				if (dx < 0 &&
					(feld[x + 1][y][WEST].tueroffen ||
					feld[x + 1][y][WEST].farbe == TRANSPARENT) &&
					!(feld[x][y + 1][NORD].tueroffen ||
					feld[x][y + 1][NORD].farbe == TRANSPARENT))
				{
					objekte[i].kann_frueher = 1;
					objekte[i].durchgang[FRUEHER] =
						objekte[i].durchgang[AKTUELL] - 1;
					objekte[i].richtung[FRUEHER] = LINKS_NORD;
					objekte[i].index[FRUEHER] = dy;
				}

				/* ist rechts neben aber nicht vor dem Objekt eine Wand? */
				if (dy > 0 &&
					!(feld[x + 1][y][WEST].tueroffen ||
					feld[x + 1][y][WEST].farbe == TRANSPARENT) &&
					(feld[x][y + 1][NORD].tueroffen ||
					feld[x][y + 1][NORD].farbe == TRANSPARENT))
				{
					objekte[i].kann_frueher = 1;
					objekte[i].durchgang[FRUEHER] =
						objekte[i].durchgang[AKTUELL] - 1;
					objekte[i].richtung[FRUEHER] = LINKS_WEST;
					objekte[i].index[FRUEHER] = abs(dx);
				}

				/* ist weder vor noch rechts neben dem Objekt eine Wand? */
				if (dx < 0 && dy > 0 &&
					(feld[x + 1][y][WEST].tueroffen ||
					feld[x + 1][y][WEST].farbe == TRANSPARENT) &&
					(feld[x][y + 1][NORD].tueroffen ||
					feld[x][y + 1][NORD].farbe == TRANSPARENT))
				{
					objekte[i].kann_frueher = 1;
					objekte[i].durchgang[FRUEHER] =
						objekte[i].durchgang[AKTUELL] - 1;
					objekte[i].richtung[FRUEHER] = VORHER;
					objekte[i].index[FRUEHER] = 0;
				}
			}

			/* 2. Fall: Objekt befindet sich rechts vom Spieler;
			   Test, ob das Objekt auch frueher gezeichnet werden kann */
			if (!links && objekte[i].durchgang[AKTUELL] > 0 &&
				x > 0 && y < feldlaenge - 1)
			{
				/* ist vor aber nicht links neben dem Objekt eine Wand? */
				if (dx > 0 &&
					(feld[x - 1][y][OST].tueroffen ||
					feld[x - 1][y][OST].farbe == TRANSPARENT) &&
					!(feld[x][y + 1][NORD].tueroffen ||
					feld[x][y + 1][NORD].farbe == TRANSPARENT))
				{
					objekte[i].kann_frueher = 1;
					objekte[i].durchgang[FRUEHER] =
						objekte[i].durchgang[AKTUELL] - 1;
					objekte[i].richtung[FRUEHER] = RECHTS_NORD;
					objekte[i].index[FRUEHER] = dy;
				}

				/* ist links neben aber nicht vor dem Objekt eine Wand? */
				if (dy > 0 &&
					!(feld[x - 1][y][OST].tueroffen ||
					feld[x - 1][y][OST].farbe == TRANSPARENT) &&
					(feld[x][y + 1][NORD].tueroffen ||
					feld[x][y + 1][NORD].farbe == TRANSPARENT))
				{
					objekte[i].kann_frueher = 1;
					objekte[i].durchgang[FRUEHER] =
						objekte[i].durchgang[AKTUELL] - 1;
					objekte[i].richtung[FRUEHER] = RECHTS_OST;
					objekte[i].index[FRUEHER] = abs(dx);
				}

				/* ist weder vor noch links neben dem Objekt eine Wand? */
				if (dx > 0 && dy > 0 &&
					(feld[x - 1][y][OST].tueroffen ||
					feld[x - 1][y][OST].farbe == TRANSPARENT) &&
					(feld[x][y + 1][NORD].tueroffen ||
					feld[x][y + 1][NORD].farbe == TRANSPARENT))
				{
					objekte[i].kann_frueher = 1;
					objekte[i].durchgang[FRUEHER] =
						objekte[i].durchgang[AKTUELL] - 1;
					objekte[i].richtung[FRUEHER] = VORHER;
					objekte[i].index[FRUEHER] = 0;
				}
			}

			/* 3. Fall: Objekt befindet sich links vom Spieler;
			   Test, ob das Objekt auch spaeter gezeichnet werden kann */
			if (links)
			{
				/* ist hinter aber nicht links neben dem Objekt eine Wand? */
				if ((feld[x][y][WEST].tueroffen ||
					feld[x][y][WEST].farbe == TRANSPARENT) &&
					!(feld[x][y][NORD].tueroffen ||
					feld[x][y][NORD].farbe == TRANSPARENT))
				{
					objekte[i].kann_spaeter = 1;
					objekte[i].durchgang[SPAETER] =
						objekte[i].durchgang[AKTUELL];
					objekte[i].richtung[SPAETER] = LINKS_NORD;
					objekte[i].index[SPAETER] = dy;
				}

				/* ist links neben aber nicht hinter dem Objekt eine Wand? */
				if (!(feld[x][y][WEST].tueroffen ||
					feld[x][y][WEST].farbe == TRANSPARENT) &&
					(feld[x][y][NORD].tueroffen ||
					feld[x][y][NORD].farbe == TRANSPARENT))
				{
					objekte[i].kann_spaeter = 1;
					objekte[i].durchgang[SPAETER] =
						objekte[i].durchgang[AKTUELL];
					objekte[i].richtung[SPAETER] = LINKS_WEST;
					objekte[i].index[SPAETER] = abs(dx);
				}

				/* ist weder hinter noch links neben dem Objekt eine Wand? */
				if ((feld[x][y][WEST].tueroffen ||
					feld[x][y][WEST].farbe == TRANSPARENT) &&
					(feld[x][y][NORD].tueroffen ||
					feld[x][y][NORD].farbe == TRANSPARENT))
				{
					objekte[i].kann_spaeter = 1;
					objekte[i].durchgang[SPAETER] =
						objekte[i].durchgang[AKTUELL] + 1;
					objekte[i].richtung[SPAETER] = VORHER;
					objekte[i].index[SPAETER] = 0;
				}
			}

			/* 4. Fall: Objekt befindet sich rechts vom Spieler;
			   Test, ob das Objekt auch spaeter gezeichnet werden kann */
			if (!links)
			{
				/* ist hinter aber nicht rechts neben dem Objekt eine Wand? */
				if ((feld[x][y][OST].tueroffen ||
					feld[x][y][OST].farbe == TRANSPARENT) &&
					!(feld[x][y][NORD].tueroffen ||
					feld[x][y][NORD].farbe == TRANSPARENT))
				{
					objekte[i].kann_spaeter = 1;
					objekte[i].durchgang[SPAETER] =
						objekte[i].durchgang[AKTUELL];
					objekte[i].richtung[SPAETER] = RECHTS_NORD;
					objekte[i].index[SPAETER] = dy;
				}

				/* ist rechts neben aber nicht hinter dem Objekt eine Wand? */
				if (!(feld[x][y][OST].tueroffen ||
					feld[x][y][OST].farbe == TRANSPARENT) &&
					(feld[x][y][NORD].tueroffen ||
					feld[x][y][NORD].farbe == TRANSPARENT))
				{
					objekte[i].kann_spaeter = 1;
					objekte[i].durchgang[SPAETER] =
						objekte[i].durchgang[AKTUELL];
					objekte[i].richtung[SPAETER] = RECHTS_OST;
					objekte[i].index[SPAETER] = abs(dx);
				}

				/* ist weder hinter noch rechts neben dem Objekt eine Wand? */
				if ((feld[x][y][OST].tueroffen ||
					feld[x][y][OST].farbe == TRANSPARENT) &&
					(feld[x][y][NORD].tueroffen ||
					feld[x][y][NORD].farbe == TRANSPARENT))
				{
					objekte[i].kann_spaeter = 1;
					objekte[i].durchgang[SPAETER] =
						objekte[i].durchgang[AKTUELL] + 1;
					objekte[i].richtung[SPAETER] = VORHER;
					objekte[i].index[SPAETER] = 0;
				}
			}

			/* keine weiteren Korrekturmoeglichkeiten */
			continue;
		}

		/* befindet sich das Objekt in zwei aneinandergrenzenden Bloecken? */
		if (!schnittx || !schnitty)
		{
			/* berechnen, zwischen welchen Waenden das Objekt
			   gezeichnet werden muss */

			/* liegen die Bloecke nebeneinander? */
			if (schnittx)
			{
				objekte[i].richtung[AKTUELL] = links ? LINKS_NORD : RECHTS_NORD;
				objekte[i].index[AKTUELL] = dy;
			}
			/* sonst hintereinander */
			else
			{
				objekte[i].richtung[AKTUELL] = links ? LINKS_WEST : RECHTS_OST;
				objekte[i].index[AKTUELL] = abs(dx);
			}

			continue;
		}

		/* befinden sich Teile des Objekts in vier Bloecken,
		   d.h. die Blockecke im Objekt? */
		if (sqr((int)objekte[i].pos.xfein < radius ?
			(int)objekte[i].pos.xfein : RASPT - (int)objekte[i].pos.xfein) +
			sqr((int)objekte[i].pos.yfein < radius ?
			(int)objekte[i].pos.yfein : RASPT - (int)objekte[i].pos.yfein) <
			sqr(radius))
		{
			/* dann einen Durchgang spaeter zeichnen */
			objekte[i].durchgang[AKTUELL]++;
			continue;
		}

		/* sonst befinden sich Teile des Objekts in genau drei Bloecken */

		/* befindet sich das Objekt links vom Spieler und liegt ganz vor
		   der Ecke, an der sich die drei Bloecke treffen? */
		if (links && (int)objekte[i].pos.xfein < radius &&
			(int)objekte[i].pos.yfein < radius)
		{
			/* dann einen Durchgang spaeter zeichnen */
			objekte[i].durchgang[AKTUELL]++;
			continue;
		}

		/* befindet sich das Objekt rechts vom Spieler und liegt ganz vor
		   der Ecke, an der sich die drei Bloecke treffen? */
		if (!links && (int)objekte[i].pos.xfein >= radius &&
			(int)objekte[i].pos.yfein < radius)
		{
			/* dann einen Durchgang spaeter zeichnen */
			objekte[i].durchgang[AKTUELL]++;
			continue;
		}

		/* befindet sich das Objekt links vom Spieler und liegt ganz hinter
		   der Ecke, an der sich die drei Bloecke treffen? */
		if (links && (int)objekte[i].pos.xfein >= radius &&
			(int)objekte[i].pos.yfein >= radius)
		{
			/* dann einen Durchgang spaeter zeichnen */
			objekte[i].durchgang[AKTUELL]++;
			continue;
		}

		/* befindet sich das Objekt rechts vom Spieler und liegt ganz hinter
		   der Ecke, an der sich die drei Bloecke treffen? */
		if (!links && (int)objekte[i].pos.xfein < radius &&
			(int)objekte[i].pos.yfein >= radius)
		{
			/* dann einen Durchgang spaeter zeichnen */
			objekte[i].durchgang[AKTUELL]++;
			continue;
		}

		/* Koordinaten der Ecke, an der sich die drei Bloecke
		   treffen, berechnen */
		ecke_x = RASPT * (objekte[i].pos.xgrob +
			((int)objekte[i].pos.xfein < RASPT / 2 ? 0 : 1));
		ecke_y = RASPT * (objekte[i].pos.ygrob +
			((int)objekte[i].pos.yfein < RASPT / 2 ? 0 : 1));

		/* mit Skalarprodukt bestimmen: liegt das Objekt vor der Ecke? */
		if ((XPOS(objekte[i].pos) - ecke_x) * (XPOS(*ichpos) - ecke_x) +
			(YPOS(objekte[i].pos) - ecke_y) * (YPOS(*ichpos) - ecke_y) > 0)
		{
			/* ist das Objekt in y-Richtung naeher als die Ecke? */
			if ((int)objekte[i].pos.yfein < radius)
			{
				objekte[i].richtung[AKTUELL] = links ? LINKS_NORD : RECHTS_NORD;
				objekte[i].index[AKTUELL] = dy;
			}
			else
			{
				objekte[i].richtung[AKTUELL] = links ? LINKS_WEST : RECHTS_OST;
				objekte[i].index[AKTUELL] = abs(dx);
			}

			continue;
		}

		/* ist das Objekt in y-Richtung naeher als die Ecke? */
		if ((int)objekte[i].pos.yfein < radius)
		{
			objekte[i].durchgang[AKTUELL]++;
			objekte[i].richtung[AKTUELL] = links ? LINKS_WEST : RECHTS_OST;
			objekte[i].index[AKTUELL] = abs(dx) + 1;
		}
		else
		{
			objekte[i].durchgang[AKTUELL]++;
			objekte[i].richtung[AKTUELL] = links ? LINKS_NORD : RECHTS_NORD;
			objekte[i].index[AKTUELL] = dy + 1;
		}
	}

	/* fuer Objekte, die nicht ganz in einem Block liegen,
	   Ausweichpositionen berechnen */
	for (i = 0; i < anzobjekte; i++)
	{
		int x, y; /* Koordinaten des Blocks, dessen Wand fuer eine
		             Ausweichposition beruecksichtigt werden muss */

		/* wird das Objekt normalerweise zwischen rechts liegenden
		   Ostwaenden gezeichnet? */
		if (objekte[i].richtung[AKTUELL] == RECHTS_OST)
		{
			/* befindet sich vor dem Objekt keine Ostwand? */
			x = (int)ichpos->xgrob + (objekte[i].index[AKTUELL] - 1);
			y = (int)ichpos->ygrob - objekte[i].durchgang[AKTUELL] +
				(objekte[i].index[AKTUELL] - 1);
			if (objekte[i].index[AKTUELL] > 0 && !(x >= feldbreite || y < 0) &&
				(feld[x][y][OST].tueroffen ||
				feld[x][y][OST].farbe == TRANSPARENT))

				/* dann kann das Objekt auch frueher gezeichnet werden */
				objekte[i].kann_frueher = 1,
				objekte[i].durchgang[FRUEHER] = objekte[i].durchgang[AKTUELL];

			/* befindet sich hinter dem Objekt keine Ostwand? */
			x = (int)ichpos->xgrob + objekte[i].index[AKTUELL];
			y = (int)ichpos->ygrob - objekte[i].durchgang[AKTUELL] +
				objekte[i].index[AKTUELL];
			if (!(x >= feldbreite || y < 0) &&
				(feld[x][y][OST].tueroffen ||
				feld[x][y][OST].farbe == TRANSPARENT))

				/* dann kann das Objekt auch spaeter gezeichnet werden */
				objekte[i].kann_spaeter = 1,
				objekte[i].durchgang[SPAETER] =
					objekte[i].durchgang[AKTUELL] + 1;
		}

		/* wird das Objekt normalerweise zwischen links liegenden
		   Westwaenden gezeichnet? */
		if (objekte[i].richtung[AKTUELL] == LINKS_WEST)
		{
			/* befindet sich vor dem Objekt keine Westwand? */
			x = (int)ichpos->xgrob - (objekte[i].index[AKTUELL] - 1);
			y = (int)ichpos->ygrob - objekte[i].durchgang[AKTUELL] +
				(objekte[i].index[AKTUELL] - 1);
			if (objekte[i].index[AKTUELL] > 0 && !(x < 0 || y < 0) &&
				(feld[x][y][WEST].tueroffen ||
				feld[x][y][WEST].farbe == TRANSPARENT))

				/* dann kann das Objekt auch frueher gezeichnet werden */
				objekte[i].kann_frueher = 1,
				objekte[i].durchgang[FRUEHER] = objekte[i].durchgang[AKTUELL];

			/* befindet sich hinter dem Objekt keine Westwand? */
			x = (int)ichpos->xgrob - objekte[i].index[AKTUELL];
			y = (int)ichpos->ygrob - objekte[i].durchgang[AKTUELL] +
				objekte[i].index[AKTUELL];
			if (!(x < 0 || y < 0) &&
				(feld[x][y][WEST].tueroffen ||
				feld[x][y][WEST].farbe == TRANSPARENT))

				/* dann kann das Objekt auch spaeter gezeichnet werden */
				objekte[i].kann_spaeter = 1,
				objekte[i].durchgang[SPAETER] =
					objekte[i].durchgang[AKTUELL] + 1;
		}

		/* wird das Objekt normalerweise zwischen rechts liegenden
		   Nordwaenden gezeichnet? */
		if (objekte[i].richtung[AKTUELL] == RECHTS_NORD)
		{
			/* befindet sich vor dem Objekt keine Nordwand? */
			x = (int)ichpos->xgrob + objekte[i].durchgang[AKTUELL] -
				(objekte[i].index[AKTUELL] - 1);
			y = (int)ichpos->ygrob - (objekte[i].index[AKTUELL] - 1);
			if (objekte[i].index[AKTUELL] > 0 && !(x >= feldbreite || y < 0) &&
				(feld[x][y][NORD].tueroffen ||
				feld[x][y][NORD].farbe == TRANSPARENT))

				/* dann kann das Objekt auch frueher gezeichnet werden */
				objekte[i].kann_frueher = 1,
				objekte[i].durchgang[FRUEHER] = objekte[i].durchgang[AKTUELL];

			/* befindet sich hinter dem Objekt keine Nordwand? */
			x = (int)ichpos->xgrob + objekte[i].durchgang[AKTUELL] -
				objekte[i].index[AKTUELL];
			y = (int)ichpos->ygrob - objekte[i].index[AKTUELL];
			if (!(x >= feldbreite || y < 0) &&
				(feld[x][y][NORD].tueroffen ||
				feld[x][y][NORD].farbe == TRANSPARENT))

				/* dann kann das Objekt auch spaeter gezeichnet werden */
				objekte[i].kann_spaeter = 1,
				objekte[i].durchgang[SPAETER] =
					objekte[i].durchgang[AKTUELL] + 1;
		}

		/* wird das Objekt normalerweise zwischen links liegenden
		   Nordwaenden gezeichnet? */
		if (objekte[i].richtung[AKTUELL] == LINKS_NORD)
		{
			/* befindet sich vor dem Objekt keine Nordwand? */
			x = (int)ichpos->xgrob - objekte[i].durchgang[AKTUELL] +
				(objekte[i].index[AKTUELL] - 1);
			y = (int)ichpos->ygrob - (objekte[i].index[AKTUELL] - 1);
			if (objekte[i].index[AKTUELL] > 0 && !(x < 0 || y < 0) &&
				(feld[x][y][NORD].tueroffen ||
				feld[x][y][NORD].farbe == TRANSPARENT))

				/* dann kann das Objekt auch frueher gezeichnet werden */
				objekte[i].kann_frueher = 1,
				objekte[i].durchgang[FRUEHER] = objekte[i].durchgang[AKTUELL];

			/* befindet sich hinter dem Objekt keine Nordwand? */
			x = (int)ichpos->xgrob - objekte[i].durchgang[AKTUELL] +
				objekte[i].index[AKTUELL];
			y = (int)ichpos->ygrob - objekte[i].index[AKTUELL];
			if (!(x < 0 || y < 0) &&
				(feld[x][y][NORD].tueroffen ||
				feld[x][y][NORD].farbe == TRANSPARENT))

				/* dann kann das Objekt auch spaeter gezeichnet werden */
				objekte[i].kann_spaeter = 1,
				objekte[i].durchgang[SPAETER] =
					objekte[i].durchgang[AKTUELL] + 1;
		}

		/* wenn das Objekt normalerweise zwischen zwei Durchgaengen
		   gezeichnet wird, aber nicht ganz in einem Block liegt, dann
		   keine Ausweichposition berechnen */
	}

	/* bei allen Objekte ueberpruefen, ob der Index der Wand gueltig ist;
	   sonst korrigieren */
	for (i = 0; i < anzobjekte; i++)
	{
		/* ist der Index der normalen Position zu gross? */
		if ((objekte[i].richtung[AKTUELL] == RECHTS_OST ||
			objekte[i].richtung[AKTUELL] == LINKS_WEST) &&
			objekte[i].index[AKTUELL] > objekte[i].durchgang[AKTUELL])
		{
			/* dann spaeter (vor dem naechsten Durchgang) zeichnen */
			objekte[i].durchgang[AKTUELL]++;
			objekte[i].richtung[AKTUELL] = VORHER;
			objekte[i].index[AKTUELL] = 0;

			/* Ausweichpositionen sind jetzt ungueltig */
			objekte[i].kann_frueher = objekte[i].kann_spaeter = 0;
		}

		/* ist der Index der Ausweichposition (frueher zeichnen) zu gross? */
		if (objekte[i].kann_frueher &&
			objekte[i].richtung[FRUEHER] != VORHER &&
			objekte[i].index[FRUEHER] > objekte[i].durchgang[AKTUELL] - 1)

			/* dann Ausweichposition wieder loeschen */
			objekte[i].kann_frueher = 0;

		/* ist der Index der Ausweichposition (spaeter zeichnen) zu gross? */
		if (objekte[i].kann_spaeter &&
			objekte[i].richtung[SPAETER] != VORHER &&
			objekte[i].index[SPAETER] > objekte[i].durchgang[AKTUELL])

			/* dann Ausweichposition wieder loeschen */
			objekte[i].kann_spaeter = 0;
	}
}


/*
** objekte_sortieren
**  sortiert eine Liste von Objekten in die Reihenfolge, in der sie spaeter
**  gezeichnet werden muessen
**
** Parameter:
**  anzobjekte: Anzahl der Objekte
**  objekte: Liste von Positionen, etc. der Objekte
**
** Seiteneffekte:
**  *objekte wird umsortiert
*/
static void objekte_sortieren(int anzobjekte, struct objekt *objekte)
{
	int nochmal; /* Flag: Schleife nochmal durchlaufen? */
	int i;       /* Index fuer Objekte */

	/* keine Objekte zu sortieren? */
	if (anzobjekte < 2)
		return;

	/* Objekte sortieren */
	do
	{
		/* Flag initialisieren */
		nochmal = 0;

		/* Korrekturflag fuer alle Objekte initialisieren */
		for (i = 0; i < anzobjekte; i++)
			objekte[i].soll = KEINE_KORREKTUR;

		/* Sortieren und objekte[i].soll (in objekt_vergleich) setzen */
		qsort(objekte, anzobjekte, sizeof(struct objekt), objekt_vergleich);

		/* benoetigte Korrekturen fuer alle Objekte ausfuehren */
		for (i = 0; i < anzobjekte; i++)

			/* kann und soll dieses Objekt korrigiert werden? */
			if (objekte[i].soll == FRUEHER && objekte[i].kann_frueher ||
				objekte[i].soll == SPAETER && objekte[i].kann_spaeter)
			{
				/* noch eine Korrektur bei diesem Objekt nicht moeglich */
				objekte[i].kann_frueher = objekte[i].kann_spaeter = 0;

				/* korrigieren */
				if (objekte[i].soll == SPAETER)
				{
					objekte[i].durchgang[AKTUELL] =
						objekte[i].durchgang[SPAETER];
					objekte[i].richtung[AKTUELL] = objekte[i].richtung[SPAETER];
					objekte[i].index[AKTUELL] = objekte[i].index[SPAETER];
				}
				else
				{
					objekte[i].durchgang[AKTUELL] =
						objekte[i].durchgang[FRUEHER];
					objekte[i].richtung[AKTUELL] = objekte[i].richtung[FRUEHER];
					objekte[i].index[AKTUELL] = objekte[i].index[FRUEHER];
				}

				/* Schleife mindestens noch einmal durchlaufen */
				nochmal = 1;
			}
	}
	while (nochmal);
}


/*
** rechne_alles
**  berechnet die Koordinaten der bereits sortierten Waende und Kugeln und
**  traegt sie in vorinitialisierte Listen ein
**
** Parameter:
**  anzwaende: Zeiger auf die Anzahl der sichtbaren Waende
**  waende: Zeiger auf die Liste der sichtbaren Waende
**  anzkugeln: Zeiger auf die Anzahl der sichtbaren Kugeln
**  kugeln: Zeiger auf die Liste der sichtbaren Kugeln
**  feld: das Spielfeld
**  feldbreite: Breite des Spielfeldes
**  feldlaenge: Laenge des Spielfeldes
**  anzobjekte: Anzahl der Objekte
**  objekte: Liste von Positionen, etc. der Objekte
**  ichpos: Zeiger auf die Position des Spielers
**  ichblick: Blickrichtung des Spielers
**
** Seiteneffekte:
**  anzwaende, waende, anzkugeln und kugeln werden veraendert
*/
static void rechne_alles(int *anzwaende, struct flaeche **waende,
	int *anzkugeln, struct kugel **kugeln, block **feld, int feldbreite,
	int feldlaenge, int anzobjekte, struct objekt *objekte,
	struct position *ichpos, int ichblick)
{
	int i;         /* Index fuer Objekte */
	int x, y;      /* x/y-Koordinate des aktuellen Blocks */
	int durchgang; /* Nummer des Durchgangs beim Waendezeichnen:
	                  |delta x| + |delta y| */
	int index;     /* Nummer der Wand */
	int auslassen; /* Flag: Objekte und Waende im aktuellen Block
	                  nicht zeichnen */

	/* i initialisieren; Objekte hinter dem Spieler ueberspringen */
	for (i = 0; i < anzobjekte && objekte[i].durchgang[AKTUELL] < 0; i++);

	/* genuegend Durchgaenge machen, um alles im Abstand bis SICHTW zu
	   bearbeiten; die Waende eines Durchgangs verlaufen in Treppenstufen
	   um den Spieler herum */
	for (durchgang = 0; durchgang <= 2 * SICHTW; durchgang++)
	{
		/* Objekte zeichnen, die vor diesem Durchgang dran sind */
		while (i < anzobjekte && objekte[i].durchgang[AKTUELL] == durchgang &&
			objekte[i].richtung[AKTUELL] == VORHER)
		{
			if ((int)objekte[i].pos.xgrob >= (int)ichpos->xgrob - SICHTW &&
				(int)objekte[i].pos.xgrob <= (int)ichpos->xgrob + SICHTW &&
				(int)objekte[i].pos.ygrob >= (int)ichpos->ygrob - SICHTW)
				dreh_rechne_kugel(*anzwaende, *waende, anzkugeln, kugeln,
					&objekte[i], ichpos, ichblick);
			i++;
		}

		/* RECHTS_OST: rechts vom Spieler liegende Ostwaende */
		for (index = 0; index <= durchgang; index++)
		{
			/* Koordinaten des Blocks berechnen */
			x = (int)ichpos->xgrob + index;
			y = (int)ichpos->ygrob - durchgang + index;

			/* ist dieser Block auf dem Spielfeld und
			   innerhalb der Sichtweite? */
			auslassen = index > SICHTW || durchgang - index > SICHTW ||
				x >= feldbreite || y < 0;

			/* Objekte in diesem Block zeichnen */
			while (i < anzobjekte &&
				objekte[i].durchgang[AKTUELL] == durchgang &&
				objekte[i].richtung[AKTUELL] == RECHTS_OST &&
				objekte[i].index[AKTUELL] == index)
			{
				if (!auslassen)
					dreh_rechne_kugel(*anzwaende, *waende, anzkugeln, kugeln,
						&objekte[i], ichpos, ichblick);
				i++;
			}

			/* Ostwand zeichnen */
			if (!auslassen)
				rechne_ostwand(anzwaende, waende, feld, x, y,
					ichpos, ichblick);
		}

		/* LINKS_WEST: links vom Spieler liegende Westwaende */
		for (index = 0; index <= durchgang; index++)
		{
			/* Koordinaten des Blocks berechnen */
			x = (int)ichpos->xgrob - index;
			y = (int)ichpos->ygrob - durchgang + index;

			/* ist dieser Block auf dem Spielfeld und
			   innerhalb der Sichtweite? */
			auslassen = index > SICHTW || durchgang - index > SICHTW ||
				x < 0 || y < 0;

			/* Objekte in diesem Block zeichnen */
			while (i < anzobjekte &&
				objekte[i].durchgang[AKTUELL] == durchgang &&
				objekte[i].richtung[AKTUELL] == LINKS_WEST &&
				objekte[i].index[AKTUELL] == index)
			{
				if (!auslassen)
					dreh_rechne_kugel(*anzwaende, *waende, anzkugeln, kugeln,
						&objekte[i], ichpos, ichblick);
				i++;
			}

			/* Westwand zeichnen */
			if (!auslassen)
				rechne_westwand(anzwaende, waende, feld, x, y,
					ichpos, ichblick);
		}

		/* RECHTS_NORD: rechts vom Spieler liegende Nordwaende */
		for (index = 0; index <= durchgang; index++)
		{
			/* Koordinaten des Blocks berechnen */
			x = (int)ichpos->xgrob + durchgang - index;
			y = (int)ichpos->ygrob - index;

			/* ist dieser Block auf dem Spielfeld und
			   innerhalb der Sichtweite? */
			auslassen = index > SICHTW || durchgang - index > SICHTW ||
				x >= feldbreite || y < 0;

			/* Objekte in diesem Block zeichnen */
			while (i < anzobjekte &&
				objekte[i].durchgang[AKTUELL] == durchgang &&
				objekte[i].richtung[AKTUELL] == RECHTS_NORD &&
				objekte[i].index[AKTUELL] == index)
			{
				if (!auslassen)
					dreh_rechne_kugel(*anzwaende, *waende, anzkugeln, kugeln,
						&objekte[i], ichpos, ichblick);
				i++;
			}

			/* Nordwand zeichnen, es sei denn, es ist die letzte;
			   dann erst bei LINKS_NORD zeichnen */
			if (!auslassen && index < durchgang)
				rechne_nordwand(anzwaende, waende, feld, x, y,
					ichpos, ichblick);
		}

		/* LINKS_NORD: links vom Spieler liegende Nordwaende */
		for (index = 0; index <= durchgang; index++)
		{
			/* Koordinaten des Blocks berechnen */
			x = (int)ichpos->xgrob - durchgang + index;
			y = (int)ichpos->ygrob - index;

			/* ist dieser Block auf dem Spielfeld und
			   innerhalb der Sichtweite? */
			auslassen = index > SICHTW || durchgang - index > SICHTW ||
				x < 0 || y < 0;

			/* Objekte in diesem Block zeichnen */
			while (i < anzobjekte &&
				objekte[i].durchgang[AKTUELL] == durchgang &&
				objekte[i].richtung[AKTUELL] == LINKS_NORD &&
				objekte[i].index[AKTUELL] == index)
			{
				if (!auslassen)
					dreh_rechne_kugel(*anzwaende, *waende, anzkugeln, kugeln,
						&objekte[i], ichpos, ichblick);
				i++;
			}

			/* Nordwand zeichnen */
			if (!auslassen)
				rechne_nordwand(anzwaende, waende, feld, x, y,
					ichpos, ichblick);
		}
	}
}


/* bis hier lokaler Teil                       */
/***********************************************/
/* ab hier globaler Teil                       */


/*
** rechne3d
**  berechnet alle sichtbaren Waende und Kugeln
**
** Parameter:
**  objektdaten: Zeiger auf Liste/Anzahl der sichtbaren Waende und Kugeln
**  ich: Position/Blickrichtung von 'ich'
**  anzgegner: Anzahl der anderen Spieler
**  gegner: Liste von Positionen/Blickrichtungen/Farben der anderen Spieler
**  anzschuesse: Anzahl der Schuesse
**  schuesse: Liste von Positionen/Farben der Schuesse
**
** Seiteneffekte:
**  die Komponenten von *objektdaten werden gesetzt
*/
void rechne3d(struct objektdaten *objektdaten, struct spieler *ich,
	int anzgegner, struct spieler *gegner, int anzschuesse,
	struct schuss *schuesse)
{
	int i;                         /* Index fuer Objekte */
	u_char himricht;               /* grobe Blickrichtung von 'ich' */
	u_char nordbreite, nordlaenge; /* Breite/Lange des
	                                  gedrehten Labyrinths */
	int anzobjekte;                /* Anzahl der Objekte, die
	                                  beruecksichtigt werden */
	struct objekt *objekte;        /* Spieler/Schuesse, die
	                                  beruecksichtigt werden */
	u_char ichblick;               /* gedrehte Blickrichtung von 'ich' */
	struct position ichpos;        /* gedrehte Position von 'ich' */
	int *anzwaende;                /* Zeiger auf Anzahl der Waende */
	int *anzkugeln;                /* Zeiger auf Anzahl der Kugeln */
	struct flaeche **waende;       /* Zeiger auf Liste der Waende */
	struct kugel **kugeln;         /* Zeiger auf Liste der Kugeln */
	block **feld;                  /* gedrehtes Spielfeld */
	int anzoffen;                  /* Anzahl der offenen Tueren */
	u_char (*offen)[3];            /* offene Tueren: x/y/Himmelsrichtung */

	anzwaende = &objektdaten->anzwaende;
	waende = &objektdaten->waende;
	anzkugeln = &objektdaten->anzkugeln;
	kugeln = &objektdaten->kugeln;

	/* blickrichtung grob NORD = 0, WEST = 1, SUED = 2, OST = 3 */
	himricht = (ich->blick * (TRIGANZ / WINKANZ) + TRIGANZ / 8) %
		TRIGANZ / (TRIGANZ / 4);

	/* gedrehtes Spielfeld merken */
	feld = feld_himricht[himricht];

	/* bei Blickrichtung WEST und OST Breite/Laenge vertauschen */
	if (himricht % 2)
	{
		nordbreite = feldlaenge;
		nordlaenge = feldbreite;
	}
	else
	{
		nordbreite = feldbreite;
		nordlaenge = feldlaenge;
	}

	/* Position/Blickrichtung von 'ich' drehen */
	einnorden(&ichpos, &ich->pos, himricht, feldbreite, feldlaenge);
	ichblick = ((int)ich->blick - WINKANZ / 4 * (int)himricht +
		WINKANZ) % WINKANZ;

	liste_initialisieren(&anzobjekte, (void **)&objekte);
	liste_initialisieren(&anzoffen, (void **)&offen);

	/* Tueren, durch die der Spieler selbst laeuft, auch
	   als offen markieren */
	tueren_offen_markieren(&anzoffen, &offen, feld,
		nordbreite, nordlaenge, &ichpos, KUGELRAD);

	/* Objekte ausserhalb des Blickfeldes
	   koennten hier wegoptimiert werden */

	for (i = 0; i < anzgegner; i++)
	{
		struct position tmppos; /* temporaer fuer die neue Position */

		/* Position des Spielers drehen */
		einnorden(&tmppos, &gegner[i].pos, himricht,
			feldbreite, feldlaenge);

		/* Spieler nur beruecksichtigen, falls innerhalb der Sichtweite
		   und nicht hinter 'ich' */
		if (tmppos.ygrob <= ichpos.ygrob &&
			(int)tmppos.ygrob >= (int)ichpos.ygrob - SICHTW - 1)
		{
			liste_verlaengern(&anzobjekte, (void **)&objekte,
				sizeof(struct objekt));

			memcpy(&objekte[anzobjekte - 1].pos, &tmppos,
				sizeof(struct position));
			if (gegner[i].blick < 0 || gegner[i].blick >= WINKANZ)
				objekte[anzobjekte - 1].blick = -1;
			else
				objekte[anzobjekte - 1].blick =
					((int)gegner[i].blick - WINKANZ / 4 * (int)himricht +
					WINKANZ) % WINKANZ;
			objekte[anzobjekte - 1].farbe = gegner[i].farbe;
			objekte[anzobjekte - 1].radius = KUGELRAD;
		}

		tueren_offen_markieren(&anzoffen, &offen, feld,
			nordbreite, nordlaenge, &tmppos, KUGELRAD);
	}

	for (i = 0; i < anzschuesse; i++)
	{
		struct position tmppos; /* temporaer fuer die neue Position */

		/* Position des Schusses drehen */
		einnorden(&tmppos, &schuesse[i].pos, himricht,
			feldbreite, feldlaenge);

		/* Schuss nur beruecksichtigen, falls innerhalb der Sichtweite
		   und nicht hinter 'ich' */
		if (tmppos.ygrob <= ichpos.ygrob &&
			(int)tmppos.ygrob >= (int)ichpos.ygrob - SICHTW - 1)
		{
			liste_verlaengern(&anzobjekte, (void **)&objekte,
				sizeof(struct objekt));

			memcpy(&objekte[anzobjekte - 1].pos, &tmppos,
				sizeof(struct position));
			objekte[anzobjekte - 1].blick = -1;
			objekte[anzobjekte - 1].farbe = schuesse[i].farbe;
			objekte[anzobjekte - 1].radius = SCHUSSRAD;
		}
	}

	/* Daten, die zum Sortieren der Objekte benoetigt werden, berechnen */
	objekte_sortieren_vorberechnung(anzobjekte, objekte, &ichpos,
		feld, nordbreite, nordlaenge);

	/* Objekte im Verhaeltnis zu den Waenden sortieren */
	objekte_sortieren(anzobjekte, objekte);

	/* Objekte und Waende in der vorgegebenen Reihenfolge projezieren;
	   Daten fuer Clipping berechnen */
	rechne_alles(anzwaende, waende, anzkugeln, kugeln,
		feld, nordbreite, nordlaenge, anzobjekte, objekte, &ichpos, ichblick);

	/* markierte Waende wieder in vorherigen Zustand setzen */
	for (i = 0; i < anzoffen; i++)
		feld[offen[i][0]][offen[i][1]][offen[i][2]].tueroffen = 0;

	/* Speicher freigeben */
	liste_freigeben(&anzoffen, (void **)&offen);
	liste_freigeben(&anzobjekte, (void **)&objekte);
}
