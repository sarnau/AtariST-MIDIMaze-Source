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
** Datei: ninja.c
**
** Kommentar:
**  Der Ninja
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "argv.h"
#include "global.h"
#include "speicher.h"
#include "labyrinth.h"
#include "farben.h"
#include "spieler.h"
#include "signale.h"
#include "ereignisse.h"
#include "netzwerk.h"
#include "spiel.h"
#include "system.h"

static char sccsid[] = "@(#)ninja.c	3.22 12/3/01";


static int ziel_farbe_parsen(char *str);
static int schussdichte_parsen(char *str);


static char *spruch = NULL;
static int inverse_strategie = 0;
static int use_quickturn = 0;


struct arg_option argv_opts[] =
{
	{ Arg_Include, NULL, net_opts },
	{ Arg_Simple, "i", &inverse_strategie, "inverse strategy: flee" },
	{ Arg_String, "m", &spruch, "set kill message", "message" },
	{ Arg_Callback, "s", (void *)schussdichte_parsen,
		"set shot probability", "percentage" },
	{ Arg_Callback, "t", (void *)ziel_farbe_parsen,
		"hunt player", "color-index" },
	{ Arg_Simple, "Q", &use_quickturn, "use quickturn" },
	{ Arg_End }
};


u_int feldbreite, feldlaenge;
block **spielfeld;


static int lebenszeit = 2 * 60 * 60 * 1000; /* in ms */

static int schussdichte = 100;
static int ziel_farbe = -1;

static int spieler_aktiv, gegneranz, schuss_aktiv;
static struct spieler spieler, gegner[SPIELERANZ];
static char ereignisse[ERG_ANZ];

static int ziel_feld = -1;


static int ziel_farbe_parsen(char *str)
{
	long farbe;
	char *end;

	farbe = strtol(str, &end, 10);
	if (*end != 0 || farbe < 0 || farbe > SPIELERANZ)
		return 1; /* parse error */

	ziel_farbe = farbe;
	return 0; /* ok */
}


static int schussdichte_parsen(char *str)
{
	long wert;
	char *end;

	wert = strtol(str, &end, 10);
	if (*end != 0 || wert < 0 || wert > 100)
		return 1; /* parse error */

	schussdichte = wert;
	return 0; /* ok */
}


static void laufen(char signale[SIGNALANZ])
{
	int *dist, i, j;

	speicher_belegen((void **)&dist, feldlaenge * feldbreite * sizeof(int));

	for (i = feldlaenge * feldbreite; i--; dist[i] = 0);

	dist[spieler.pos.ygrob * feldbreite + spieler.pos.xgrob] = 1;
	for (j = 0, i = 0; i < gegneranz; i++)
	{
		int k;

		if (gegner[i].farbe != ziel_farbe)
			continue;

		k = gegner[i].pos.ygrob * feldbreite + gegner[i].pos.xgrob;
		if (!dist[k])
			j++, dist[k] = -1;
	}
	if (!j)
		for (j = 0, i = 0; i < gegneranz; i++)
		{
			int k;

			k = gegner[i].pos.ygrob * feldbreite +
				gegner[i].pos.xgrob;
			if (!dist[k])
				j++, dist[k] = -1;
		}
	if (!j)
	{
		if (ziel_feld < 0 || zufall() % 50 == 0)
			ziel_feld = zufall() % (feldlaenge * feldbreite);
		dist[ziel_feld] = -1;
	}
	else
		ziel_feld = -1;

	for (;;)
	{
		int weiter;

		weiter = 0;

		for (i = feldlaenge * feldbreite; i--;)
		{
			if (dist[i] > 0)
			{
				int x, y;

				x = i % feldbreite;
				y = i / feldbreite;
				if (!spielfeld[x][y][NORD].unbegehbar &&
					dist[j = i - feldbreite] <= 0)
				{
					if (dist[j])
						break;
					dist[j] = i + 2;
					weiter = 1;
				}
				if (!spielfeld[x][y][WEST].unbegehbar &&
					dist[j = i - 1] <= 0)
				{
					if (dist[j])
						break;
					dist[j] = i + 2;
					weiter = 1;
				}
				if (!spielfeld[x][y][SUED].unbegehbar &&
					dist[j = i + feldbreite] <= 0)
				{
					if (dist[j])
						break;
					dist[j] = i + 2;
					weiter = 1;
				}
				if (!spielfeld[x][y][OST].unbegehbar &&
					dist[j = i + 1] <= 0)
				{
					if (dist[j])
						break;
					dist[j] = i + 2;
					weiter = 1;
				}
			}
			j = -1;
		}

		if (j >= 0)
			break;
		if (!weiter)
		{
			speicher_freigeben((void **)&dist);
			return;
		}
	}

	while (dist[i] > 1)
		j = i, i = dist[i] - 2;

	speicher_freigeben((void **)&dist);

	switch (j -= i)
	{
		case -1:
			i = WINKANZ / 4;
			break;
		case 1:
			i = WINKANZ * 3 / 4;
			break;
		default:
			if (j < 0)
				i = 0;
			else
				i = WINKANZ / 2;
	}

	if (spieler.pos.ygrob > 0 && (int)spieler.pos.yfein < KUGELRAD &&
		spielfeld[spieler.pos.xgrob][spieler.pos.ygrob - 1][SUED].unbegehbar)
		i = 0;
	else if (spieler.pos.xgrob > 0 && (int)spieler.pos.xfein < KUGELRAD &&
		spielfeld[spieler.pos.xgrob - 1][spieler.pos.ygrob][OST].unbegehbar)
		i = WINKANZ / 4;
	else if ((int)spieler.pos.ygrob < feldlaenge - 1 &&
		(int)spieler.pos.yfein > RASPT - KUGELRAD &&
		spielfeld[spieler.pos.xgrob][spieler.pos.ygrob + 1][NORD].unbegehbar)
		i = WINKANZ / 2;
	else if ((int)spieler.pos.xgrob < feldbreite - 1 &&
		(int)spieler.pos.xfein > RASPT - KUGELRAD &&
		spielfeld[spieler.pos.xgrob + 1][spieler.pos.ygrob][WEST].unbegehbar)
		i = WINKANZ * 3 / 4;

	i = (i - spieler.blick + WINKANZ) % WINKANZ;
	if (i)
		if (i < WINKANZ / 2)
			signale[LINKSSIGNAL] = 1;
		else
			signale[RECHTSSIGNAL] = 1;

	if (use_quickturn && i > WINKANZ / 4 && i < WINKANZ * 3 / 4 &&
		zufall() % 10 == 0)
		signale[QUICKTURNSIGNAL] = 1;

	if (i < WINKANZ / 4 || i > WINKANZ * 3 / 4)
		if (inverse_strategie)
			signale[ZURUECKSIGNAL] = 1;
		else
			signale[VORSIGNAL] = 1;
}


static void schiessen(char signale[SIGNALANZ])
{
	if (!schuss_aktiv && zufall() % 100 < schussdichte)
		signale[SCHUSSSIGNAL] = 1;

	if (schuss_aktiv && ereignisse[SCHUSS_PRALL_ERG] && zufall() % 4 == 0)
		signale[SCHUSSSIGNAL] = 1;
}


static void abbruch_signal_handler(int signum)
{
	spiel_verlassen();
	exit(0);
}


void uebler_fehler(char **meldung, char *knopf)
{
	int i;

	for (i = 0; meldung[i] != NULL; i++)
		fprintf(stderr, "ninja: %s\n", meldung[i]);

	exit(1);
}


void milder_fehler(char **meldung, char *knopf)
{
	uebler_fehler(meldung, knopf);
}


void spiel_puffer_anlegen(int **spieler_aktiv_var,
	struct spieler **spieler_var, int **gegneranz_var,
	struct spieler **gegner_feld, int **schuss_aktiv_var,
	struct schuss **schuss_var, int **schussanz_var,
	struct schuss **schuesse_feld, int **abgeschossen_durch_var,
	char ***abschuss_spruch_var, char **ereignisse_feld, int **punkte_feld)
{
	*spieler_aktiv_var = &spieler_aktiv;
	*spieler_var = &spieler;
	*gegneranz_var = &gegneranz;
	*gegner_feld = gegner;

	*schuss_aktiv_var = &schuss_aktiv;
	*schuss_var = NULL;
	*schussanz_var = NULL;
	*schuesse_feld = NULL;

	*abgeschossen_durch_var = NULL;
	*abschuss_spruch_var = NULL;

	*ereignisse_feld = ereignisse;

	*punkte_feld = NULL;

	lebenszeit -= 300 * 1000 - timer_restzeit();
	timer_starten(300 * 1000);

	if (lebenszeit < 0)
	{
		spiel_verlassen();
		exit(0);
	}
}


void netzwerk_spielende(void)
{
	exit(0);
}


void signale_abfragen(char signale[SIGNALANZ])
{
	int i;

	for (i = 0; i < SIGNALANZ; i++)
		signale[i] = 0;

	if (spieler_aktiv)
	{
		laufen(signale);
		schiessen(signale);
	}
}


/*
** main
**  die Hauptroutine
**
** Parameter:
**  argc: Anzahl der Argumente inklusive Programmname
**  argv: Argumentliste
*/
int main(int argc, char **argv)
{
	char tmp[100];

	process_args(&argc, argv);

	/* zur Sicherheit ein Timeout */
	signal(SIGALRM, SIG_DFL);
	timer_starten(300 * 1000);

	/* Netzwerkroutinen initialisieren */
	netzwerk_init();

	if (spruch == NULL)
	{
		void *name;

		strcpy(tmp, "Ninja of ");
		name = benutzer_name();
		if (name == NULL)
			strcpy(tmp + 9, "nobody");
		else
		{
			strncpy(tmp + 9, name, sizeof tmp - 9);
			tmp[sizeof tmp - 1] = 0;
			speicher_freigeben(&name);
		}

		spruch = tmp;
	}

	timer_starten(300 * 1000);

	verbindung_aufbauen(NULL);

	timer_starten(300 * 1000);

	if (spielparameter_empfangen("iMaze ninja JC/HUK 1.4", "Ninja", spruch,
		0 /* keine kamera */, &feldbreite, &feldlaenge, &spielfeld,
		NULL, 1 /* starten */))
		exit(1);

	{
		int x, y, i;

		for (x = 0; x < feldbreite; x++)
			for (y = 0; y < feldlaenge; y++)
				for (i = 0; i < 4; i++)
				{
					struct wand *wand;

					wand = &spielfeld[x][y][i];
					wand->unbegehbar = !IST_TUER(wand->farbe) &&
						wand->farbe != TRANSPARENT;
				}
	}

	timer_starten(300 * 1000);

	if (verbindung_auf_spiel())
		exit(1);

	handle_signal(SIGHUP, abbruch_signal_handler);
	handle_signal(SIGINT, abbruch_signal_handler);
	handle_signal(SIGTERM, abbruch_signal_handler);

	timer_starten(300 * 1000);

	/* Hauptschleife, ein Ninja stirbt nicht */
	for (;;)
		if (spiel_paket_erwarten(-1))
			break; /* ausser bei Fehlern */

	return 1;
}
