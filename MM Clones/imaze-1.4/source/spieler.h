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
** Datei: spieler.h
**
** benoetigt:
**  global.h
**
** Kommentar:
**  Datenstruktur zur Speicherung der Spieler und Schuesse sowie Funktionen
**  zur Umrechnung deren Position
*/


static char sccsid_spieler[] = "@(#)spieler.h	3.1 12/3/01";


#define SPIELERANZ 42

#define WINKANZ 64  /* Anzahl der Drehwinkel fuer einen Spieler */

#define KUGELRAD (RASPT * 3 / 16)  /* Radius eines Spielers (relativ zur
        Gangbreite); muss <= RASPT * (sqrt(2)-1)/2 sein */

#define SCHUSSRAD (RASPT * 3 / 64) /* Radius eines Schusses (relativ zur
        Gangbreite) */


/* Rechnet Labyrinth-Koordinaten von grob/fein-Darstellung auf absolute
   Werte im feinen Raster um */
#define XPOS(pos) ((int)(pos).xgrob * RASPT + (int)(pos).xfein)
#define YPOS(pos) ((int)(pos).ygrob * RASPT + (int)(pos).yfein)

/* Rechnet Labyrinth-Koordinaten von absoluten Werten im feinen Raster
   auf grob/fein-Darstellung um */
#define SETX(pos,x) ((pos).xgrob = (x) / RASPT, (pos).xfein = (x) % RASPT)
#define SETY(pos,y) ((pos).ygrob = (y) / RASPT, (pos).yfein = (y) % RASPT)

/* Addiert Wert in feinem Raster zu Labyrinth-Koordinaten in grob/fein-
   Darstellung */
#define ADDX(pos,x) (SETX((pos),XPOS(pos)+(x)))
#define ADDY(pos,y) (SETY((pos),YPOS(pos)+(y)))

/* Subtahiert Wert in feinem Raster von Labyrinth-Koordinaten in grob/fein-
   Darstellung */
#define SUBX(pos,x) (ADDX((pos),-(x)))
#define SUBY(pos,y) (ADDY((pos),-(y)))


/* Position aufgeteilt in grobes (Labyrinth-Bloecke) und feines Raster */
struct position
{
	u_char xgrob;
	u_char xfein;
	u_char ygrob;
	u_char yfein;
};

/* Informationen ueber Spieler fuer Eingabe von rechne3d */
struct spieler
{
	struct position pos;
	s_char blick;         /* Blickrichtung in 0..WINKANZ */
	u_char farbe;
};

/* Informationen ueber Schuesse fuer Eingabe von rechne3d */
struct schuss
{
	struct position pos;
	u_char farbe;
};
