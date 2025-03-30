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
** Datei: X_grafik.c
**
** Kommentar:
**  Grafikausgabe auf Basis von X11
*/


#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>

#include "global.h"
#include "speicher.h"
#include "farben.h"
#include "grafik.h"
#include "fehler.h"
#include "X_farben.h"
#include "X_grafik.h"

static char sccsid[] = "@(#)X_grafik.c	3.11 12/03/01";


#include "iMaze_1_4.xbm"


/* private Datenstruktur fuer das 3D-Blickfeld bzw. den Kompass */
struct blickfeld
{
	Pixmap zeichenfeld; /* eine Pixmap fuer Zeichnen im Hintergrund */
};

/* private Datenstruktur fuer die Karte */
struct karte
{
	Pixmap zeichenfeld1, zeichenfeld2; /* zwei Pixmaps fuer Zeichnen
	                                      im Hintergrund */
};

static GC farben[FARBENANZ]; /* GCs fuer alle Farben bzw.
                                schwarz/weiss-Raster */
static GC text_farbe, titelfarbe, schriftzug_farbe;
static int schwarzweiss;     /* Flag, ob die Farbausgabe mit
                                schwarz/weiss-Rastern erfolgt */

static XFontStruct *text_font, *titelfont;

/* 7-Segment-Darstellung der Ziffern 0 bis 9 und des Leerfeldes */
static int segment_belegung[11][7] =
{
	/* 0 */
	  1,
	1,  1,
	  0,
	1,  1,
	  1,

	/* 1 */
	  0,
	0,  1,
	  0,
	0,  1,
	  0,

	/* 2 */
	  1,
	0,  1,
	  1,
	1,  0,
	  1,

	/* 3 */
	  1,
	0,  1,
	  1,
	0,  1,
	  1,

	/* 4 */
	  0,
	1,  1,
	  1,
	0,  1,
	  0,

	/* 5 */
	  1,
	1,  0,
	  1,
	0,  1,
	  1,

	/* 6 */
	  1,
	1,  0,
	  1,
	1,  1,
	  1,

	/* 7 */
	  1,
	0,  1,
	  0,
	0,  1,
	  0,

	/* 8 */
	  1,
	1,  1,
	  1,
	1,  1,
	  1,

	/* 9 */
	  1,
	1,  1,
	  1,
	0,  1,
	  1,

	/* Leerfeld */
	  0,
	0,  0,
	  0,
	0,  0,
	  0
};

/* Beschreibung der sieben Segmente: */
static int segment_senkrecht[7] = { 0, 1, 1, 0, 1, 1, 0 }; /* senkrechtes
                                                              Segment? */
static int segment_x[7] = { 0, 0, 1, 0, 0, 1, 0 }; /* x-Koordinate */
static int segment_y[7] = { 0, 0, 0, 1, 1, 1, 2 }; /* y-Koordinate */


/*
** font_laden
**  erzeugt einen GC mit dem angegebenen Font in schwarz
**
** Parameter:
**  namen: Liste der zu probierenden Namen
**
** Seiteneffekte:
**  setzt *font
*/
static GC font_laden(Display *display, int screen, char **namen,
	XFontStruct **font)
{
	XFontStruct *f;
	XGCValues gc_init;
	char *meldung[] = { "iMaze - Fatal Error", "", "Font unavailable:",
		NULL, NULL };

	meldung[3] = namen[0];

	for (;; namen++)
	{
		if (*namen == NULL)
			uebler_fehler(meldung, NULL);

		if ((f = XLoadQueryFont(display, *namen)) != NULL)
			break;
	}

	*font = f;

	gc_init.foreground = BlackPixel(display, screen);
	gc_init.font = f->fid;
	gc_init.graphics_exposures = False;
	return XCreateGC(display, RootWindow(display, screen),
		GCForeground | GCFont | GCGraphicsExposures, &gc_init);
}


static void text_zentriert_malen(Display *display, Drawable drawable, GC gc,
	XFontStruct *font, int x, int y, char *text)
{
	XCharStruct format;
	int direction_hint, font_ascent, font_descent;

	XTextExtents(font, text, strlen(text), &direction_hint,
		&font_ascent, &font_descent, &format);
	XDrawString(display, drawable, gc, x - format.width / 2,
		y + format.ascent / 2, text, strlen(text));
}


/*
** x_skalieren
**  skaliert eine x-Koordinate einer Wand oder eines Clipping-Ausschnitts;
**  0 wird -1, FENSTER_BREITE wird breite, alle anderen Werte dazwischen
**
** Parameter:
**  x: alte x-Koordinate in virtuellen Bildpunkten
**  breite: tatsaechliche Fensterbreite in Pixeln
**
** Rueckgabewert:
**  neue x-Koordinate in Pixeln
*/
static int x_skalieren(int x, int breite)
{
	int produkt;

	produkt = (x - 1) * breite;
	if (produkt >= 0)
		return produkt / (FENSTER_BREITE - 1);
	else
		/* obskure Definition der C-Division ausgleichen */
		return (produkt - (FENSTER_BREITE - 2)) / (FENSTER_BREITE - 1);
}


/*
** malwaende
**  zeichnet alle Waende
**
** Parameter:
**  fenster: Parameter des Fensters, in dem gezeichnet wird
**  zeichenfeld: Zeichenfeld, in das gezeichnet wird
**  anzwaende: Anzahl der zu zeichnenden Waende
**  waende: Liste der zu zeichnenden Waende
*/
static void malwaende(struct fenster *fenster, Pixmap zeichenfeld,
	int anzwaende, struct flaeche *waende)
{
	int i; /* Index fuer die Waende */

	for (i = 0; i < anzwaende; i++)
	{
		XPoint ecke[7]; /* auf Pixel skalierte Koordinaten der Polygonecken;
		                   die Polygone werden in der Hoehe auf die
		                   Fensterhoehe begrenzt, um Ueberlaeufe zu
		                   vermeiden, dadurch bis zu 6 Ecken */
		int ecken;      /* Anzahl der Polygonecken */

		/* ausgehen von Viereckecn */
		ecken = 4;

		{
			int j; /* Index fuer die Ecken */

			/* alle Eckenkoordinaten skalieren */
			for (j = 0; j < ecken; j++)
			{
				ecke[j].x = x_skalieren(waende[i].ecke[j].x,
					fenster->breite);
				ecke[j].y = waende[i].ecke[j].y *
					fenster->hoehe / FENSTER_HOEHE;
			}
		}

		/* falls die linke und rechte obere Ecke nach oben aus dem Fenster
		   hinausragen, beide auf die obere Fenstergrenze setzen und beide
		   unteren auf die untere Fenstergrenze (die unteren Ecken sind
		   genau symmetrisch zu den oberen) */
		if (ecke[0].y < 0 && ecke[3].y < 0)
		{
			ecke[0].y = ecke[3].y = -1;
			ecke[1].y = ecke[2].y = fenster->hoehe;
		}
		/* falls nur eine der oberen Ecken nach oben aus dem Fenster
		   hinausragt, diese auf die obere Fenstergrenze setzen, die
		   zugehoerige untere auf die untere Fenstergrenze und oben und
		   unten jeweils eine weitere Ecke einfuegen */
		else if (ecke[0].y < 0 || ecke[3].y < 0)
		{
			int d; /* Hoehendifferenz der oberen Ecken */
			int x; /* x-Koordinate des Schnittpunktes obere
			          Linie/Fensterobergrenze */

			/* Schnittpunkt berechnen */
			d = ecke[3].y - ecke[0].y;
			x = ecke[0].x - ((ecke[0].y + 1) *
				(ecke[3].x - ecke[0].x) + d / 2) / d;

			/* 2 Ecken einfuegen */
			ecke[4].x = ecke[3].x;
			ecke[4].y = ecke[3].y;

			ecke[3].x = ecke[2].x;
			ecke[3].y = ecke[2].y;

			ecke[2].x = ecke[5].x = x;
			ecke[2].y = fenster->hoehe;
			ecke[5].y = -1;

			/* aus dem Viereck ist ein Sechseck geworden */
			ecken = 6;

			/* linke Ecken anpassen? */
			if (ecke[0].y < 0)
			{
				ecke[0].y = -1;
				ecke[1].y = fenster->hoehe;
			}
			/* sonst rechte Ecken */
			else
			{
				ecke[3].y = fenster->hoehe;
				ecke[4].y = -1;
			}
		}

		/* Polygon wieder schliessen */
		ecke[ecken].x = ecke[0].x;
		ecke[ecken].y = ecke[0].y;

		/* Polygon fuellen: falls nur schwarz/weiss, dann Raster fuer Tuer
		   und Wand unterscheiden, sonst Farbe benutzen */
		XFillPolygon(fenster->display, zeichenfeld,
			schwarzweiss ? farben[WANDFARBEN + waende[i].tuer] :
			farben[WANDFARBEN + waende[i].farbe - 1],
			ecke, ecken, Convex, CoordModeOrigin);

		/* Polygon umranden */
		XDrawLines(fenster->display, zeichenfeld,
			farben[RANDFARBE], ecke, ecken + 1, CoordModeOrigin);
	}
}


/*
** gesicht_berechnen
**  berechnet die Augen- und Mundpolygone sowie die Sichtbarkeitsflags
**  fuer ein Gesicht
**
** Parameter:
**  augelda: Zeiger auf Flag, ob linkes Auge sichtbar
**  augerda: Zeiger auf Flag, ob rechtes Auge sichtbar
**  mundda: Zeiger auf Flag, ob Mund sichtbar
**  augel: Feld mit Ecken des linken Auges
**  auger: Feld mit Ecken des rechten Auges
**  mund: Feld mit Ecken des Mundes
**  x0: x-Koordinate des linken Randes der Kugel
**  y0: y-Koordinate des oberen Randes der Kugel
**  breite: Breite der Kugel
**  hoehe: Hoehe der Kugel
**  drehwink: relative Blickrichtung des Spielers
**
** Seiteneffekte:
**  *augelda, *augerda, *mundda, augel, auger und mund werden gesetzt
*/
static void gesicht_berechnen(int *augelda, int *augerda, int *mundda,
	XPoint augel[AUGENECKENANZ], XPoint auger[AUGENECKENANZ],
	XPoint mund[2 * MUNDECKENANZ + 2], int x0, int y0, int breite,
	int hoehe, int drehwink)
{
	int i;     /* Index fuer die Augen- und Mundecken */
	int randx; /* Winkel des Randes, auf den unsichtbare Ecken
	              abgebildet werden */

	/* Augen und Mund als nicht sichtbar annehmen */
	*augelda = *augerda = *mundda = 0;

	/* falls keine gueltige Blickrichtung, kein Gesicht zeichnen */
	if (drehwink < 0 || drehwink >= TRIGANZ)
		return;

	/* falls Blickrichtung > 180 Grad, randx auf 270 Grad,
	   sonst auf 90 Grad (jeweils auf den naechstgelegenen Rand) */
	if (drehwink > TRIGANZ / 2)
		randx = 3 * TRIGANZ / 4;
	else
		randx = TRIGANZ / 4;

	/* Augen berechnen */
	for (i = 0; i < AUGENECKENANZ; i++)
	{
		int x, y;   /* Laengen- und Breitengrad einer Augenecke */
		int xl, xr; /* Laengengrad der Ecke beim linken und beim
		               rechten Auge */

		{
			int winkel; /* Winkel einer Augenecke um den Augenmittelpunkt */

			winkel = i * TRIGANZ / AUGENECKENANZ;

			/* Laengen- und Breitengrade der Augenecken ergeben
			   als Rechteckkoordinaten interpretiert eine Ellipse */
			x = costab[winkel] * (TRIGANZ * 3 / 64) / TRIGFAKTOR + drehwink;
			y = TRIGANZ / 3 + sintab[winkel] * (TRIGANZ / 6) / TRIGFAKTOR;
		}

		/* die Augen sind von der Gesichtsmitte um 1/16 Vollkreis
		   entfernt, modulo TRIGANZ reduzieren */
		xl = (x - TRIGANZ / 16 + TRIGANZ) % TRIGANZ;
		xr = (x + TRIGANZ / 16 + TRIGANZ) % TRIGANZ;

		/* falls der Winkel mehr als 90 Grad von der 180-Grad-Richtung
		   entfernt ist, wird der Punkt auf den Kugelrand abgebildet,
		   sonst wird er gezeichnet und das Auge ist sichtbar */
		if (xl < TRIGANZ / 4 || xl > TRIGANZ * 3 / 4)
			xl = randx;
		else
			*augelda = 1;

		/* falls der Winkel mehr als 90 Grad von der 180-Grad-Richtung
		   entfernt ist, wird der Punkt auf den Kugelrand abgebildet,
		   sonst wird er gezeichnet und das Auge ist sichtbar */
		if (xr < TRIGANZ / 4 || xr > TRIGANZ * 3 / 4)
			xr = randx;
		else
			*augerda = 1;

		/* die Laengen- und Breitengrade werden durch Parallelprojektion
		   auf die Bildebene abgebildet (Pfusch, es muesste
		   Zentralprojektion sein!) */
		augel[i].x = ((-sintab[xl] / 4) * (sintab[y / 2] / 4) /
			(TRIGFAKTOR / 16) + TRIGFAKTOR) * breite /
			(2 * TRIGFAKTOR) + x0;
		auger[i].x = ((-sintab[xr] / 4) * (sintab[y / 2] / 4) /
			(TRIGFAKTOR / 16) + TRIGFAKTOR) * breite /
			(2 * TRIGFAKTOR) + x0;
		augel[i].y = auger[i].y = (-costab[y / 2] + TRIGFAKTOR) *
			hoehe / (2 * TRIGFAKTOR) + y0;
	}

	/* Mund berechnen */
	for (i = 0; i <= MUNDECKENANZ; i++)
	{
		int x, y; /* Laengen- und Breitengrad einer Mundecke */

		{
			int winkel; /* Winkel einer Mundecke um den Mundmittelpunkt */

			winkel = (i + 3 * MUNDECKENANZ / 2) * TRIGANZ /
				(4 * MUNDECKENANZ);

			/* Laengen- und Breitengrade der Mundecken ergeben
			   als Rechteckkoordinaten interpretiert einen
			   Ellipsenbogen */
			x = (sintab[winkel] * (TRIGANZ / 5) / TRIGFAKTOR +
				drehwink + TRIGANZ) % TRIGANZ;
			y = TRIGANZ / 8 - costab[winkel] *
				(5 * TRIGANZ / 8) / TRIGFAKTOR;
		}

		/* falls der Winkel mehr als 90 Grad von der 180-Grad-Richtung
		   entfernt ist, wird der Punkt auf den Kugelrand abgebildet,
		   sonst wird er gezeichnet und der Mund ist sichtbar */
		if (x < TRIGANZ / 4 || x > 3 * TRIGANZ / 4)
			x = randx;
		else
			*mundda = 1;

		/* die Laengen- und Breitengrade werden durch Parallelprojektion
		   auf die Bildebene abgebildet (Pfusch, es muesste
		   Zentralprojektion sein!) */
		mund[i].x = ((-sintab[x] / 4) * (sintab[y / 2] / 4) /
			(TRIGFAKTOR / 16) + TRIGFAKTOR) * breite /
			(2 * TRIGFAKTOR) + x0;
		mund[i].y = (-costab[y / 2] + TRIGFAKTOR) *
			hoehe / (2 * TRIGFAKTOR) + y0;

		/* der andere Bogen des Mundes ist um 1/32 Vollkreis verschoben */
		y += TRIGANZ / 32;

		mund[2 * MUNDECKENANZ + 1 - i].x = ((-sintab[x] / 4) *
			(sintab[y / 2] / 4) /
			(TRIGFAKTOR / 16) + TRIGFAKTOR) * breite /
			(2 * TRIGFAKTOR) + x0;
		mund[2 * MUNDECKENANZ + 1 - i].y =
			(-costab[y / 2] + TRIGFAKTOR) *
			hoehe / (2 * TRIGFAKTOR) + y0;
	}
}


/*
** malkugel
**  zeichnet eine Kugel (3D-Spieler) mit oder ohne Clipping
**
** Parameter:
**  fenster: Parameter des Fensters, in dem gezeichnet wird
**  zeichenfeld: Zeichenfeld, in das gezeichnet wird
**  sichtbar: Liste der sichtbaren Bereiche
**  sichtanz: Anzahl der sichtbaren Bereiche, -1 fuer kein Clipping
**  x0: x-Koordinate der linken oberen Ecke des die Kugel umrandenden Rechtecks
**  y0: y-Koordinate der linken oberen Ecke des die Kugel umrandenden Rechtecks
**  breite: Breite der Kugel
**  hoehe: Hoehe der Kugel
**  blick: Blickrichtung des Spielers; 0 bis TRIGANZ - 1
**  farbe: Farbe der Kugel
*/
static void malkugel(struct fenster *fenster, Pixmap zeichenfeld,
	XRectangle *sichtbar, int sichtanz, int x0, int y0, int breite,
	int hoehe, int blick, int farbe)
{
	GC gc;                             /* GC fuer Clipping */
	int augelda, augerda, mundda;      /* Flags, welche Teile des
	                                      Gesichts sichtbar sind */
	XPoint augel[AUGENECKENANZ];       /* Ecken des linken Auges */
	XPoint auger[AUGENECKENANZ];       /* Ecken des rechten Auges */
	XPoint mund[2 * MUNDECKENANZ + 2]; /* Ecken des Mundes */

	/* Kugel nur fuellen, falls nicht transparent */
	if (farbe != TRANSPARENT)
	{
		gc = farben[KUGELFARBEN + farbe - 1];

		if (sichtanz >= 0)
			/* Clipping aktivieren */
			XSetClipRectangles(fenster->display, gc, 0, 0,
				sichtbar, sichtanz, YXBanded);

		/* Ellipse fuellen */
		XFillArc(fenster->display, zeichenfeld, gc, x0, y0,
			breite, hoehe, 0, 64 * 360);

		if (sichtanz >= 0)
			/* Clipping desaktivieren */
			XSetClipMask(fenster->display, gc, None);
	}

	/* Ecken der Augen- und Mundpolygone berechnen und
	   Sichtbarkeitsflags setzen */
	gesicht_berechnen(&augelda, &augerda, &mundda,
		augel, auger, mund, x0, y0, breite, hoehe, blick);

	gc = farben[KUGELGFARBEN + farbe];

	if (sichtanz >= 0)
		/* Clipping aktivieren */
		XSetClipRectangles(fenster->display, gc, 0, 0,
			sichtbar, sichtanz, YXBanded);

	/* linkes Auge zeichnen, falls sichtbar */
	if (augelda)
		XFillPolygon(fenster->display, zeichenfeld, gc, augel,
			AUGENECKENANZ, Nonconvex, CoordModeOrigin);

	/* rechtes Auge zeichnen, falls sichtbar */
	if (augerda)
		XFillPolygon(fenster->display, zeichenfeld, gc, auger,
			AUGENECKENANZ, Nonconvex, CoordModeOrigin);

	/* Mund zeichnen, falls sichtbar */
	if (mundda)
		XFillPolygon(fenster->display, zeichenfeld, gc, mund,
			2 * MUNDECKENANZ + 2, Nonconvex, CoordModeOrigin);

	if (sichtanz >= 0)
		/* Clipping desaktivieren */
		XSetClipMask(fenster->display, gc, None);

	gc = farben[KUGELRANDFARBE];

	if (sichtanz >= 0)
		/* Clipping aktivieren */
		XSetClipRectangles(fenster->display, gc, 0, 0,
			sichtbar, sichtanz, YXBanded);

	/* Kugelrand als Ellipse zeichnen */
	XDrawArc(fenster->display, zeichenfeld, gc,
		x0, y0, breite, hoehe, 0, 64 * 360);

	if (sichtanz >= 0)
		/* Clipping desaktivieren */
		XSetClipMask(fenster->display, gc, None);
}


/*
** malkugeln
**  zeichnet alle Kugeln (3D-Spieler) mit Clipping
**
** Parameter:
**  fenster: Parameter des Fensters, in dem gezeichnet wird
**  zeichenfeld: Zeichenfeld, in das gezeichnet wird
**  anzkugeln: Anzahl der Kugeln
**  kugeln: Liste der Kugeln
*/
static void malkugeln(struct fenster *fenster, Pixmap zeichenfeld,
	int anzkugeln, struct kugel *kugeln)
{
	int i;        /* Index fuer die Kugeln */
	int sichtanz; /* Anzahl der sichtbaren Bereiche */

	/* Kugeln rueckwaerts durchlaufen (nach Abstand aufsteigend sortiert) */
	for (i = anzkugeln - 1; i >= 0; i--)
	{
		XRectangle *sichtbar;      /* Liste der sichtbaren Bereiche,
		                              Koordinaten in Pixeln */
		int x0, y0, breite, hoehe; /* linke obere Ecke, Breite und Hoehe
		                              des (virtuellen) die Kugel umrandenden
		                              Rechtecks */

		/* Speicher fuer das Skalieren der sichtbaren Bereiche belegen,
		   hier ist kugeln[i].sichtanz > 0 */
		speicher_belegen((void **)&sichtbar,
			kugeln[i].sichtanz * sizeof(XRectangle));

		{
			int j; /* Index fuer die sichtbaren Bereiche */

			for (j = sichtanz = 0; j < kugeln[i].sichtanz; j++)
			{
				int breite; /* Zwischenspeicher fuer die Breite des Bereichs */

				/* sichtbaren Bereich berechnen/skalieren */
				sichtbar[sichtanz].x = x_skalieren(kugeln[i].sichtbar[j].x,
					fenster->breite) + 1;
				sichtbar[sichtanz].y = 0;
				breite = x_skalieren(kugeln[i].sichtbar[j].x +
					kugeln[i].sichtbar[j].breite, fenster->breite) -
					sichtbar[sichtanz].x;
				sichtbar[sichtanz].width = breite;
				sichtbar[sichtanz].height = fenster->hoehe;

				/* faellt der Bereich nicht weg? */
				if (breite > 0)
					/* dann zaehlen */
					sichtanz++;
			}

			/* keine sichtbaren Bereiche? */
			if (sichtanz < 1)
			{
				/* Speicher fuer das Skalieren der sichtbaren Bereiche wieder
				   freigeben */
				speicher_freigeben((void **)&sichtbar);
				return;
			}
		}

		/* (virtuelles) die Kugel umrandendes Rechteck berechnen */
		x0 = (kugeln[i].mittelpunkt.x - kugeln[i].radiusx) *
			fenster->breite / FENSTER_BREITE;
		y0 = (kugeln[i].mittelpunkt.y - kugeln[i].radiusy) *
			fenster->hoehe / FENSTER_HOEHE;
		breite = 2 * kugeln[i].radiusx *
			fenster->breite / FENSTER_BREITE;
		hoehe = 2 * kugeln[i].radiusy *
			fenster->hoehe / FENSTER_HOEHE;

		/* Schatten nur zeichnen, falls Radius (in y-Richtung) > 0 */
		if (kugeln[i].schattenry > 0)
		{
			/* Clipping aktivieren */
			XSetClipRectangles(fenster->display, farben[SCHATTENFARBE],
				0, 0, sichtbar, sichtanz, YXBanded);

			/* Schatten als gefuellte Ellipse zeichnen */
			XFillArc(fenster->display, zeichenfeld, farben[SCHATTENFARBE],
				x0,
				(kugeln[i].schatteny - kugeln[i].schattenry) *
					fenster->hoehe / FENSTER_HOEHE,
				breite,
				2 * kugeln[i].schattenry *
					fenster->hoehe / FENSTER_HOEHE,
				0, 64 * 360);

			/* Clipping desaktivieren */
			XSetClipMask(fenster->display, farben[SCHATTENFARBE], None);
		}

		/* die Kugel mit Clipping zeichnen */
		malkugel(fenster, zeichenfeld, sichtbar, sichtanz,
			x0, y0, breite, hoehe, kugeln[i].blick, kugeln[i].farbe);

		/* Speicher fuer das Skalieren der sichtbaren Bereiche wieder
		   freigeben */
		speicher_freigeben((void **)&sichtbar);
	}
}


/*
** mallinie
**  zeichnet eine Linie (Wand) in einem angegebenen Muster
**
** Parameter:
**  fenster: Parameter des Fensters, in dem gezeichnet wird
**  zeichenfeld: Zeichenfeld, in das gezeichnet wird
**  gc: GC, der zum Zeichnen verwendet wird
**  x: x-Koordinaten des linken/oberen Linienendes
**  y: y-Koordinaten des linken/oberen Linienendes
**  laenge: Laenge der Linie
**  senkrecht: Flag, ob die Linie senkrecht laeuft
**  typ: Typ der Linie (KEINELINIE/WANDLINIE/TUERLINIE)
*/
static void mallinie(struct fenster *fenster, Pixmap zeichenfeld, GC gc, int x,
	int y, int laenge, u_char senkrecht, u_char typ)
{
	static char gepunktet[] = { 1, 1 }; /* Muster fuer gepunktete Linie
	                                       (KEINELINIE) */
	char gestrichelt[2]; /* Muster fuer gestrichelte Linie (WANDLINIE) */

	/* Linientypen unterscheiden */
	switch (typ)
	{
		case KEINELINIE: /* gepunktete Linie, Wand von dieser Seite
		                    unsichtbar */
			XSetLineAttributes(fenster->display, gc, 0,
				LineOnOffDash, CapButt, JoinRound);
			XSetDashes(fenster->display, gc, 0, gepunktet, 2);
			break;

		case WANDLINIE: /* durchgezogene Linie, tatsaecliche Wand */
			XSetLineAttributes(fenster->display, gc, 0,
				LineSolid, CapButt, JoinRound);
			break;

		case TUERLINIE: /* gestrichelte Linie, Tuer */

			/* falls Strichmuster zu kurz, nicht zeichnen */
			if ((gestrichelt[0] = gestrichelt[1] = (laenge + 3) / 6) == 0)
				return;

			XSetLineAttributes(fenster->display, gc, 0,
				LineOnOffDash, CapButt, JoinRound);
			XSetDashes(fenster->display, gc,
				(laenge + 6) / 12, gestrichelt, 2);
			break;
	}

	/* Linie senkrecht zeichnen? */
	if (senkrecht)
		XDrawLine(fenster->display, zeichenfeld, gc,
			x, y, x, y + laenge);
	/* sonst waagerecht */
	else
		XDrawLine(fenster->display, zeichenfeld, gc,
			x, y, x + laenge, y);
}


/*
** malkreise
**  zeichnet Kreise (Spieler) auf der Karte
**
** Parameter:
**  fenster: Parameter des Fensters, in dem gezeichnet wird
**  zeichenfeld: Zeichenfeld, in das gezeichnet wird
**  anzkreise: Anzahl der zu zeichnenden Kreise
**  kreise: Liste der zu zeichnenden Kreise
*/
static void malkreise(struct fenster *fenster, Pixmap zeichenfeld,
	int anzkreise, struct kartenkreis *kreise)
{
	int i; /* Index fuer die Kreise */

	for (i = 0; i < anzkreise; i++)
	{
		int x, y, rx, ry; /* Mittelpunkt und Radius des Kreises in Pixeln */

		/* Mittelpunkt und Radius skalieren */
		x = kreise[i].mittelpunkt.x * fenster->breite / FENSTER_BREITE;
		y = kreise[i].mittelpunkt.y * fenster->hoehe / FENSTER_HOEHE;
		rx = kreise[i].radiusx * fenster->breite / FENSTER_BREITE;
		ry = kreise[i].radiusy * fenster->hoehe / FENSTER_HOEHE;

		/* Kreis nur fuellen, falls nicht transparent */
		if (kreise[i].farbe != TRANSPARENT)
			XFillArc(fenster->display, zeichenfeld,
				farben[KUGELFARBEN + kreise[i].farbe - 1],
				x - rx, y - ry, max(2 * rx - 1, 0), max(2 * ry - 1, 0),
				0, 64 * 360);

		/* Rand des Kreises zeichnen */
		XDrawArc(fenster->display, zeichenfeld, farben[KUGELRANDFARBE],
			x - rx, y - ry, max(2 * rx - 1, 0), max(2 * ry - 1, 0),
			0, 64 * 360);
	}
}


/*
** kopier_kreise
**  kopiert Kreise aus einem Fenster/Pixmap in ein anderes
**
** Parameter:
**  fenster: Parameter des Fensters, zu dem die Pixmaps gehoeren
**  von: Fenster/Pixmap, aus dem kopiert wird
**  nach: Fenster/Pixmap, in das kopiert wird
**  gc: Dummy-GC, der beim Kopieren verwendet wird
**  anzkreise: Anzahl der Kreise
**  kreise: Liste der Kreise
*/
static void kopier_kreise(struct fenster *fenster, Drawable von, Drawable nach,
	GC gc, int anzkreise, struct kartenkreis *kreise)
{
	int i; /* Index fuer die Kreise */

	for (i = 0; i < anzkreise; i++)
	{
		int x, y, rx, ry; /* Mittelpunkt und Radius des Kreises in Pixeln */

		/* Mittelpunkt und Radius skalieren */
		x = kreise[i].mittelpunkt.x * fenster->breite / FENSTER_BREITE;
		y = kreise[i].mittelpunkt.y * fenster->hoehe / FENSTER_HOEHE;
		rx = kreise[i].radiusx * fenster->breite / FENSTER_BREITE;
		ry = kreise[i].radiusy * fenster->hoehe / FENSTER_HOEHE;

		/* Rechteck um den Kreis kopieren */
		XCopyArea(fenster->display, von, nach, gc,
			x - rx, y - ry, max(2 * rx, 1), max(2 * ry, 1), x - rx, y - ry);
	}
}


/*
** zeichne_fadenkreuz
**  zeichnet ein Fadenkreuz
**
** Parameter:
**  fenster: Parameter des Fensters, in dem gezeichnet wird
**  zeichenfeld: Zeichenfeld, in das gezeichnet wird
*/
static void zeichne_fadenkreuz(struct fenster *fenster, Pixmap zeichenfeld)
{
	XSegment kreuz[4]; /* 4 nicht verbundene Linien */

	/* 2 senkrechte Linien erzeugen */
	kreuz[0].x1 = kreuz[0].x2 = kreuz[1].x1 = kreuz[1].x2 =
		fenster->breite / 2;
	kreuz[0].y1 = fenster->hoehe * (KREUZ_FAKTOR / 2 - KREUZ_INRAD) /
		KREUZ_FAKTOR;
	kreuz[0].y2 = fenster->hoehe * (KREUZ_FAKTOR / 2 - KREUZ_AUSRAD) /
		KREUZ_FAKTOR;
	kreuz[1].y1 = fenster->hoehe * (KREUZ_FAKTOR / 2 + KREUZ_INRAD) /
		KREUZ_FAKTOR;
	kreuz[1].y2 = fenster->hoehe * (KREUZ_FAKTOR / 2 + KREUZ_AUSRAD) /
		KREUZ_FAKTOR;

	/* 2 waagerechte Linien erzeugen */
	kreuz[2].y1 = kreuz[2].y2 = kreuz[3].y1 = kreuz[3].y2 =
		fenster->hoehe / 2;
	kreuz[2].x1 = fenster->breite * (KREUZ_FAKTOR * 3 / 4 - KREUZ_INRAD) /
		(KREUZ_FAKTOR * 3 / 2);
	kreuz[2].x2 = fenster->breite * (KREUZ_FAKTOR * 3 / 4 - KREUZ_AUSRAD) /
		(KREUZ_FAKTOR * 3 / 2);
	kreuz[3].x1 = fenster->breite * (KREUZ_FAKTOR * 3 / 4 + KREUZ_INRAD) /
		(KREUZ_FAKTOR * 3 / 2);
	kreuz[3].x2 = fenster->breite * (KREUZ_FAKTOR * 3 / 4 + KREUZ_AUSRAD) /
		(KREUZ_FAKTOR * 3 / 2);

	/* Linien in RANDFARBE zeichnen */
	XDrawSegments(fenster->display, zeichenfeld,
		farben[RANDFARBE], kreuz, 4);
}


/*
** zeichne_linien
**  zeichnet die Waende in die Karte
**
** Parameter:
**  fenster: Parameter des Fensters, in dem gezeichnet wird
**  zeichenfeld: Zeichenfeld, in das gezeichnet wird
**  anzlinien: Anzahl der Linienpaare, die gezeichnet werden sollen
**  linien: Liste der Linienpaare, die gezeichnet werden sollen
*/
static void zeichne_linien(struct fenster *fenster, Pixmap zeichenfeld,
	int anzlinien, struct kartenlinie *linien)
{
	GC gc; /* GC, mit dem die Linien gezeichnet werden */
	int i; /* Index fuer die Linienpaare */

	/* Zeichenfeld loeschen */
	XFillRectangle(fenster->display, zeichenfeld, farben[HIMMELFARBE],
		0, 0, fenster->breite, fenster->hoehe);

	/* Linien in Randfarbe zeichnen */
	gc = farben[RANDFARBE];

	for (i = 0; i < anzlinien; i++)
	{
		int x, y; /* Koordinaten des linken/oberen Wandendes in Pixeln */

		/* Koordinaten skalieren */
		x = linien[i].ecke.x * fenster->breite / FENSTER_BREITE;
		y = linien[i].ecke.y * fenster->hoehe / FENSTER_HOEHE;

		/* senkrechte Linien zeichen? */
		if (linien[i].senkrecht)
		{
			/* linke Linie zeichnen */
			mallinie(fenster, zeichenfeld, gc, x - 1, y, linien[i].laenge *
				fenster->hoehe / FENSTER_HOEHE, 1, linien[i].typ1);

			/* rechte Linie zeichnen */
			mallinie(fenster, zeichenfeld, gc, x, y, linien[i].laenge *
				fenster->hoehe / FENSTER_HOEHE, 1, linien[i].typ2);
		}
		/* sonst waagerechte Linien zeichnen */
		else
		{
			/* obere Linie zeichnen */
			mallinie(fenster, zeichenfeld, gc, x, y - 1, linien[i].laenge *
				fenster->breite / FENSTER_BREITE, 0, linien[i].typ1);

			/* untere Linie zeichnen */
			mallinie(fenster, zeichenfeld, gc, x, y, linien[i].laenge *
				fenster->breite / FENSTER_BREITE, 0, linien[i].typ2);
		}
	}

	/* LineAttributes restaurieren */
	XSetLineAttributes(fenster->display, gc, 0,
		LineSolid, CapButt, JoinRound);
}


/*
** malziffer
**  zeichnet eine 7-Segment-Ziffer
**
** Parameter:
**  fenster: Parameter des Fensters, in dem gezeichnet wird
**  zeichenfeld: Zeichenfeld, in das gezeichnet wird
**  fg: Vordergrundfarbe
**  bg: Hintergrundfarbe
**  x0: x-Koordinate der linken oberen Ecke
**  y0: y-Koordinate der linken oberen Ecke
**  breite: Ziffernbreite
**  hoehe: Haelfte der Ziffernhoehe
**  wert: die Ziffer oder 10 fuer Leerfeld
*/
static void malziffer(struct fenster *fenster, Pixmap zeichenfeld, int fg,
	int bg, int x0, int y0, int breite, int hoehe, int wert)
{
	GC gc;
	int i; /* Index fuer Segmente */

	for (i = 0; i < 7; i++)
	{
		gc = farben[segment_belegung[wert][i] ? fg : bg];

		if (segment_senkrecht[i])
			XDrawLine(fenster->display, zeichenfeld, gc,
				x0 + breite * segment_x[i],
				y0 + hoehe * segment_y[i] + 1,
				x0 + breite * segment_x[i],
				y0 + hoehe * (segment_y[i] + 1) - 1);
		else
			XDrawLine(fenster->display, zeichenfeld, gc,
				x0 + breite * segment_x[i] + 1,
				y0 + hoehe * segment_y[i],
				x0 + breite * (segment_x[i] + 1) - 1,
				y0 + hoehe * segment_y[i]);
	}
}


/*
** malzahl
**  zeichnet eine dreistellige Zahl aus 7-Segment-Ziffern
**
** Parameter:
**  fenster: Parameter des Fensters, in dem gezeichnet wird
**  zeichenfeld: Zeichenfeld, in das gezeichnet wird
**  fg: Vordergrundfarbe
**  bg: Hintergrundfarbe
**  x0: x-Koordinate der linken oberen Ecke
**  y0: y-Koordinate der linken oberen Ecke
**  breite: Ziffernbreite
**  hoehe: Haelfte der Ziffernhoehe
**  abstand: horizontaler Abstand zwischen den Ziffern
**  wert: die Zahl
*/
static void malzahl(struct fenster *fenster, Pixmap zeichenfeld, int fg,
	int bg, int x0, int y0, int breite, int hoehe, int abstand, int wert)
{
	int i; /* Index fuer die Ziffern */

	for (i = 3; i--;)
	{
		/* letzte Ziffer zeichnen */
		malziffer(fenster, zeichenfeld, fg, bg,
			x0 + (breite + abstand) * i, y0,
			breite, hoehe, wert || i == 2 ? wert % 10 : 10);

		/* letzte Ziffer wegwerfen */
		wert /= 10;
	}
}


/*
** bitmap_skalieren
**  skaliert eine Bitmap von breite * hoehe auf
**  breite*xfaktor * hoehe*yfaktor, indem die Daten wiederholt werden
**
** Parameter:
**  display: Display, fuer das die Bitmap berechnet werden soll
**  screen: Screen, fuer den die Bitmap berechnet werden soll
**  daten: Daten der unskalierten Bitmap, je 8 Pixelwerte in einem Byte
**  breite: urspruengliche Breite der Bitmap geteilt durch 8
**  hoehe: urspruengliche Hoehe der Bitmap
**  xfaktor: Skalierungsfaktor fuer die Breite
**  yfaktor: Skalierungsfaktor fuer die Hoehe
**
** Rueckgabewert:
**  die skalierte Bitmap
*/
static Pixmap bitmap_skalieren(Display *display, int screen, char *daten,
	unsigned breite, unsigned hoehe, unsigned xfaktor, unsigned yfaktor)
{
	char *daten_neu; /* die skalierten Daten */
	Pixmap bitmap;   /* die erzeugte Bitmap */
	int x, y;        /* Index fuer die Bits in den urspruenglichen Daten */
	int x2, y2;      /* Index fuer die Wiederholungen */

	/* Speicher fuer die skalierten Daten belegen */
	speicher_belegen((void **)&daten_neu,
		breite * hoehe * xfaktor * yfaktor);

	/* Daten wiederholt kopieren */
	for (x2 = 0; x2 < xfaktor; x2++)
		for (y2 = 0; y2 < yfaktor; y2++)

			/* Daten einmal kopieren */
			for (x = 0; x < breite; x++)
				for (y = 0; y < hoehe; y++)

					/* ein Byte kopieren */
					daten_neu[x + breite * (x2 + xfaktor *
						(y + hoehe * y2))] = daten[x + breite * y];

	/* Bitmap aus den skalierten Daten erzeugen, Breite in Pixeln statt
	   Byte angeben */
	bitmap = XCreateBitmapFromData(display, RootWindow(display, screen),
		daten_neu, 8 * breite * xfaktor, hoehe * yfaktor);

	/* Speicher fuer die skalierten Daten wieder freigeben */
	speicher_freigeben((void **)&daten_neu);

	return bitmap;
}


/* bis hier lokaler Teil                       */
/***********************************************/
/* ab hier globaler Teil                       */


/*
** X_zeichne_blickfeld
**  zeichnet ein 3D-Blickfeld
**
** Parameter:
**  X_daten: Zeiger auf die private Datenstruktur
**  fenster: Parameter des Fensters, in dem gezeichnet wird
**  daten: Zeiger auf eine Struktur mit der Liste der Waende, der Kugeln
**         und deren Anzahl, sowie einem Flag, ob ein Fadenkreuz
**         gezeichnet werden soll, und ob ueberhaupt gezeichnet werden soll
*/
void X_zeichne_blickfeld(void *X_daten, struct fenster *fenster, void *daten)
{
	struct blickfeld *blickfeld;    /* private Daten */
	struct blickfelddaten *objekte; /* die Grafikobjekte */
	Pixmap zeichenfeld;             /* Zeichenfeld, in das im Hintergrund
	                                   gezeichnet wird */

	blickfeld = X_daten;
	objekte = daten;
	zeichenfeld = blickfeld->zeichenfeld;

	/* nur Titelbild zeichnen? */
	if (objekte->titelbild)
	{
		XGCValues gc_v;
		int breite, x, y, kugelbreite, delta;

		/* alles mit BODENFARBE fuellen */
		XFillRectangle(fenster->display, zeichenfeld,
			farben[BODENFARBE],
			0, 0, fenster->breite, fenster->hoehe);

		breite = iMaze_1_4_width;
		x = (fenster->breite - breite) / 2;
		y = fenster->hoehe * 2 / 5 - iMaze_1_4_height / 2;

		gc_v.ts_x_origin = x;
		gc_v.ts_y_origin = y;
		XChangeGC(fenster->display, schriftzug_farbe,
			GCTileStipXOrigin | GCTileStipYOrigin, &gc_v);

		XFillRectangle(fenster->display, zeichenfeld, schriftzug_farbe,
			x, y, breite, iMaze_1_4_height);

		text_zentriert_malen(fenster->display, zeichenfeld,
			titelfarbe, titelfont, fenster->breite / 2,
			fenster->hoehe * 3 / 4 -
				(titelfont->ascent + titelfont->descent) *
				2 / 3,
			"Copyright \xa9 1993-2001 by");

		text_zentriert_malen(fenster->display, zeichenfeld,
			titelfarbe, titelfont, fenster->breite / 2,
			fenster->hoehe * 3 / 4 +
				(titelfont->ascent + titelfont->descent) *
				2 / 3,
			"Hans-Ulrich Kiel & J\xf6rg Czeranski");

		kugelbreite = (fenster->breite - breite) / 5;
		delta = (fenster->breite - breite - 2 * kugelbreite) / 4;
		if (kugelbreite < 20)
			kugelbreite = 20;

		malkugel(fenster, zeichenfeld, NULL, -1,
			(fenster->breite - breite) / 2 - delta - kugelbreite,
			fenster->hoehe * 2 / 5 - kugelbreite / 2,
			kugelbreite, kugelbreite,
			TRIGANZ * 15 / 32, 1);

		malkugel(fenster, zeichenfeld, NULL, -1,
			(fenster->breite + breite) / 2 + delta,
			fenster->hoehe * 2 / 5 - kugelbreite / 2,
			kugelbreite, kugelbreite,
			TRIGANZ * 17 / 32, 2);

		/* Zeichenfeld in das Fenster kopieren */
		XCopyArea(fenster->display, zeichenfeld, fenster->window,
			farben[RANDFARBE], 0, 0,
			fenster->breite, fenster->hoehe, 0, 0);
		return;
	}


	/* Hintergrund zeichnen? */
	if (objekte->objekte.hintergrund_zeichnen)
	{

		/* Himmel zeichnen */
		XFillRectangle(fenster->display, zeichenfeld, farben[HIMMELFARBE],
			0, 0, fenster->breite, fenster->hoehe / 2);

		/* Boden zeichnen */
		XFillRectangle(fenster->display, zeichenfeld, farben[BODENFARBE], 0,
			fenster->hoehe / 2, fenster->breite, (fenster->hoehe + 1) / 2);

		/* Trennlinie zwischen Himmel und Boden zeichnen */
		XDrawLine(fenster->display, zeichenfeld, farben[RANDFARBE],
			0, fenster->hoehe / 2, fenster->breite - 1, fenster->hoehe / 2);
	}
	else
	{
		/* alles mit BODENFARBE fuellen */
		XFillRectangle(fenster->display, zeichenfeld, farben[BODENFARBE],
			0, 0, fenster->breite, fenster->hoehe);
	}

	/* Waende zeichnen */
	malwaende(fenster, zeichenfeld, objekte->objekte.anzwaende,
		objekte->objekte.waende);

	/* Kugeln zeichnen */
	malkugeln(fenster, zeichenfeld, objekte->objekte.anzkugeln,
		objekte->objekte.kugeln);

	/* Fadenkreuz zeichnen */
	if (objekte->objekte.hintergrund_zeichnen && objekte->fadenkreuz)
		zeichne_fadenkreuz(fenster, zeichenfeld);

	if (objekte->objekte.text != NULL)
		text_zentriert_malen(fenster->display, zeichenfeld,
			text_farbe, text_font, fenster->breite / 2,
			fenster->hoehe * 15 / 16, objekte->objekte.text);


	/* Zeichenfeld in das Fenster kopieren */
	XCopyArea(fenster->display, zeichenfeld, fenster->window,
		farben[RANDFARBE], 0, 0, fenster->breite, fenster->hoehe, 0, 0);
}


/*
** X_zeichne_karte
**  loescht die Spieler an deren alter Position von der Karte und
**  zeichnet sie neu
**
** Parameter:
**  X_daten: Zeiger auf die private Datenstruktur
**  fenster: Parameter des Fensters, in dem gezeichnet wird
**  daten: Zeiger auf eine Struktur mit der Liste der Linien, der Kreise,
**         Anzahl, deren vorige Positionen und deren vorige Anzahl
*/
void X_zeichne_karte(void *X_daten, struct fenster *fenster, void *daten)
{
	struct karte *karte; /* private Daten */
	struct grundrissdaten *grundriss; /* die Grafikobjekte */

	karte = X_daten;
	grundriss = daten;

	/* Kreise aus zeichenfeld2 in zeichenfeld1 kopieren, dadurch loeschen */
	kopier_kreise(fenster, karte->zeichenfeld2, karte->zeichenfeld1,
		farben[RANDFARBE], grundriss->anzkreise_alt, grundriss->kreise_alt);

	/* Kreise an neuer Position in zeichenfeld1 zeichnen */
	malkreise(fenster, karte->zeichenfeld1,
		grundriss->anzkreise, grundriss->kreise);

	/* Bereiche an alten und neuen Positionen aus zeichenfeld1 in das
	   Fenster kopieren */
	kopier_kreise(fenster, karte->zeichenfeld1, fenster->window,
		farben[RANDFARBE], grundriss->anzkreise_alt, grundriss->kreise_alt);
	kopier_kreise(fenster, karte->zeichenfeld1, fenster->window,
		farben[RANDFARBE], grundriss->anzkreise, grundriss->kreise);
}


/*
** X_zeichne_grundriss
**  zeichnet den Grundriss der Karte und die Spieler neu
**
** Parameter:
**  X_daten: Zeiger auf die private Datenstruktur
**  fenster: Parameter des Fensters, in dem gezeichnet wird
**  daten: Zeiger auf eine Struktur mit der Liste der Linien, der Kreise
**         und deren Anzahl
*/
void X_zeichne_grundriss(void *X_daten, struct fenster *fenster, void *daten)
{
	struct karte *karte; /* private Daten */
	struct grundrissdaten *grundriss; /* die Grafikobjekte */

	karte = X_daten;
	grundriss = daten;

	/* Grundriss der Karte in zeichenfeld2 zeichnen */
	zeichne_linien(fenster, karte->zeichenfeld2,
		grundriss->anzlinien, grundriss->linien);

	/* Grundriss aus zeichenfeld2 in zeichenfeld1 kopieren */
	XCopyArea(fenster->display, karte->zeichenfeld2, karte->zeichenfeld1,
		farben[RANDFARBE], 0, 0, fenster->breite, fenster->hoehe, 0, 0);

	/* Kreise in zeichenfeld1 zeichnen (Anzahl/Liste der zu loeschenden
	   Kreise wird an X_zeichne_grundriss als 0/NULL uebergeben) */
	X_zeichne_karte(karte, fenster, grundriss);

	/* Karte aus zeichenfeld1 in das Fenster kopieren */
	XCopyArea(fenster->display, karte->zeichenfeld1, fenster->window,
		farben[RANDFARBE], 0, 0, fenster->breite, fenster->hoehe, 0, 0);
}


/*
** X_zeichne_kompass
**  zeichnet den Kompass
**
** Parameter:
**  X_daten: Zeiger auf die private Datenstruktur
**  fenster: Parameter des Fensters, in dem gezeichnet wird
**  daten: Zeiger auf die Blickrichtung des Spielers
*/
void X_zeichne_kompass(void *X_daten, struct fenster *fenster, void *daten)
{
	struct blickfeld *kompass; /* private Daten */
	int blickrichtung;         /* die Blickrichtung des Spielers; -1, falls
			                      keine Nadel zu zeichnen */
	Pixmap zeichenfeld;        /* Zeichenfeld, in das im Hintergrund
	                              gezeichnet wird */

	kompass = X_daten;
	blickrichtung = *(int *)daten;
	zeichenfeld = kompass->zeichenfeld;

	/* Zeichenfeld loeschen */
	XFillRectangle(fenster->display, zeichenfeld, farben[HIMMELFARBE],
		0, 0, fenster->breite, fenster->hoehe);


	/* 8 Kompassegmente mit je 4 Ecken zeichnen */
	{
		int i;            /* Index fuer Segmente */
		XPoint ecken[4];  /* 4 Punkte fuer ein Kompasssegment */
		XPoint umriss[9]; /* 9 Punkte fuer den Kompassumriss */

		umriss[0].x = umriss[8].x = 0;
		umriss[1].x = umriss[7].x = fenster->breite * 3 / 8;
		umriss[2].x = umriss[6].x = fenster->breite / 2;
		umriss[3].x = umriss[5].x = fenster->breite * 5 / 8;
		umriss[4].x = fenster->breite;

		umriss[2].y = 0;
		umriss[1].y = umriss[3].y = fenster->hoehe * 3 / 8;
		umriss[0].y = umriss[4].y = umriss[8].y = fenster->hoehe / 2;
		umriss[5].y = umriss[7].y = fenster->hoehe * 5 / 8;
		umriss[6].y = fenster->hoehe;

		ecken[0].x = ecken[3].x = fenster->breite / 2;
		ecken[0].y = ecken[3].y = fenster->hoehe / 2;

		ecken[1].x = umriss[0].x;
		ecken[1].y = umriss[0].y;

		for (i = 1; i <= 8; i += 2)
		{
			ecken[2].x = umriss[i].x;
			ecken[2].y = umriss[i].y;

			/* Polygon fuellen */
			XFillPolygon(fenster->display, zeichenfeld,
				farben[KOMPASSFARBE1], ecken, 3, Convex, CoordModeOrigin);

			ecken[1].x = umriss[i + 1].x;
			ecken[1].y = umriss[i + 1].y;

			/* Polygon fuellen */
			XFillPolygon(fenster->display, zeichenfeld,
				farben[KOMPASSFARBE2], ecken, 3, Convex, CoordModeOrigin);
		}

		/* Kompass umranden */
		XDrawLines(fenster->display, zeichenfeld, farben[RANDFARBE],
			umriss, 9, CoordModeOrigin);
	}


	/* Zeiger zeichnen? */
	if (blickrichtung >= 0)
	{
		XPoint ecken[5];      /* 5 Punkte fuer den Zeiger */
		int sinwert, coswert; /* Sinus/Cosinus der Blickrichtung */

		sinwert = sintab[blickrichtung];
		coswert = costab[blickrichtung];

		ecken[0].x = ecken[4].x = fenster->breite / 2 *
			(TRIGFAKTOR - sinwert * 7 / 8) / TRIGFAKTOR;
		ecken[0].y = ecken[4].y = fenster->hoehe / 2 *
			(TRIGFAKTOR - coswert * 7 / 8) / TRIGFAKTOR;

		ecken[1].x = fenster->breite / 2 *
			(TRIGFAKTOR + sinwert / 2 - coswert / 8) / TRIGFAKTOR;
		ecken[1].y = fenster->hoehe / 2 *
			(TRIGFAKTOR + coswert / 2 + sinwert / 8) / TRIGFAKTOR;

		ecken[2].x = fenster->breite / 2 *
			(TRIGFAKTOR + sinwert / 4) / TRIGFAKTOR;
		ecken[2].y = fenster->hoehe / 2 *
			(TRIGFAKTOR + coswert / 4) / TRIGFAKTOR;

		ecken[3].x = fenster->breite / 2 *
			(TRIGFAKTOR + sinwert / 2 + coswert / 8) / TRIGFAKTOR;
		ecken[3].y = fenster->hoehe / 2 *
			(TRIGFAKTOR + coswert / 2 - sinwert / 8) / TRIGFAKTOR;

		/* Polygon fuellen */
		XFillPolygon(fenster->display, zeichenfeld,
			farben[KOMPASSZEIGERFARBE], ecken, 4, Nonconvex,
			CoordModeOrigin);
		/* Polygon umranden */
		XDrawLines(fenster->display, zeichenfeld, farben[RANDFARBE],
			ecken, 5, CoordModeOrigin);
		/* Mittelpunkt zeichnen */
		XDrawPoint(fenster->display, zeichenfeld, farben[RANDFARBE],
			fenster->breite / 2, fenster->hoehe / 2);
	}


	/* Zeichenfeld in Fenster kopieren */
	XCopyArea(fenster->display, zeichenfeld, fenster->window,
		farben[RANDFARBE], 0, 0, fenster->breite, fenster->hoehe, 0, 0);
}


/*
** X_zeichne_punkte
**  zeichnet den Punktestand
**
** Parameter:
**  X_daten: Zeiger auf die private Datenstruktur
**  fenster: Parameter des Fensters, in dem gezeichnet wird
**  daten: Zeiger auf eine Struktur mit der Liste der Punktestaende
**         und Spielerfarben und deren Anzahl
*/
void X_zeichne_punkte(void *X_daten, struct fenster *fenster, void *daten)
{
	struct blickfeld *blickfeld; /* private Daten */
	struct punktedaten *punkte;  /* der Punktestand */
	Pixmap zeichenfeld;          /* Zeichenfeld, in das im Hintergrund
	                                gezeichnet wird */
	int i;                       /* Index fuer die Spieler */

	blickfeld = X_daten;
	punkte = daten;
	zeichenfeld = blickfeld->zeichenfeld;

	/* Zeichenfeld loeschen */
	XFillRectangle(fenster->display, zeichenfeld, farben[BODENFARBE],
		0, 0, fenster->breite, fenster->hoehe);

	for (i = 0; i < punkte->anzpunkte; i++)
	{
		int hervorheben;

		hervorheben = punkte->punkte[i].hervorheben;

		if (hervorheben)
			XFillRectangle(fenster->display, zeichenfeld,
				farben[SCHATTENFARBE], 3, 3 + 30 * i, 75, 25);

		/* Gesicht des Spielers zeichnen */
		malkugel(fenster, zeichenfeld, NULL, -1,
			5, 5 + 30 * i, 20, 20, TRIGANZ / 2,
			punkte->punkte[i].farbe);

		/* Punkte des Spielers zeichnen */
		if (punkte->punkte[i].punkte >= 0)
			malzahl(fenster, zeichenfeld, RANDFARBE,
				hervorheben ? SCHATTENFARBE : BODENFARBE,
				35, 5 + 30 * i, 10, 10, 4,
				punkte->punkte[i].punkte);
	}

	/* Zeichenfeld in Fenster kopieren */
	XCopyArea(fenster->display, zeichenfeld, fenster->window,
		farben[BODENFARBE], 0, 0, fenster->breite, fenster->hoehe, 0, 0);
}


/*
** X_farben_init
**  initialisiert die globale Farbpalette
**
** Parameter:
**  display: Display, auf dem sich die Fenster befinden
**  screen: Screen, auf dem sich die Fenster befinden
**  new_colormap: Zeiger auf Colormap, die verwendet wurde;
**                NULL falls nur die Default-Map benutzt werden darf
**
** Rueckgabewert:
**  0 bei ordentlicher Initialisierung, 1 bei Fehler
**
** Seiteneffekte:
**  initialisiert die globalen Variablen farben und schwarzweiss;
**  new_colormap wird bei Erfolg gesetzt
*/
int X_farben_init(Display *display, int screen, Colormap *new_colormap)
{
	int i;                       /* Schleifenindex fuer die Farben */
	Window rootwindow;           /* Rootwindow auf diesem Display/Screen */
	Colormap colormap;           /* globale Farbpalette */
	XColor farbwerte[FARBENANZ]; /* verschiedene RGB-Farbwerte */
	char *farbnamen[FARBENANZ];  /* Namen fuer Farbwerte */
	int farbindizes[FARBENANZ];  /* Zuordnung defaultfarben <-> farbwerte */
	int farbenanz;               /* Anzahl der verschiedenen Farbwerte */
	XGCValues gc_init;           /* Parameter fuer Erzeugung von GCs */

	colormap = DefaultColormap(display, screen);

	/* auf diesem Display/Screen nur schwarz/weiss moeglich? */
	schwarzweiss = DefaultDepth(display, screen) <= 1;

	rootwindow = RootWindow(display, screen);

	/* kein GraphicsExpose/NoExpose erzeugen */
	gc_init.graphics_exposures = False;

	/* falls Farben moeglich, erst farbig probieren */
	if (!schwarzweiss)
	{
		/* alle RGB-Farbwerte aus den Namen bestimmen und Zuordnung in
		   farbindizes festhalten */
		for (i = farbenanz = 0; i < FARBENANZ; i++)
		{
			int j; /* Index, an dem eine Farbe zuerst auftritt */

			/* Farbwert bestimmen, falls Name unbekannt, Fehler melden */
			if (!XParseColor(display, colormap, defaultfarben[i],
				&farbwerte[farbenanz]))
			{
				static char *meldung[] = { "iMaze - Error", "",
					"Unknown color:", NULL, NULL }; /* Fehlermeldung */

				meldung[3] = defaultfarben[i];

				/* Benutzer fragen, ob schwarz/weiss benutzt werden soll */
				if (!rueckfrage(meldung, "Continue BW", NULL))
					/* Programm wieder beenden */
					return 1;
				else
				{
					/* schwarz/weiss benutzen */
					schwarzweiss = 1;
					break;
				}
			}

			/* trat der Farbwert schon einmal auf? */
			for (j = 0; j < farbenanz; j++)
				if (farbwerte[j].red == farbwerte[farbenanz].red &&
					farbwerte[j].green == farbwerte[farbenanz].green &&
					farbwerte[j].blue == farbwerte[farbenanz].blue)
					break;

			/* Zuordnung festhalten */
			farbindizes[i] = j;
			if (j == farbenanz)
			{
				farbnamen[farbenanz] = defaultfarben[i];
				farbenanz++;
			}
		}
	}

	if (!schwarzweiss)
	{
		int bereits_kopiert; /* wurde die Colormap bereits kopiert? */

		bereits_kopiert = 0;

		for (i = 0; i < farbenanz; i++)
		{
			int erfolg;

			/* Farbe belegen, falls moeglich */
			erfolg = XAllocColor(display, colormap, &farbwerte[i]);

			/* falls nicht moeglich, mit eigener
			   Colormap probieren (sofern erlaubt) */
			if (!erfolg && !bereits_kopiert &&
				new_colormap != NULL)
			{
				colormap = XCopyColormapAndFree(display,
					colormap);
				bereits_kopiert = 1;

				/* nochmal probieren */
				erfolg = XAllocColor(display, colormap,
					&farbwerte[i]);
			}

			/* falls immer noch erfolglos, Fehler melden */
			if (!erfolg)
			{
				static char *meldung[] = { "iMaze - Error", "",
					NULL, NULL, NULL }; /* Fehlermeldung */
				char zeile2[40];

				sprintf(zeile2, "Can't allocate color %d out of %d:",
					i + 1, farbenanz);
				meldung[2] = zeile2;
				meldung[3] = farbnamen[i];

				/* Benutzer fragen, ob schwarz/weiss benutzt werden soll */
				if (!rueckfrage(meldung, "Continue BW", NULL))
					/* Programm wieder beenden */
					return 1;
				else
				{
					/* schwarz/weiss benutzen */
					schwarzweiss = 1;

					/* alle i Farben wieder freigeben */
					if (i)
					{
						/* X11-Pixelwerte: */
						unsigned long farbpixel[FARBENANZ];
						int j; /* Index */

						for (j = 0; j < i; j++)
							farbpixel[j] = farbwerte[j].pixel;

						XFreeColors(display, colormap, farbpixel, i, 0);
					}

					break;
				}
			}
		}
	}

	if (!schwarzweiss)
	{
		GC gcs[FARBENANZ]; /* alle GCs einzeln */

		for (i = 0; i < farbenanz; i++)
		{
			/* GC fuer diese Farbe erzeugen */
			gcs[i] = XCreateGC(display, rootwindow,
				GCGraphicsExposures, &gc_init);
			XSetForeground(display, gcs[i], farbwerte[i].pixel);
		}

		/* GCs zuordnen */
		for (i = 0; i < FARBENANZ; i++)
			farben[i] = gcs[farbindizes[i]];
	}

	/* schwarz/weiss-Raster erzeugen */
	if (schwarzweiss)
	{
		/* optimale Groesse (Anzahl der Wiederholungen)
		   fuer Stipple (fuer schwarz/weiss-Raster): */
		unsigned breite, hoehe;

		/* optimale Stipple-Groesse in der Naehe von 8x8 feststellen,
		   fuer schwarz/weiss-Raster */
		XQueryBestStipple(display, rootwindow, 8, 8, &breite, &hoehe);

		/* Vergroesserungsfaktoren fuer Breite und Hoehe der
		   schwarz/weiss-Raster berechnen, mindestens 1 */
		if ((breite = (breite + 4) / 8) < 1)
			breite = 1;
		if ((hoehe = (hoehe + 4) / 8) < 1)
			hoehe = 1;

		for (i = 0; i < FARBENANZ; i++)
		{
			GC gc; /* GC fuer aktuelles Raster */
			int j; /* Index, an dem ein Raster zuerst auftritt */

			/* suchen, ob das Raster schon mit kleinerem Index existiert */
			for (j = 0; j < i && sw_defaultfarben[i] !=
				sw_defaultfarben[j]; j++);

			/* Raster existiert schon */
			if (j < i)
			{
				farben[i] = farben[j];
				continue;
			}

			/* GC fuer aktuelles Raster erzeugen */
			gc = XCreateGC(display, rootwindow,
				GCGraphicsExposures, &gc_init);

			switch (sw_defaultfarben[i])
			{
				case SCHWARZ: /* einfarbig schwarz */
					XSetForeground(display, gc,
						BlackPixel(display, screen));
					break;

				case WEISS: /* einfarbig weiss */
					XSetForeground(display, gc,
						WhitePixel(display, screen));
					break;

				default: /* schwarz/weiss-Raster */
					XSetForeground(display, gc,
						BlackPixel(display, screen));
					XSetBackground(display, gc,
						WhitePixel(display, screen));

					/* Raster skalieren */
					XSetStipple(display, gc,
						bitmap_skalieren(display, screen,
							(char *)sw_raster[sw_defaultfarben[i] - RASTER],
							1, 8, breite, hoehe));

					/* FillStyle auf Raster setzen */
					XSetFillStyle(display, gc, FillOpaqueStippled);
			}

			farben[i] = gc;
		}
	}

	{
		static char *namen[] = { "10x20", "fixed", NULL };

		text_farbe = font_laden(display, screen, namen, &text_font);
	}

	{
		static char *namen[] =
		{
			"-*-lucida-medium-r-normal-sans-17-*-iso8859-1",
			"-*-lucida-*-r-*-*-17-*-*-*-*-*-iso8859-1",
			"-*-helvetica-medium-r-normal--17-*-iso8859-1",
			"-*-helvetica-*-r-*-*-17-*-*-*-*-*-iso8859-1",
			"fixed",
			NULL
		};

		titelfarbe = font_laden(display, screen, namen, &titelfont);

		gc_init.foreground = BlackPixel(display, screen);
		gc_init.fill_style = FillStippled;
		gc_init.stipple = XCreateBitmapFromData(display, rootwindow,
			(void *)iMaze_1_4_bits, iMaze_1_4_width,
			iMaze_1_4_height);
		gc_init.graphics_exposures = False;
		schriftzug_farbe = XCreateGC(display, rootwindow,
			GCForeground | GCFillStyle | GCStipple |
				GCGraphicsExposures,
			&gc_init);
	}

	/* kein Fehler aufgetreten */
	if (new_colormap != NULL)
		*new_colormap = colormap;
	return 0;
}


/*
** X_zeichne_sync_anfang
**  synchronisiert den Anfang des Zeichnens mit dem X-server
**
** Parameter:
**  display: Display, auf dem sich die Fenster befinden
*/
void X_zeichne_sync_anfang(Display *display)
{
	/* warten, bis die vorige Grafik fertig aufgebaut ist */
	XSync(display, 0);
}


/*
** X_zeichne_sync_ende
**  synchronisiert das Ende des Zeichnens mit dem X-server
**
** Parameter:
**  display: Display, auf dem sich die Fenster befinden
*/
void X_zeichne_sync_ende(Display *display)
{
	/* alle Grafik-Kommandos, die noch gepuffert sind, an
	   den Server schicken */
	XFlush(display);
}


/*
** X_fenster_init
**  initialisert die private Datenstruktur fuer das 3D-Blickfeld
**  bzw. den Kompass
**
** Parameter:
**  X_daten: Adresse des Zeigers auf die private Datenstruktur
**  fenster: Zeiger auf die Datenstruktur, die das zugehoerige
**           Fenster beschreibt
**
** Seiteneffekte:
**  *X_daten wird gesetzt
*/
void X_fenster_init(void **X_daten, struct fenster *fenster)
{
	speicher_belegen(X_daten, sizeof(struct blickfeld));

	/* Pixmap fuer Zeichnen im Hintergrund erzeugen */
	(*(struct blickfeld **)X_daten)->zeichenfeld =
		XCreatePixmap(fenster->display, fenster->window, fenster->breite,
			fenster->hoehe, fenster->farbtiefe);
}


/*
** X_fenster_freigeben
**  gibt die private Datenstruktur fuer das 3D-Blickfeld bzw. den
**  Kompass frei
**
** Parameter:
**  X_daten: Adresse des Zeigers auf die private Datenstruktur
**  fenster: Zeiger auf die Datenstruktur, die das zugehoerige
**           Fenster beschreibt
*/
void X_fenster_freigeben(void **X_daten, struct fenster *fenster)
{
	/* Pixmap fuer Zeichnen im Hintergrund freigeben */
	XFreePixmap(fenster->display,
		(*(struct blickfeld **)X_daten)->zeichenfeld);

	speicher_freigeben(X_daten);
}


/*
** X_karte_init
**  initialisert die private Datenstruktur fuer die Karte
**
** Parameter:
**  X_daten: Adresse des Zeigers auf die private Datenstruktur
**  fenster: Zeiger auf die Datenstruktur, die das zugehoerige
**           Fenster beschreibt
**
** Seiteneffekte:
**  *X_daten wird gesetzt
*/
void X_karte_init(void **X_daten, struct fenster *fenster)
{
	speicher_belegen(X_daten, sizeof(struct karte));

	/* Pixmaps fuer Zeichnen im Hintergrund erzeugen */
	(*(struct karte **)X_daten)->zeichenfeld1 =
		XCreatePixmap(fenster->display, fenster->window, fenster->breite,
			fenster->hoehe, fenster->farbtiefe);
	(*(struct karte **)X_daten)->zeichenfeld2 =
		XCreatePixmap(fenster->display, fenster->window, fenster->breite,
			fenster->hoehe, fenster->farbtiefe);
}


/*
** X_karte_freigeben
**  gibt die private Datenstruktur fuer die Karte frei
**
** Parameter:
**  X_daten: Adresse des Zeigers auf die private Datenstruktur
**  fenster: Zeiger auf die Datenstruktur, die das zugehoerige
**           Fenster beschreibt
*/
void X_karte_freigeben(void **X_daten, struct fenster *fenster)
{
	/* Pixmaps fuer Zeichnen im Hintergrund freigeben */
	XFreePixmap(fenster->display,
		(*(struct karte **)X_daten)->zeichenfeld1);
	XFreePixmap(fenster->display,
		(*(struct karte **)X_daten)->zeichenfeld2);

	speicher_freigeben(X_daten);
}
