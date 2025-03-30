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
** Datei: grafik.h
**
** benoetigt:
**  global.h
**
** Kommentar:
**  Definition der Datentypen und Prototypen fuer den Grafikteil
*/


static char sccsid_grafik[] = "@(#)grafik.h	3.4 12/3/01";


#ifndef SICHTW
#define SICHTW 16    /* Sichtweite des Spielers in Bloecken, maximal 64 */
#endif

#define WANDH RASPT  /* Hoehe der Waende (relativ zur Gangbreite) */

#define FENSTER_BREITE 65536  /* virtuelle Fensterbreite in Bildpunkten*/
#define FENSTER_HOEHE 65536   /* virtuelle Fensterhoehe in Bildpunkten*/

#define KEINELINIE 0 /* Flags fuer struct kartenlinie */
#define WANDLINIE  1
#define TUERLINIE  2


struct punkt
{
	int x, y;
};

/* enthaelt Koordinaten in Bildpunkten zum Zeichnen der Waende
   auf der Karte; wird in rechne_karte berechnet */
struct kartenlinie
{
	struct punkt ecke; /* linkes/oberes Ende der Wand */
	int laenge;        /* Laenge der Wand */
	u_char senkrecht;  /* falls Wand von oben nach unten */
	u_char typ1;       /* Typ der Wand von links/oben */
	u_char typ2;       /* Typ der Wand von rechts/unten */
};

/* enthaelt Koordinaten in Bildpunkten zum Zeichen der Spieler
   auf der Karte; wird in rechne_karte berechnet */
struct kartenkreis
{
	struct punkt mittelpunkt;
	int radiusx, radiusy;
	u_char farbe;
};

/* enthaelt Koordinaten in Bildpunkten zum Zeichnen der Waende;
   wird in rechne3d berechnet */
struct flaeche
{
	/* ecke[0] links oben, ecke[1] links unten,
	   ecke[2] rechts unten, ecke[3] rechts oben */
	struct punkt ecke[4];
	u_char farbe; /* darf nicht TRANSPARENT sein */
	u_char tuer;  /* falls Wand eine Tuer ist */
};

struct ausschnitt
{
	int x, breite;
};

/* enthaelt Koordinaten in Bildpunkten und Grafikinformationen zum Zeichnen
   von Spielern und Schuessen; wird in rechne3d berechnet */
struct kugel
{
	struct punkt mittelpunkt;
	struct ausschnitt *sichtbar; /* Clipping-Bereiche */
	int sichtanz;                /* Anzahl der Clipping-Bereiche */
	int radiusx, radiusy;
	/* schattenx wie mittelpunkt.x/radiusx */
	int schatteny, schattenry;   /* schattenry 0, falls kein Schatten */
	int blick;                   /* ist -1 falls kein Gesicht */
	u_char farbe;                /* kann TRANSPARENT sein */
};

/* enthaelt gesammelte Daten fuer 3D-Zeichnung und Spruch des Gegners */
struct objektdaten
{
	int hintergrund_zeichnen; /* Himmel, Boden, Horizont zeichnen */
	int anzwaende;
	struct flaeche *waende;
	int anzkugeln;
	struct kugel *kugeln;
	char *text; /* Spruch des Gegners oder NULL */
};

/* enthaelt Punkte und Farbe eines Spielers */
struct punktestand
{
	int punkte; /* -1 = nicht die Punkte, nur den Spieler zeichnen */
	u_char farbe;
	int hervorheben;
};

/* enthaelt gesamte Punktestanddaten fuer alle Spieler */
struct punktedaten
{
	int anzpunkte;
	struct punktestand *punkte;
};


/* Prototypen fuer globale Grafik-Funktionen */

void zeichne_blickfeld(struct objektdaten *objekte_neu);
void zeichne_rueckspiegel(struct objektdaten *objekte_rueck_neu);
void zeichne_karte(int spieler_index, int anzkreise_neu,
	struct kartenkreis *kreise_neu);
void zeichne_grundriss(int anzlinien_neu, struct kartenlinie *linien_neu);
void zeichne_kompass(int blickrichtung_neu);
void zeichne_punkte(struct punktedaten *punkte_neu);
void zeichne_sync_anfang(void);
void zeichne_sync_ende(void);
void grafik_schleife(void);
int grafik_init(int *argc, char **argv);
void grafik_ende(void);
void disconnect_abfrage(void);
void ueblen_fehler_anzeigen(char **meldung, char *knopf);
int rueckfrage(char **meldung, char *weiter_knopf, char *abbruch_knopf);
