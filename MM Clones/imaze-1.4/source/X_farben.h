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
** Datei: X_farben.h
**
** benoetigt:
**  farben.h
**
** Kommentar:
**  Definiert Indizes/Offsets aller Farben fuer die X11-Farbtabelle
*/


static char sccsid_X_farben[] = "@(#)X_farben.h	3.1 12/3/01";


#define RANDFARBE          0
#define HIMMELFARBE        1
#define BODENFARBE         2
#define SCHATTENFARBE      3
#define KUGELRANDFARBE     4
#define KOMPASSFARBE1      5
#define KOMPASSFARBE2      6
#define KOMPASSZEIGERFARBE 7

#define WANDFARBEN         8 /* Offset */
#define KUGELFARBEN        (WANDFARBEN + WANDFARBENANZ) /* Offset */
#define KUGELGFARBEN       (KUGELFARBEN + KUGELFARBENANZ) /* Offset */
#define FARBENANZ          (KUGELGFARBEN + KUGELFARBENANZ) + 1

/* Farben fuer Schwarz/Weiss und div. Rasterstufen*/
#define SCHWARZ 0
#define WEISS   1
#define RASTER  2 /* Offset */


extern char *defaultfarben[FARBENANZ];  /* Namen der X11-Farben */
extern int sw_defaultfarben[FARBENANZ]; /* Indizes auf sw_raster */
extern unsigned char sw_raster[][8];    /* Grauraster-Beschreibung fuer X11 */
