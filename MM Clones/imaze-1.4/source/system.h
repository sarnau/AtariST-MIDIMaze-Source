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
** Datei: system.h
**
** Kommentar:
**  Definition der Prototypen fuer den betriebssystemspezifischen Teil
*/


static char sccsid_system[] = "@(#)system.h	3.5 12/3/01";


/* Prototypen fuer betriebssystemspezifische Funktionen */

void (*handle_signal(int signum, void (*handler)(int signum)))(int signum);
int signal_blockiert(int signum);
void signal_blockieren(int signum);
void signal_freigeben(int signum);
int max_deskriptor_anzahl(void);
int deskriptor_nicht_blockieren(int deskriptor);
int puffer_lesen(int deskriptor, int puffer_laenge, void *puffer);
int puffer_schreiben(int deskriptor, int puffer_laenge, void *puffer);
void timer_starten(int ms);
void timer_stoppen(void);
int timer_restzeit(void);
void timer_abwarten(void);
void *timer_retten(void);
void timer_restaurieren(void *zustand);
char *uhrzeit(void);
char *benutzer_name(void);
char *fehler_text(void);
