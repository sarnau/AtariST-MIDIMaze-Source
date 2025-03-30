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
** Datei: X_daten.c
**
** Kommentar:
**  Verwaltung der Objektlisten
*/


#include <stdio.h>
#include <string.h>

#include "global.h"
#include "speicher.h"
#include "grafik.h"
#include "X_daten.h"

static char sccsid[] = "@(#)X_daten.c	3.3 12/3/01";


/*
** objekte_kopieren
**  kopiert Liste, in der die Grafikobbjekte fuer die 3D-Darstellung
**  zwischengespeichert werden
**
** Parameter:
**  objekte: Liste, Ziel
**  objekte_neu: Liste, Quelle
**
** Seiteneffekte:
**  objekte_neu wird gesetzt
*/
void objekte_kopieren(struct objektdaten *objekte,
	struct objektdaten *objekte_neu)
{
	int i; /* Index fuer die Kugeln */

	objekte->hintergrund_zeichnen = objekte_neu->hintergrund_zeichnen;

	/* Liste der Waende freigeben und dann neue Kopie erstellen */
	liste_freigeben(&objekte->anzwaende, (void **)&objekte->waende);
	liste_kopieren(&objekte->anzwaende, (void **)&objekte->waende,
		&objekte_neu->anzwaende, (void **)&objekte_neu->waende,
		sizeof(struct flaeche));

	/* in Liste mit Kugeln jeweils Unterliste mit Sichtbarkeitsbereichen
	   freigeben */
	for (i = 0; i < objekte->anzkugeln; i++)
		liste_freigeben(&objekte->kugeln[i].sichtanz,
			(void **)&objekte->kugeln[i].sichtbar);

	/* Liste mit Kugeln freigeben und dann neu kopieren */
	liste_freigeben(&objekte->anzkugeln, (void **)&objekte->kugeln);
	liste_kopieren(&objekte->anzkugeln, (void **)&objekte->kugeln,
		&objekte_neu->anzkugeln, (void **)&objekte_neu->kugeln,
		sizeof(struct kugel));

	/* in Liste mit Kugeln jeweils Unterliste mit Sichtbarkeitsbereichen
	   neu kopieren */
	for (i = 0; i < objekte->anzkugeln; i++)
		liste_kopieren(&objekte->kugeln[i].sichtanz,
			(void **)&objekte->kugeln[i].sichtbar,
			&objekte_neu->kugeln[i].sichtanz,
			(void **)&objekte_neu->kugeln[i].sichtbar,
			sizeof(struct ausschnitt));

	/* alten Spruch des Gegners freigeben */
	if (objekte->text != NULL)
		speicher_freigeben((void **)&objekte->text);

	/* Spruch des Gegners kopieren */
	if (objekte_neu->text == NULL)
		objekte->text = NULL;
	else
	{
		speicher_belegen((void **)&objekte->text,
			strlen(objekte_neu->text) + 1);
		strcpy(objekte->text, objekte_neu->text);
	}
}


/*
** objekt_listen_init
**  initialisiert Liste, in der die Grafikobbjekte fuer die 3D-Darstellung
**  zwischengespeichert werden
**
** Parameter:
**  objekte: Liste, die initialisiert werden soll
**
** Seiteneffekte:
**  *objekte wird gesetzt
*/
void objekt_listen_init(struct objektdaten *objekte)
{
	/* zu Anfang ist das Bild leer */
	objekte->hintergrund_zeichnen = 0;

	/* objekte enthaelt ihrerseits eine Liste von Waenden und Kugeln */
	liste_initialisieren(&objekte->anzwaende, (void **)&objekte->waende);
	liste_initialisieren(&objekte->anzkugeln, (void **)&objekte->kugeln);

	/* kein Gegner-Spruch */
	objekte->text = NULL;
}
