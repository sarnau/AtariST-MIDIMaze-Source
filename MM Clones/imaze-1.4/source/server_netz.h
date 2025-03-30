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
** Datei: server_netz.h
**
** benoetigt:
**  argv.h
**
** Kommentar:
**  Definition der Prototypen fuer den Netzwerkteil im Server
*/


static char sccsid_server_netz[] = "@(#)server_netz.h	3.6 12/3/01";


extern struct arg_option net_opts[];


/* Prototypen fuer globale Netzwerk-Funktionen */

int prolog_paket_senden(void *verbindung, int paket_laenge, void *paket);
int prolog_paket_empfangen(void *verbindung, int *paket_laenge,
	void **paket);
void spiel_paket_senden(void *verbindung, int paket_laenge, void *paket);
int spiel_paket_empfangen(int verbindungen_anz, void **verbindungen,
	int *paket_laenge, void **paket);
void verbindung_freigeben(void *verbindung);
void *verbindung_annehmen(void);
void verbindung_abbrechen(void *verbindung, int fehler);
int verbindung_auf_spiel(void *session, void *verbindung);
void netzwerk_init(void);
void *session_starten(void);
void nachricht_senden(void *session, void *verbindung, int daten_laenge,
	void *daten);
int nachricht_empfangen(void *session, void **verbindung, int *daten_laenge,
	void **daten);
void nachricht_erwarten(void *session);
void *nachricht_oder_verbindung_erwarten(int anzsessions, void **sessions);
char *name_der_verbindung(void *verbindung);
