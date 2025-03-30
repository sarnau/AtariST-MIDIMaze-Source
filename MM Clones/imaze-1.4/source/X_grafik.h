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
** Datei: X_grafik.h
**
** benoetigt:
**  X11/Xlib.h, global.h, grafik.h
**
** Kommentar:
**  Interne Defines fuer X-Zeichen-routinen
*/


static char sccsid_X_grafik[] = "@(#)X_grafik.h	3.4 12/3/01";


/* Anzahl der Ecken der Augen- und Mundpolygone */
#ifndef AUGENECKENANZ
#define AUGENECKENANZ 32
#endif

#ifndef MUNDECKENANZ
#define MUNDECKENANZ 32
#endif


/* Innen- und Aussenradius des Fadenkreuzes in Fensterhoehe/KREUZ_FAKTOR */
#ifndef KREUZ_INRAD
#define KREUZ_INRAD 4
#endif

#ifndef KREUZ_AUSRAD
#define KREUZ_AUSRAD 10
#endif

#define KREUZ_FAKTOR 256


/* gesammelte Daten zum Zeichnen des Blickfeldes */
struct blickfelddaten
{
	struct objektdaten objekte;
	int fadenkreuz; /* 1, falls Fadenkreuz zu zeichnen */
	int titelbild; /* 1, falls Titelbild zu zeichnen, Rest ignorieren */
};

/* gesammelte Daten zum Zeichnen des Grundrisses */
struct grundrissdaten
{
	int anzlinien;
	struct kartenlinie *linien;
	int anzkreise;
	struct kartenkreis *kreise;
	int anzkreise_alt;              /* nur fuer X_zeichne_karte,
	                                   muss sonst 0 sein */
	struct kartenkreis *kreise_alt; /* nur fuer X_zeichne_karte,
	                                   muss sonst NULL sein */
};

struct fenster
{
	Display *display;
	Window window;
	int breite, hoehe;
	unsigned farbtiefe;
};


/* Prototypen fuer globale Funktionen */

void X_zeichne_blickfeld(void *X_daten, struct fenster *fenster,
	void *daten);
void X_zeichne_karte(void *X_daten, struct fenster *fenster, void *daten);
void X_zeichne_grundriss(void *X_daten, struct fenster *fenster,
	void *daten);
void X_zeichne_kompass(void *X_daten, struct fenster *fenster, void *daten);
void X_zeichne_punkte(void *X_daten, struct fenster *fenster, void *daten);
int X_farben_init(Display *display, int screen, Colormap *new_colormap);
void X_zeichne_sync_anfang(Display *display);
void X_zeichne_sync_ende(Display *display);
void X_fenster_init(void **X_daten, struct fenster *fenster);
void X_fenster_freigeben(void **X_daten, struct fenster *fenster);
void X_karte_init(void **X_daten, struct fenster *fenster);
void X_karte_freigeben(void **X_daten, struct fenster *fenster);
