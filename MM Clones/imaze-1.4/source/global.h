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
** Datei: global.h
**
** Kommentar:
**  globale Typ- und Funktion-Definitionen
*/


#include <sys/types.h> /* fuer u_int... */

static char sccsid_global[] = "@(#)global.h	3.3 12/3/01";


/* zusaetzliche Grundfunktionen */
#define min(x,y) ((x) < (y) ? (x) : (y))
#define max(x,y) ((x) > (y) ? (x) : (y))
#define sqr(x) ((x) * (x))

#define s_char signed char

/* Ausmasse der globalen Sinus- und Cosinus-Tabelle;
   TRIGANZ muss 2er-Potenz sein */
#ifndef TRIGANZ
#define TRIGANZ 1024
#endif

#define TRIGFAKTOR 65536

/* Codes fuer den Parameter runden bei wurzel */
#define W_ABRUNDEN -1
#define W_RUNDEN    0
#define W_AUFRUNDEN 1


/* Cosinus/Sinus in Schritten von 360/TRIGANZ Grad;
   gespeichert als Integerwerte: sin*TRIGFAKTOR */
extern int costab[TRIGANZ], sintab[TRIGANZ];

/* Arcustangens von -1 bis 1 in Schritten von 2/TRIGANZ;
   gespeichert als Indizes fuer costab/sintab */
extern int atantab[TRIGANZ + 1];


/* Prototypen fuer globale portable Funktionen */

long zufall(void);
void zufall_init(unsigned long startwert);
long wurzel(unsigned long x, int runden);
