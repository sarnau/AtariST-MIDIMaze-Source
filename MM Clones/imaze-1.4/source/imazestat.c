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
** Datei: imazestat.c
**
** Kommentar:
**  Ein Client zum Abfragen des Serverstatus
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "argv.h"
#include "global.h"
#include "labyrinth.h"
#include "spieler.h"
#include "signale.h"
#include "netzwerk.h"
#include "spiel.h"
#include "system.h"

static char sccsid[] = "@(#)imazestat.c	3.8 12/3/01";


static char *server = NULL;

struct arg_option argv_opts[] =
{
	{ Arg_End, NULL, &server, NULL, "server" }
};


void uebler_fehler(char **meldung, char *knopf)
{
	int i;

	for (i = 0; meldung[i] != NULL; i++)
		fprintf(stderr, "imazestat: %s\n", meldung[i]);

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
	/* sollte hier nie ankommen */
	exit(10);
}


void signale_abfragen(char signale[SIGNALANZ])
{
	/* sollte hier nie ankommen */
	exit(10);
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
	char *server_status;

	process_args(&argc, argv);

	/* zur Sicherheit ein Timeout */
	signal(SIGALRM, SIG_DFL);
	timer_starten(300 * 1000);

	/* Netzwerkroutinen initialisieren */
	netzwerk_init();

	timer_starten(300 * 1000);

	verbindung_aufbauen(server);

	timer_starten(300 * 1000);

	if (spielparameter_empfangen("iMaze imazestat JC/HUK 1.4",
		"Anonymous User", NULL, 0 /* keine kamera */,
		NULL, NULL, NULL,
		&server_status, 0 /* nicht starten */))
	{
		fprintf(stderr, "%s: connection to server failed\n",
			program_name);
		exit(1);
	}

	if (server_status == NULL)
	{
		fprintf(stderr, "%s: server doesn't support status request\n",
			program_name);
		exit(1);
	}

	fputs(server_status, stdout);
	return 0;
}
