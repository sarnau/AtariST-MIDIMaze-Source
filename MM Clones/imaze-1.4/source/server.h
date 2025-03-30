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
** Datei: server.h
**
** Kommentar:
**  Definition des Prototypen fuer den Session-Prozess im Server
**  und der Konstanten fuer die Kommunikation zwischen den Prozessen
*/


static char sccsid_server[] = "@(#)server.h	3.7 12/04/01";


extern struct arg_option feature_opts[];


/* Nachrichten fuer Interprozess-Kommunikation: */

/* von server.c an session.c */
#define NT_SPIEL_BEENDEN 0 /* Spiel soll beendet werden */
#define NT_NEUER_SPIELER 1 /* neuer Spieler kommt ins Spiel */
#define NT_NEUE_KAMERA   2 /* neue Kamera (zusehender Spieler) startet */

/* von session.c an server.c */
#define NT_SPIEL_ENDE      16 /* Spiel wurde beendet */
#define NT_SPIELER_ENDE    17 /* Spieler hat das Spiel verlassen */
#define NT_SPIELER_TIMEOUT 18 /* Spieler wurde nach einem Timeout aus dem
                                 Spiel entfernt */
#define NT_STATUS          19 /* aktueller Status, insbesondere alle Spieler */


/* Prototyp fuer Session-Prozess */

void session(int zeittakt);
