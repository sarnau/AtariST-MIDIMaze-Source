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
** Datei: protokoll.h
**
** Kommentar:
**  Definition des Protokollformats
*/


static char sccsid_protokoll[] = "@(#)protokoll.h	3.3 12/3/01";


/* Werte, die das ignoriert-Bit annehmen kann */
#define PAKET_NICHT_IGNORIERT 0
#define PAKET_IGNORIERT       1

/* das oberste Bit ist gesetzt, falls das letzte Paket ignoriert wurde */
#define PROL_PAKET_IGNORIERT(kennung) (((kennung) & 128) != 0)

/* die anderen Bits enthalten den Typ des Paketes selbst */
#define PROL_PAKET_TYP(kennung) ((kennung) & 127)

#define PROL_PAKET_KENNUNG(ignoriert, typ) ((ignoriert) << 7 | (typ))


/* Prolog-Pakete in beide Richtungen */
#define PT_LEER          0
#define PT_ABBRUCH       1
#define PT_PROGRAMM_NAME 2


/* Prolog-Pakete in Richtung Client -> Server */
#define PT_BEREIT_FUER_SPIEL 64
#define PT_WILL_LABYRINTH    65
#define PT_SPIELER_NAME      66
#define PT_SPIELER_SPRUCH    67
#define PT_KAMERA            68 /* nur zusehen */
#define PT_WILL_STATUS       69


/* Prolog-Pakete in Richtung Server -> Client */
#define PT_SPIEL_BEGINNT 96
#define PT_LABYRINTH     97
#define PT_STATUS        98


/* bei Blockkennungen 0-127 ist die Datenlaenge 0, bei 128-255 variabel */
#define DATENLAENGE_VARIABEL(block_kennung) (((block_kennung) & 128) != 0)


/* Spiel-Pakete in Richtung Client -> Server */

/* Datenlaenge 0 */
#define BK_ENDE_SIGNAL 0

/* Datenlaenge variabel */
#define BK_SIGNALE 128 /* siehe signale.h */


/* Spiel-Pakete in Richtung Server -> Client */

/* Datenlaenge 0 */
#define BK_ENDE_DURCH_SERVER 0
#define BK_ENDE_DURCH_CLIENT 1

/* Datenlaenge variabel */
#define BK_SPIELER_POS        128
#define BK_GEGNER_POS         129
#define BK_SPIELER_SCHUSS_POS 130
#define BK_GEGNER_SCHUSS_POS  131
#define BK_ABGESCHOSSEN_DURCH 132
#define BK_EREIGNISSE         133 /* siehe ereignisse.h */
#define BK_ALTE_EREIGNISSE    134 /* noch nicht bestaetigte Ereignisse */
#define BK_PUNKTESTAND        135
