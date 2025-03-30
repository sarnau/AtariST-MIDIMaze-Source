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
** Datei: ereignisse.h
**
** Kommentar:
**  Definiert moegliche Ausgabe-Signale (Ereignisse, die mit
**  Sounds/Grafikeffekten o.ae. versehen werden koennen)
*/


static char sccsid_ereignisse[] = "@(#)ereignisse.h	3.2 12/3/01";


#define AKTION_FEHLER_ERG 0  /* Kaufaktion o.ae. nicht moeglich */
#define AKTION_OK_ERG     1  /* Kaufaktion o.ae. war erfolgreich */
#define SCHUSS_LOS_ERG    2  /* Spieler hat Schuss abgefeuert */
#define SCHUSS_WEG_ERG    3  /* Schuss hat Wand getroffen und ist weg */
#define GETROFFEN_ERG     4  /* Spieler wurde getroffen */
#define ABGESCHOSSEN_ERG  5  /* Spieler wurde getoetet */
#define TREFFER_ERG       6  /* Spieler hat Gegner getroffen */
#define ABSCHUSS_ERG      7  /* Spieler hat Gegner getoetet */
#define PAUSE_ERG         8  /* Spieler legt eine Pause ein */
#define SUSPENDIERT_ERG   9  /* Spieler wurde aus dem Spiel genommen */
#define ERWACHT_ERG       10 /* Spieler ist wieder im Spiel */
#define SCHUSS_PRALL_ERG  11 /* Schuss ist an der Wand abgeprallt */

#define ERG_ANZ           12 /* Anzahl der Ereignisse */
