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
** Datei: spiel.h
**
** benoetigt:
**  global.h, labyrinth.h, signale.h, spieler.h
**
** Kommentar:
**  Definition der Prototypen fuer die Schnittstelle
**  zwischen Netzwerk und Client
*/


static char sccsid_spiel[] = "@(#)spiel.h	3.4 12/3/01";


/* Prototypen fuer die Schnittstellenfunktionen */

int spielparameter_empfangen(char *programmname, char *spielername,
	char *spielerspruch, int kamera, u_int *feldbreite, u_int *feldlaenge,
	block ***spielfeld, char **server_status, int wirklich_starten);
void spiel_verlassen(void);
void spiel_puffer_anlegen(int **spieler_aktiv, struct spieler **spieler,
	int **gegneranz, struct spieler **gegner, int **schuss_aktiv,
	struct schuss **schuss, int **schussanz, struct schuss **schuesse,
	int **abgeschossen_durch, char ***abschuss_spruch, char **ereignisse,
	int **punkte);
void netzwerk_spielende(void);
void signale_abfragen(char signale[SIGNALANZ]);
