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
** Datei: gentrigtab.c
**
** Kommentar:
**  Berechnet die Sinus-, Cosinus- und Arcustangens-Tabellen
**  und schreibt sie in trigtab.c
*/


#include <stdio.h>
#include <math.h>

#include "global.h"

static char sccsid[] = "@(#)gentrigtab.c	3.2 12/3/01";


#define N (TRIGANZ / 4)  /* N = 90 Grad */

#define M (TRIGANZ / 2)  /* M entspricht tan = 1 */

static int tab[N + 1];   /* fuer Cosinuswerte von 0 bis 90 Grad (incl.) */
static int tab2[M + 1];  /* fuer Arctustangenswerte von 0 bis 1 (incl.) */


/*
** main
**  die Hauptroutine
**
** Seiteneffekte:
**  Die Datei 'trigtab.c' wird geschrieben
*/
int main(void)
{
	int i;

	/* Cosinuswerte bis 90 Grad berechnen (-TRIGFAKTOR ... TRIGFAKTOR) */
	for (i = 0; i < N; i++)
	{
		tab[i] = cos(3.1415926536 / 2.0 * i / N) * TRIGFAKTOR;
	}
	tab[N] = 0;  /* 90 Grad */

	/* Arcustangenswerte von 0 bis 1 mittels Sinus/Cosinus-Tabelle
	   berechnen (0 ... TRIGANZ/8) */
	{
		int winkel;
		int j, k; /* Tangens von winkel - 1 und winkel */

		j = -1;
		for (winkel = 1; winkel <= N / 2; winkel++)
		{
			k = (M * tab[N - winkel] + tab[winkel] / 2) / tab[winkel];
			for (i = j + 1; i <= k; i++)
				tab2[i] = i <= (j + k) / 2 ? winkel - 1 : winkel;
			j = k;
		}
	}

	/* Dateikopf schreiben */
	puts("/*");
	puts("** iMaze: Sinus-, Cosinus- und Arcustangens-Tabelle");
	puts("** Diese Datei wird automatisch erstellt!");
	puts("*/");
	putchar('\n');
	puts("#include \"global.h\"");
	putchar('\n');
	printf("static char sccsid[] = \"%s\";\n\n", sccsid);

	/* Cosinus-Tabelle ausgeben */
	puts("int costab[TRIGANZ] =");
	putchar('{');
	for (i = 0; i < TRIGANZ; i++)
	{
		/* 8 Werte pro Zeile */
		if (i % 8)
			putchar(' ');
		else
			putchar('\n'), putchar('\t');

		printf("%d,", i < N ? tab[i] : i < 2 * N ? -tab[2 * N - i] :
			i < 3 * N ? -tab[i - 2 * N] : tab[4 * N - i]);
	}
	putchar('\n');
	puts("};");
	putchar('\n');

	/* Sinus-Tabelle ausgeben */
	puts("int sintab[TRIGANZ] =");
	putchar('{');
	for (i = 0; i < TRIGANZ; i++)
	{
		/* 8 Werte pro Zeile */
		if (i % 8)
			putchar(' ');
		else
			putchar('\n'), putchar('\t');

		printf("%d,", i < N ? tab[N - i] : i < 2 * N ? tab[i - N] :
			i < 3 * N ? -tab[3 * N - i] : -tab[i - 3 * N]);
	}
	putchar('\n');
	puts("};");
	putchar('\n');

	/* Arcustangens-Tabelle ausgeben */
	puts("int atantab[TRIGANZ + 1] =");
	putchar('{');
	for (i = 0; i <= TRIGANZ; i++)
	{
		/* 8 Werte pro Zeile */
		if (i % 8)
			putchar(' ');
		else
			putchar('\n'), putchar('\t');

		printf("%d,", i < M ?
			(TRIGANZ - tab2[M - i]) % TRIGANZ : tab2[i - M]);
	}
	putchar('\n');
	puts("};");
	return 0;
}
