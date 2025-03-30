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
** Datei: baulab.c
**
** Kommentar:
**  Erzeugt ein zufaelliges Labyrinth
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "global.h"
#include "speicher.h"
#include "labyrinth.h"
#include "farben.h"

static char sccsid[] = "@(#)baulab.c	3.7 12/04/01";


#define MAGIC "iMazeLab1\n"


#define BASISFARBE 1
#define BASISTUERFARBE 8

#ifndef NORMALFARBE
#define NORMALFARBE 5
#endif


extern char *optarg;
extern int optind;

u_int feldlaenge, feldbreite;
block **spielfeld;

static char *av0;

static int wanddichte, tuerdichte;
static long startwert;
static int suchen, laber;


static u_char zufallsfarbe(void)
{
#if NORMALFARBE == 0
	return zufall() % (WANDFARBENANZ / 2) + BASISFARBE;
#else
	return NORMALFARBE;
#endif
}


static u_char zufallstuerfarbe(void)
{
	return zufall() % (WANDFARBENANZ / 2 + 1) + BASISTUERFARBE;
}


static void wuerfellab(void)
{
	int x, y;

	for (x = 0; x < feldbreite; x++)
	{
		spielfeld[x][0][NORD].farbe = zufallsfarbe();
		for (y = 1; y < feldlaenge; y++)
			spielfeld[x][y - 1][SUED].farbe =
			spielfeld[x][y][NORD].farbe =
				zufall() % 100 < wanddichte ? zufallsfarbe() : TRANSPARENT;
		spielfeld[x][feldlaenge - 1][SUED].farbe = zufallsfarbe();
	}

	for (y = 0; y < feldlaenge; y++)
	{
		spielfeld[0][y][WEST].farbe = zufallsfarbe();
		for (x = 1; x < feldbreite; x++)
			spielfeld[x - 1][y][OST].farbe =
			spielfeld[x][y][WEST].farbe =
				zufall() % 100 < wanddichte ? zufallsfarbe() : TRANSPARENT;
		spielfeld[feldbreite - 1][y][OST].farbe = zufallsfarbe();
	}

	for (x = 0; x < feldbreite; x++)
		for (y = 0; y < feldlaenge; y++)
		{
			int i;

			for (i = 0; i < 4; i++)
			{
				spielfeld[x][y][i].schusssicher =
					spielfeld[x][y][i].farbe != TRANSPARENT;
				if (spielfeld[x][y][i].farbe == TRANSPARENT ||
					i == NORD && y == 0 ||
					i == WEST && x == 0 ||
					i == SUED && y == feldlaenge - 1 ||
					i == OST && x == feldbreite - 1)
					spielfeld[x][y][i].tuer = 0;
				else
				{
					spielfeld[x][y][i].tuer = zufall() % 100 < tuerdichte;
					if (spielfeld[x][y][i].tuer)
						spielfeld[x][y][i].farbe = zufallstuerfarbe();
				}
				spielfeld[x][y][i].unbegehbar =
					spielfeld[x][y][i].farbe != TRANSPARENT &&
					!spielfeld[x][y][i].tuer;
				spielfeld[x][y][i].tuer =
					spielfeld[x][y][i].tueroffen = 0;
			}
		}
}


static int durchgang(int x, int y, int nach)
{
	if (x < 0 || x >= feldbreite || y < 0 || y >= feldlaenge)
		return 0;
	switch (nach)
	{
		case NORD:
			if (y == 0)
				return 0;
			break;
		case WEST:
			if (x == 0)
				return 0;
			break;
		case SUED:
			if (y == feldlaenge - 1)
				return 0;
			break;
		case OST:
			if (x == feldbreite - 1)
				return 0;
			break;
	}
	return !spielfeld[x][y][nach].unbegehbar;
}


static int *flags;

static int setzen(int *x, int i)
{
	if (x - flags < 0 || x - flags >= feldlaenge * feldbreite)
		fprintf(stderr, "\n{%d:%d}\n", x - flags, i), exit(1);
	if (*x >= 0)
		return 0;
	*x = i;
	return 1;
}


static int testlab(void)
{
	int i, j, x, y, weiter, frei;

	speicher_belegen((void **)&flags,
		feldlaenge * feldbreite * sizeof(int));
	for (i = 0; i < feldlaenge * feldbreite; i++)
		flags[i] = -1;
	frei = feldlaenge * feldbreite;
	for (i = 0;;)
	{
		if (flags[i] != -1)
			fprintf(stderr, "\n[-%d:%d-]\n", i, flags[i]), exit(1);
		flags[i] = i;
		frei--;
		for (weiter = 1; weiter;)
		{
			weiter = 0;
			for (x = 0; x < feldbreite; x++)
				for (y = 0; y < feldlaenge; y++)
				{
					int k;
					if ((k = flags[y * feldbreite + x]) < 0)
						continue;
					if (durchgang(x - 1, y, OST))
						weiter += setzen(&flags[y * feldbreite + x - 1],
							durchgang(x, y, WEST) ? k :
							y * feldbreite + x - 1);
					if (durchgang(x + 1, y, WEST))
						weiter += setzen(&flags[y * feldbreite + x + 1],
							durchgang(x, y, OST) ? k :
							y * feldbreite + x + 1);
					if (durchgang(x, y - 1, SUED))
						weiter += setzen(&flags[(y - 1) * feldbreite + x],
							durchgang(x, y, NORD) ? k :
							(y - 1) * feldbreite + x);
					if (durchgang(x, y + 1, NORD))
						weiter += setzen(&flags[(y + 1) * feldbreite + x],
							durchgang(x, y, SUED) ? k :
							(y + 1) * feldbreite + x);
				}
			frei -= weiter;
		}
	nochmal:
		for (x = 0; x < feldbreite; x++)
			for (y = 0; y < feldlaenge; y++)
				if (flags[y * feldbreite + x] == i &&
					(durchgang(x, y, WEST) &&
					flags[j = y * feldbreite + x - 1] != i ||
					durchgang(x, y, OST) &&
					flags[j = y * feldbreite + x + 1] != i ||
					durchgang(x, y, NORD) &&
					flags[j = (y - 1) * feldbreite + x] != i ||
					durchgang(x, y, SUED) &&
					flags[j = (y + 1) * feldbreite + x] != i))
					goto ausgang;
		break;
	ausgang:
		if (j < 0 || j >= feldlaenge * feldbreite)
			fprintf(stderr, "[%d %d %d]\n", j, x, y), exit(1);
		if (flags[j] != -1)
		{
			int k = flags[j];
			for (j = 0; j < feldlaenge * feldbreite; j++)
				if (flags[j] == k)
					flags[j] = i;
			goto nochmal;
		}
		i = j;
	}
	if (frei != 0)
	{
		speicher_freigeben((void **)&flags);
		return 0;
	}
	for (j = x = 0; x < feldlaenge * feldbreite; x++)
		if (flags[x] < 0)
			fprintf(stderr, "\n=%d=\n", x), exit(1);
		else if (flags[x] == i)
			j++;
	speicher_freigeben((void **)&flags);
	return j > feldlaenge * feldbreite / 2;
}


static void baulab(void)
{
	int j;

	speicher_belegen((void **)&spielfeld, feldbreite * sizeof(block *));

	for (j = 0; j < feldbreite; j++)
		speicher_belegen((void **)&spielfeld[j],
			feldlaenge * sizeof(block));

	if (suchen)
	{
		for (j = 1; startwert > 0; startwert++, j++)
		{
			if (laber)
			{
				fprintf(stderr, "%ld\r", startwert);
				fflush(stderr);
			}
			zufall_init(startwert);
			wuerfellab();
			if (testlab())
				break;

			if (j > 999)
			{
				fprintf(stderr, "%s%s",
					"no success in 1000 tries, try",
					" reducing -d or increasing -D\n");
				exit(1);
			}
		}
		if (!(startwert > 0))
			exit(1);
	}
	else
	{
		zufall_init(startwert);
		wuerfellab();
	}
}


static void speicherlab(char *name)
{
	FILE *datei;
	u_char daten[4];
	int x, y, i;

	if (strcmp(name, "-"))
		datei = fopen(name, "wb");
	else
		datei = stdout;

	if (datei == NULL)
	{
	fehler:
		fprintf(stderr, "cannot write file '%s'\n", name);
		exit(1);
	}

	if (fwrite(MAGIC, 1, strlen(MAGIC), datei) != strlen(MAGIC))
		goto fehler;

	daten[0] = feldbreite;
	daten[1] = feldlaenge;
	if (fwrite(daten, 1, 2, datei) != 2)
		goto fehler;

	for (y = 0; y < feldlaenge; y++)
		for (x = 0; x < feldbreite; x++)
		{
			for (i = 0; i < 4; i++)
			{
				struct wand *wand;

				wand = &spielfeld[x][y][i];
				daten[i] = wand->unbegehbar << 7 |
					wand->schusssicher << 6 | wand->farbe;
			}
			if (fwrite(daten, 1, 4, datei) != 4)
				goto fehler;
		}

	if (laber)
		fprintf(datei == stdout ? stderr : stdout,
			"iMaze genlab JC/HUK 1.4: %dx%d, %d%% walls, %d%% doors, seed %ld\n",
			feldbreite, feldlaenge, wanddichte, tuerdichte, startwert);

	fprintf(datei,
		"iMaze genlab JC/HUK 1.4: %dx%d, %d%% walls, %d%% doors, seed %ld\n",
		feldbreite, feldlaenge, wanddichte, tuerdichte, startwert);

	if (datei != stdout)
		fclose(datei);
}


/* bis hier lokaler Teil                       */
/***********************************************/
/* ab hier globaler Teil                       */


void uebler_fehler(char **meldung, char *knopf)
{
	fprintf(stderr, "fatal error\n");
	exit(1);
}


int main(int argc, char **argv)
{
	int option, fehlerflag;

	if ((av0 = strrchr(argv[0], '/')) == NULL)
		av0 = argv[0];
	else
		av0++;

	feldlaenge = 16;
	feldbreite = 16;
	wanddichte = 38;
	tuerdichte = 5;

	zufall_init(time(NULL));
	zufall();
	startwert = zufall();

	laber = fehlerflag = 0;
	suchen = 1;

	while ((option = getopt(argc, argv, "w:h:d:D:Sr:v")) != -1)
		switch (option)
		{
			case 'w':
				feldbreite = atoi(optarg);
				fehlerflag |= feldbreite < 1 ||
					feldbreite > MAXFELDBREITE;
				break;
			case 'h':
				feldlaenge = atoi(optarg);
				fehlerflag |= feldlaenge < 1 ||
					feldlaenge > MAXFELDBREITE;
				break;
			case 'd':
				wanddichte = atoi(optarg);
				fehlerflag |= wanddichte < 0 ||
					wanddichte > 100;
				break;
			case 'D':
				tuerdichte = atoi(optarg);
				fehlerflag |= tuerdichte < 0 ||
					tuerdichte > 100;
				break;
			case 'S':
				suchen = 0;
				break;
			case 'r':
				startwert = atol(optarg);
				fehlerflag |= startwert < 1;
				break;
			case 'v':
				laber = 1;
				break;
			case '?':
				fehlerflag = 1;
		}

	if (fehlerflag || optind != argc - 1)
	{
		fprintf(stderr,
"usage: %s [-Sv] [-w width] [-h height] [-d wall-density]\n\
\t[-D door-density] [-r random-seed] lab-file\n", av0);
		exit(1);
	}

	baulab();

	speicherlab(argv[optind]);

	return 0;
}
