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
** Datei: speicher.c
**
** Kommentar:
**  Stellt Funktionen zum Anfordern und Freigeben von Speicher zur
**  Verfuegung
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "global.h"
#include "fehler.h"
#include "speicher.h"

static char sccsid[] = "@(#)speicher.c	3.3 12/3/01";


/*
** keinspeicher
**  wird aufgerufen, falls malloc/realloc fehlschlaegt
**
** Seiteneffekte:
**  Abbruch des Programms
*/
static void keinspeicher(void)
{
	static char *meldung[] = { "iMaze - Fatal Error", "",
		"Out of memory.", NULL };

	uebler_fehler(meldung, "I'll buy some RAM");
}


/* bis hier lokaler Teil                       */
/***********************************************/
/* ab hier globaler Teil                       */


/*
** speicher_belegen
**  fordert Speicher mit malloc an
**
** Parameter:
**  zeiger: Zeiger auf Zeiger auf neuen Speicherbereich
**  groesse: Groesse des Speicherbereichs
**
** Seiteneffekte:
**  *zeiger wird gesetzt
*/
void speicher_belegen(void **zeiger, int groesse)
{
	if ((*zeiger = malloc(groesse)) == NULL)
		keinspeicher();
}


/*
** speicher_vergroessern
**  fordert weiteren Speicher mit realloc an
**
** Parameter:
**  zeiger: Zeiger auf Zeiger auf Speicherbereich
**  groesse: neue Groesse des Speicherbereichs
**
** Seiteneffekte:
**  *zeiger wird veraendert
*/
void speicher_vergroessern(void **zeiger, int groesse)
{
	if ((*zeiger = realloc(*zeiger, groesse)) == NULL)
		keinspeicher();
}


/*
** speicher_freigeben
**  gibt Speicherbereich frei
**
** Parameter:
**  zeiger: Zeiger auf Zeiger auf alten Speicherbereich
*/
void speicher_freigeben(void **zeiger)
{
	free(*zeiger);
}


/*
** liste_initialisieren
**  setzt Parameter einer Liste auf definierte Anfangswerte
**
** Parameter:
**  anzahl: Zeiger auf Anzahl der Listenelemente
**  liste: Zeiger auf Liste
**
** Seiteneffekte:
**  *anzahl wird gesetzt
*/
void liste_initialisieren(int *anzahl, void **liste)
{
	*anzahl = 0;
}


/*
** liste_verlaengern
**  verlaengert Liste um ein Element
**
** Parameter:
**  anzahl: Zeiger auf Anzahl der Listenelemente
**  liste: Zeiger auf Liste
**  element_groesse: Groesse eines Elementes
**
** Seiteneffekte:
**  *anzahl und *liste werden veraendert
*/
void liste_verlaengern(int *anzahl, void **liste, int element_groesse)
{
	if ((*anzahl)++)
		speicher_vergroessern(liste, *anzahl * element_groesse);
	else
		speicher_belegen(liste, element_groesse);
}


/*
** liste_kopieren
**  legt eine Kopie einer Liste mit memcpy an
**
** Parameter:
**  anzahl_neu: Zeiger auf Anzahl der Listenelemente (Kopie)
**  liste_neu: Zeiger auf Kopie der Liste
**  anzahl: Zeiger auf Anzahl der Listenelemente (Original)
**  liste: Zeiger auf Liste
**  element_groesse: Groesse eines Elementes
**
** Seiteneffekte:
**  *anzahl_neu und *liste_neu werden gesetzt
*/
void liste_kopieren(int *anzahl_neu, void **liste_neu, int *anzahl,
	void **liste, int element_groesse)
{
	/* nur kopieren, wenn *anzahl !=0 */
	if (*anzahl_neu = *anzahl)
	{
		speicher_belegen(liste_neu, *anzahl_neu * element_groesse);
		memcpy(*liste_neu, *liste, *anzahl_neu * element_groesse);
	}
}


/*
** liste_freigeben
**  gibt den Speicherbereich der Liste frei
**
** Parameter:
**  anzahl: Zeiger auf alte Anzahl der Listenelemente
**  liste: Zeiger auf Liste
*/
void liste_freigeben(int *anzahl, void **liste)
{
	if (*anzahl)
		speicher_freigeben(liste);
}
