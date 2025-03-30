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
** Datei: global.c
**
** Kommentar:
**  Globale portable Routinen;
**  z.Zt. fuer Zufallswerte und Wurzelziehen
*/


#include <limits.h>

#include "global.h"

static char sccsid[] = "@(#)global.c	3.3 12/3/01";


#define ZUSTAND_EINTRAEGE (sizeof zustand / sizeof *zustand)

/* Zustandstabelle fuer additiven Kongruenzengenerator;
   initialisiert wie nach zufall_init(1) */
static unsigned long zustand[] =
{
	0x9a319039, 0x32d9c024, 0x9b663182, 0x5da1f342,
	0xde3b81e0, 0xdf0a6fb5, 0xf103bc02, 0x48f340fb,
	0x7449e56b, 0xbeb1dbb0, 0xab5c5918, 0x946554fd,
	0x8c2e680f, 0xeb3d799f, 0xb11ee0b7, 0x2d436b86,
	0xda672e2a, 0x1588ca88, 0xe369735d, 0x904f35f7,
	0xd7158fd6, 0x6fa6f051, 0x616e6b96, 0xac94efdc,
	0x36413f93, 0xc622c298, 0xf5a42ab8, 0x8a88d77b,
	0xf5ad9d0e, 0x8999220b, 0x27fb47b9
};

/* Rueckkopplungszeiger fuer additiven Kongruenzengenerator;
   initialisiert wie nach zufall_init(1) */
static unsigned long *von_zustand = zustand;      /* Quelle */
static unsigned long *nach_zustand = zustand + 3; /* Ziel */


/* bis hier lokaler Teil                       */
/***********************************************/
/* ab hier globaler Teil                       */


/*
** zufall
**  berechnet aus einer Zustandstabelle einen Zufallswert;
**  kompatibel zu BSD-random
**
** Rueckgabewert:
**  ein Zufallswert
**
** Seiteneffekte:
**  von_zustand und nach_zustand werden zyklisch hochgezaehlt,
**  die Zustandstabelle zustand wird verwuerfelt
*/
long zufall(void)
{
	unsigned long wert; /* Zwischenspeicher fuer Zufallswert */

	/* Rueckkopplung */
	*nach_zustand += *von_zustand;

	/* Zufallswert lesen und merken */
	wert = *nach_zustand;

	/* Zeiger auf Quelle der Rueckkopplung erhoehen */
	von_zustand++;
	if (von_zustand >= zustand + ZUSTAND_EINTRAEGE)
		von_zustand = zustand;

	/* Zeiger auf Ziel der Rueckkopplung erhoehen */
	nach_zustand++;
	if (nach_zustand >= zustand + ZUSTAND_EINTRAEGE)
		nach_zustand = zustand;

	return (wert >> 1) & 0x7fffffff; /* unterstes Bit hinausschieben */
}


/*
** zufall_init
**  initialisiert die Zustandstabelle fuer zufall; kompatibel zu BSD-srandom
**
** Parameter:
**  startwert: Startwert fuer die Inittialisierung der Tabelle
**
** Seiteneffekte:
**  von_zustand, nach_zustand und zustand werden gesetzt
*/
void zufall_init(unsigned long startwert)
{
	int i; /* Index und Zaehler */

	/* Zustandtabelle mit linearem Kongruenzengenerator initialisieren;
	   kompatibel zu BSD-srand gefolgt von ZUSTAND_EINTRAEGE-1 mal
	   BSD-rand */
	zustand[0] = startwert;
	for (i = 1; i < ZUSTAND_EINTRAEGE; i++)
		zustand[i] = 1103515245 * zustand[i - 1] + 12345;

	/* Zeiger fuer Rueckkopplung initialisieren */
	von_zustand = zustand;
	nach_zustand = zustand + 3;

	/* zehnmal die gesamte Zustandstabelle durchlaufen, um die Werte
	   gut zu verwuerfeln */
	for (i = 0; i < 10 * ZUSTAND_EINTRAEGE; i++)
		zufall();
}


/*
** wurzel
**  berechnet die Quadratwurzel aus einer ganzen Zahl, gerundet auf
**  die naechstliegende ganze Zahl
**
** Parameter:
**  x: Zahl, aus der die Wurzel zu ziehen ist
**  runden: Richtung, in die gerundet werden soll
**
** Rueckgabewert:
**  Wurzel aus x, gerundet
*/
long wurzel(unsigned long x, int runden)
{
	int ergebnis; /* Wurzel(x)/2^bit */
	int rest;     /* x/2^(2*bit) - ergebnis^2 */
	int bit;      /* Anzahl der noch zu ermittelnden Ergebnisbits */

	for (ergebnis = rest = 0, bit = CHAR_BIT * sizeof(unsigned) / 2; bit--;)
	{
		int rest_neu; /* Wert von rest fuer die naechste Runde, falls
		                 das naechste Ergebnisbit 1 ist */

		/* Platz fuer naechstes Ergebnisbit schaffen */
		ergebnis <<= 1;

		/* Platz fuer die naechsten zwei Restbits schaffen */
		rest <<= 2;

		/* die naechsten zwei Bits von x zum rest hinzurechnen */
		rest += x >> 2 * bit & 3;

		/* neuer rest, falls naechstes Ergebnisbit 1 ist; nach G. Binomi */
		rest_neu = rest - 2 * ergebnis - 1;

		/* kein Unterlauf? */
		if (rest_neu >= 0)
		{
			/* rest uebernehmen, Ergebnisbit auf 1 setzen */
			rest = rest_neu;
			ergebnis++;
		}
	}

	/* wohin runden? */
	switch (runden)
	{
		case W_ABRUNDEN: /* immer abrunden */
			/* Nachkommaanteil in rest einfach ignorieren */
			break;

		case W_RUNDEN: /* zum naechsten Wert runden */
			/* falls im naechsten Schleifendurchlauf ergebnis erhoeht
			   wuerde (erstes Nachkommabit 1), ergebnis jetzt erhoehen */
			if (rest > ergebnis)
				ergebnis++;
			break;

		case W_AUFRUNDEN: /* immer aufrunden */
			/* falls Wurzel nicht aufgegangen, ergebnis erhoehen */
			if (rest > 0)
				ergebnis++;
			break;
	}

	return ergebnis;
}
