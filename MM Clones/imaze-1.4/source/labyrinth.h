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
** Datei: labyrinth.h
**
** benoetigt:
**  global.h
**
** Kommentar:
**  Definiert Ausmasse und Datenstruktur des Labyrinths
*/


static char sccsid_labyrinth[] = "@(#)labyrinth.h	3.1 12/3/01";


/* Das Labyrinth besteht aus maximal MAXFELDBREITE^2 Bloecken; fuer
   jeden Block sind 4 Waende gespeichert. (die meisten Waende sind
   dadurch doppelt gespeichert, das ermoeglicht "Einbahnstrassen")
   Ein Block ist in RASPT^2 feine Abstufungen unterteilt */


#define NORD 0
#define WEST 1
#define SUED 2
#define OST 3

#define MAXFELDBREITE 127
#define RASPT 256


/* oberstes Farbbit gibt an, ob Tuer oder Wand */
#define IST_TUER(farbe) (((farbe) & 8) != 0)


/* Welche Informationen sind fuer eine einzelne Wand gespeichert? */
struct wand
{
	u_int unbegehbar:1;   /* fuer unsichtbare Waende */
	u_int schusssicher:1; /* Wand nicht durchschiessbar */
	u_int tuer:1;         /* Wand ist eine Tuer */
	u_int tueroffen:1;    /* Tuer ist offen (in rechne3d) */
	u_int farbe:4;        /* maximal 16 Farben */
};

typedef struct wand block[4];

/* reale Feldgroesse */
extern u_int feldlaenge, feldbreite;

/* ein Spielfeld + 4* Spielfeld in 90 Grad Winkeln gedreht */
extern block **spielfeld, **feld_himricht[4];
