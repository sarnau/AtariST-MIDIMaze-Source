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
** Datei: client.c
**
** Kommentar:
**  Das Hauptprogramm
**  und die Routinen, die Netzwerk, 3D-Berechnung und Grafik verbinden
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "argv.h"
#include "global.h"
#include "fehler.h"
#include "speicher.h"
#include "labyrinth.h"
#include "client.h"
#include "spieler.h"
#include "grafik.h"
#include "signale.h"
#include "ereignisse.h"
#include "einaus.h"
#include "rechne.h"
#include "netzwerk.h"
#include "spiel.h"

static char sccsid[] = "@(#)client.c	3.20 12/3/01";


struct arg_option argv_opts[] =
{
	{ Arg_Include, NULL, net_opts },
	{ Arg_Include, NULL, einaus_opts },
	{ Arg_End }
};


/* Breite und Laenge des aktuellen Spielfeldes: */
u_int feldbreite, feldlaenge;
block **spielfeld;        /* das Spielfeld */
block **feld_himricht[4]; /* das Spielfeld, in 4 Richtungen gedreht */

static int puffer_nr = 0;      /* Index auf Spieldaten-Puffer
                                  (welcher zuletzt gelesen wurde) */
static int puffer_gueltig = 0; /* Index auf Spieldaten-Puffer
                                  (welcher zuletzt beschrieben wurde) */

/* die Spieldaten werden in einem Doppelpuffer gespeichert;
   einer kann gelesen, der andere beschrieben werden */

static int spieler_aktiv[2]; /* der Spieler selbst ist im Spiel */
static int schuss_aktiv[2];  /* der Spieler selbst hat einen Schuss im
                                Spiel */
static int gegneranz[2];     /* Anzahl der aktuell sichtbaren Mitspieler */
static int schussanz[2];     /* Anzahl der sichtbaren Mitspieler-Schuesse */
static int abgeschossen_durch[2]; /* Farbe des Spielers, von dem man
                                     abgeschossen wurde */
static char *abschuss_spruch[2] = { NULL, NULL }; /* Spruch des Spielers, von
                                                     dem man abgeschossen
                                                     wurde */

/* Daten aller sichtbaren Spieler und Schuesse (Position, Farbe, ...): */
static struct spieler alle_spieler[2][SPIELERANZ];
static struct schuss alle_schuesse[2][SPIELERANZ];

static int alle_punkte[2][SPIELERANZ];  /* Punktestand aller Spieler; -1 fuer
                                           Spieler, die nicht existieren */
static char ereignisse[2][ERG_ANZ];     /* Ereignisse
                                           (siehe ereignisse.h) */
static char ereignisse_uebrig[ERG_ANZ]; /* noch nicht verarbeitete
                                           Ereignisse (temporaer) */

static int karteneu = 1; /* Flag, muss der Grundriss der Karte neu
                            gezeichnet werden?  Dann alles andere auch. */
static int spieler_farbe = 0; /* letzte bekannte Farbe des eigenen Spielers,
                                 sonst 0 */

/* Fehlermeldung und -knopf von milder_fehler (fuer milden_fehler_abfragen) */
static char **fehlermeldung = NULL, *fehlerknopf = NULL;

/* fuer das Verhindern ueberfluessigen Zeichnens */
static int alt_gueltig = 0;
static struct objektdaten alt_blick_vor, alt_blick_zurueck;
static int alt_spieler_index, alt_anzkreise;
static struct kartenkreis *alt_kreise;
static int alt_kompass;
static struct punktedaten alt_punkte;


/*
** drehlab
**  dreht das Spielfeld in 4 Richtungen (0, 90, 180, 270 Grad)
**  (damit die Projektionen nur fuer Richtung Norden implementiert werden
**  muessen)
**
** Seiteneffekte:
**  feld_himricht wird erzeugt
*/
static void drehlab(void)
{
	int i;    /* Index ueber die 4 Richtungen */
	int x, y; /* Index ueber Breite/Laenge des Spielfeldes */

	/* Speicher fuer 4 Spielfelder anlegen */
	/* bei ungeradem i Breite und Laenge tauschen */
	for (i = 1; i < 4; i++)
		speicher_belegen((void **)&feld_himricht[i],
			(i % 2 ? feldlaenge : feldbreite) * sizeof(block *));

	for (i = 1; i < 4; i++)
		for (y = 0; y < (i % 2 ? feldlaenge : feldbreite); y++)
			speicher_belegen((void **)&feld_himricht[i][y],
				(i % 2 ? feldbreite : feldlaenge) * sizeof(block));

	/* feld_himricht[0] wird nicht gedreht */
	feld_himricht[0] = spielfeld;

	/* feld_himricht[0] gedreht in feld_himricht[1,2,3] kopieren */
	for (y = 0; y < feldlaenge; y++)
		for (x = 0; x < feldbreite; x++)
			/* NORD = 0, WEST = 1, SUED = 2, OST = 3 */
			for (i = 0; i < 4; i++)
			{
				memcpy(&feld_himricht[1][feldlaenge - 1 - y]
					[x][(i + 3) % 4],
					&spielfeld[x][y][i], sizeof(struct wand));
				memcpy(&feld_himricht[2][feldbreite - 1 - x]
					[feldlaenge - 1 - y][(i + 2) % 4],
					&spielfeld[x][y][i], sizeof(struct wand));
				memcpy(&feld_himricht[3][y]
					[feldbreite - 1 - x][(i + 1) % 4],
					&spielfeld[x][y][i], sizeof(struct wand));
			}
}


/*
** objekt_listen_init
**  Listen anlegen fuer die Grafikobjekte, die rechne3d berechnet
**
** Parameter:
**  objekte: Zeiger auf die Struktur, die die Listen enthaelt
**
** Seiteneffekte:
**  *objekte wird veraendert
*/
static void objekt_listen_init(struct objektdaten *objekte)
{
	/* Waende- und Kugelliste anlegen */
	liste_initialisieren(&objekte->anzwaende, (void **)&objekte->waende);
	liste_initialisieren(&objekte->anzkugeln, (void **)&objekte->kugeln);

	/* Spruch des Gegners loeschen */
	objekte->text = NULL;
}


/*
** objekt_listen_freigeben
**  gibt die Listen mit zu zeichnenden Grafikobjekten wieder frei
**
** Parameter:
**  objekte: Zeiger auf die Struktur, die die Listen enthaelt
*/
static void objekt_listen_freigeben(struct objektdaten *objekte)
{
	int i; /* Index fuer die Kugelliste */

	liste_freigeben(&objekte->anzwaende, (void **)&objekte->waende);

	/* Unterliste (Sichbarkeitsbereiche der Kugeln) freigeben */
	for (i = 0; i < objekte->anzkugeln; i++)
		liste_freigeben(&objekte->kugeln[i].sichtanz,
			(void **)&objekte->kugeln[i].sichtbar);
	liste_freigeben(&objekte->anzkugeln, (void **)&objekte->kugeln);

	/* Speicher fuer Spruch des Gegners freigeben */
	if (objekte->text != NULL)
		speicher_freigeben((void **)&objekte->text);
}


/*
** punkte_vergleich
**  vergleicht zwei Punktestaende und gibt zurueck, in welcher Reihenfolge
**  sie angezeigt werden sollen
**
** Parameter:
**  p1: Zeiger auf einen Punktestand
**  p2: Zeiger auf anderen Punktestand
**
** Rueckgabewert:
**  < 0, falls Punktestand 1 vor Punktestand 2
**  = 0, falls egal
**  > 0, falls Punktestand 2 vor Punktestand 1
*/
static int punkte_vergleich(const void *p1, const void *p2)
{
	const struct punktestand *punkte1 = p1;
	const struct punktestand *punkte2 = p2;

	/* verschieden viele Punkte? */
	if (punkte1->punkte != punkte2->punkte)
		return punkte2->punkte - punkte1->punkte;

	/* sonst nach Farbe sortieren */
	return (int)punkte1->farbe - (int)punkte2->farbe;
}


static int flaechen_gleich(int n, struct flaeche *f1, struct flaeche *f2)
{
	int i, j;

	for (i = 0; i < n; i++)
	{
		for (j = 0; j < 4; j++)
			if (f1->ecke[j].x != f2->ecke[j].x ||
				f1->ecke[j].y != f2->ecke[j].y)
				return 0;

		if (f1->farbe != f2->farbe || f1->tuer != f2->tuer)
			return 0;

		f1++;
		f2++;
	}

	return 1;
}


static int kugeln_gleich(int n, struct kugel *k1, struct kugel *k2)
{
	int i, j;

	for (i = 0; i < n; i++)
	{
		struct ausschnitt *a1, *a2;

		if (k1->mittelpunkt.x != k2->mittelpunkt.x ||
			k1->mittelpunkt.y != k2->mittelpunkt.y)
			return 0;

		if (k1->sichtanz != k2->sichtanz ||
			k1->radiusx != k2->radiusx ||
			k1->radiusy != k2->radiusy ||
			k1->schatteny != k2->schatteny ||
			k1->schattenry != k2->schattenry ||
			k1->blick != k2->blick || k1->farbe != k2->farbe)
			return 0;

		a1 = k1->sichtbar;
		a2 = k2->sichtbar;
		for (j = 0; j < k1->sichtanz; j++)
		{
			if (a1->x != a2->x || a1->breite != a2->breite)
				return 0;

			a1++;
			a2++;
		}

		k1++;
		k2++;
	}

	return 1;
}


static int objektdaten_gleich(struct objektdaten *o1, struct objektdaten *o2)
{
	return o1->hintergrund_zeichnen == o2->hintergrund_zeichnen &&
		o1->anzwaende== o2->anzwaende &&
		o1->anzkugeln == o2->anzkugeln &&
		(o1->text == NULL && o2->text == NULL ||
			o1->text != NULL && o2->text != NULL &&
			!strcmp(o1->text, o2->text)) &&
		flaechen_gleich(o1->anzwaende, o1->waende, o2->waende) &&
		kugeln_gleich(o1->anzkugeln, o1->kugeln, o2->kugeln);
}


static int kartenkreise_gleich(int n, struct kartenkreis *k1,
	struct kartenkreis *k2)
{
	int i;

	for (i = 0; i < n; i++)
	{
		if (k1->mittelpunkt.x != k2->mittelpunkt.x ||
			k1->mittelpunkt.y != k2->mittelpunkt.y ||
			k1->radiusx != k2->radiusx ||
			k1->radiusy != k2->radiusy ||
			k1->farbe != k2->farbe)
			return 0;

		k1++;
		k2++;
	}

	return 1;
}


static int punkte_gleich(struct punktedaten *p1, struct punktedaten *p2)
{
	return p1->anzpunkte == p2->anzpunkte &&
		(p1->anzpunkte == 0 || !memcmp(p1->punkte, p2->punkte,
			p1->anzpunkte * sizeof *p1->punkte));
}


/*
** neuaufbau
**  erhaelt die neuen Spieldaten und startet die Grafikberechnung
**  und -ausgabe
**
** Parameter:
**  spieler_aktiv: Flag, ist der Spieler selbst im Spiel
**  gegneranz: Anzahl der aktuell sichtbaren Mitspieler
**  alle_spieler: Feld mit den Daten ueber alle sichtbaren Spieler
**  schuss_aktiv: Flag, hat der Spieler selbst einen Schuss im Spiel
**  schussanz: Anzahl der sichtbaren Schuesse der Mitspieler
**  alle_schuesse: Feld mit den Daten ueber alle sichtbaren Schuesse
**  abgeschossen_durch: Farbe des Spieler, von dem man abgeschossen wurde
**  abschuss_spruch: Spruch des Spieler, von dem man abgeschossen wurde
**  ereignisse: Feld mit den neuen Ereignissen
**  alle_punkte: Feld mit den Punktestaenden aller Spieler
**
** Seiteneffekte:
**  karteneu wird veraendert
*/
static void neuaufbau(int spieler_aktiv, int gegneranz,
	struct spieler alle_spieler[SPIELERANZ], int schuss_aktiv,
	int schussanz, struct schuss alle_schuesse[SPIELERANZ],
	int abgeschossen_durch, char *abschuss_spruch,
	char ereignisse[ERG_ANZ], int alle_punkte[SPIELERANZ])
{
	/* Struktur mit allen Grafik-Daten fuer die 3D-Sicht nach vorn und
	   Rueckspiegel */
	struct objektdaten blick_vor, blick_zurueck;
	int anzlinien;                /* Anzahl der Linien auf der Karte */
	int anzkreise;                /* Anzahl der Kreise auf der karte */
	struct kartenkreis *kreise;   /* Liste mit den Grafik-Daten der Linien
	                                 auf der Karte */
	struct kartenlinie *linien;   /* Liste mit den Grafik-Daten der Kreise
	                                 auf der Karte */
	struct spieler spieler_rueck; /* Daten des Spielers um 180 gedreht */
	struct punktedaten punkte;    /* Struktur mit den Daten der
	                                 Punktestaende */

	/* Listen anlegen, in den die Grafik-Daten abgelegt werden */
	objekt_listen_init(&blick_vor);
	objekt_listen_init(&blick_zurueck);
	liste_initialisieren(&anzkreise, (void **)&kreise);

	/* falls Karten-Grundriss gezeichnet werden muss, Grafik-Daten dafuer
	   berechnen */
	if (karteneu)
	{
		liste_initialisieren(&anzlinien, (void **)&linien);

		rechne_karte_linien(&anzlinien, &linien);
	}

	/* falls der Spieler lebt, muss der Horizont gezeichnet werden */
	blick_vor.hintergrund_zeichnen = blick_zurueck.hintergrund_zeichnen =
		spieler_aktiv;

	/* spieler_rueck entspricht dem Spieler mit Blick um 180 Grad gedreht */
	memcpy(&spieler_rueck, &alle_spieler[0], sizeof(struct spieler));
	spieler_rueck.blick = (spieler_rueck.blick + WINKANZ / 2) % WINKANZ;

	/* falls der Spieler lebt, werden die 3D-Berechungen fuer Front- und
	   Rueckansicht aufgerufen */
	if (spieler_aktiv)
	{
		/* Spielerfarbe merken */
		spieler_farbe = alle_spieler[0].farbe;

		rechne3d(&blick_vor, &alle_spieler[0], gegneranz,
			&alle_spieler[1], schussanz + schuss_aktiv,
			&alle_schuesse[1 - schuss_aktiv]);
		rechne3d(&blick_zurueck, &spieler_rueck, gegneranz,
			&alle_spieler[1], schussanz + schuss_aktiv,
			&alle_schuesse[1 - schuss_aktiv]);
	}
	else if (abgeschossen_durch >= 0)
	/* falls der Spieler nach einem Abschuss tot ist, wird das Gesicht des
	   Verursachers angezeigt */
	{
		struct kugel *kugel;           /* Daten des Gegnergesichts */
		struct ausschnitt *ausschnitt; /* Sichtbarkeitsbereich
		                                  des Gesichts */

		/* Listen mit Grafik-Daten erweitern */
		liste_verlaengern(&blick_vor.anzkugeln, (void **)&blick_vor.kugeln,
			sizeof(struct kugel));
		kugel = &blick_vor.kugeln[blick_vor.anzkugeln - 1];

		/* Grafik-Daten eines Gegnergesichts im Vollbild */
		kugel->mittelpunkt.x = FENSTER_BREITE / 2;
		kugel->mittelpunkt.y = FENSTER_HOEHE / 2;
		kugel->radiusx = FENSTER_BREITE / 4;
		kugel->radiusy = kugel->radiusx * 3 / 2;
		kugel->schatteny = kugel->schattenry = 0;
		kugel->blick = TRIGANZ / 2;
		kugel->farbe = abgeschossen_durch;

		/* Liste mit Sichtbarkeitsbereich (Gegner ist voll sichtbar) */
		liste_initialisieren(&kugel->sichtanz, (void **)&kugel->sichtbar);
		liste_verlaengern(&kugel->sichtanz, (void **)&kugel->sichtbar,
			sizeof(struct ausschnitt));
		ausschnitt = &kugel->sichtbar[kugel->sichtanz - 1];

		ausschnitt->x = 0;
		ausschnitt->breite = FENSTER_BREITE;

		/* Abschussspruch des Gegners bekannt? */
		if (abschuss_spruch != NULL)
		{
			/* dann Speicher belegen und kopieren */
			speicher_belegen((void **)&blick_vor.text,
				strlen(abschuss_spruch) + 1);
			strcpy(blick_vor.text, abschuss_spruch);
		}
	}

	/* falls Spieler an Leben, Karte mit ihm berechnen */
	if (spieler_aktiv)
		rechne_karte_kreise(&anzkreise, &kreise,
			gegneranz + 1, alle_spieler);
	else
	/* sonst nur die Mitspieler */
		rechne_karte_kreise(&anzkreise, &kreise,
			gegneranz, &alle_spieler[1]);

	{
		int i; /* Index fuer Spieler */

		/* Punktestaende in Liste kopieren */
		liste_initialisieren(&punkte.anzpunkte, (void **)&punkte.punkte);
		for (i = 0; i < SPIELERANZ; i++)
			if (alle_punkte[i] >= 0)
			{
				struct punktestand *stand; /* temporaerer Zeiger in Liste */

				/* Punktestand und Farbe des Spielers an Liste haengen */
				liste_verlaengern(&punkte.anzpunkte, (void **)&punkte.punkte,
					sizeof(struct punktestand));
				stand = &punkte.punkte[punkte.anzpunkte - 1];

				/* initialisieren, fuer den Vergleich */
				memset(stand, 0, sizeof *stand);

				stand->punkte = alle_punkte[i]; /* Punkte */
				stand->farbe = i + 1; /* Farbe */
				stand->hervorheben =
					spieler_farbe == i + 1; /* ich? */
			}
			else if (spieler_farbe == i + 1)
			/* ich, ohne Punktestand */
			{
				struct punktestand *stand; /* temporaerer Zeiger in Liste */

				/* Punktestand und Farbe des Spielers an Liste haengen */
				liste_verlaengern(&punkte.anzpunkte, (void **)&punkte.punkte,
					sizeof(struct punktestand));
				stand = &punkte.punkte[punkte.anzpunkte - 1];

				/* initialisieren, fuer den Vergleich */
				memset(stand, 0, sizeof *stand);

				stand->punkte = -1; /* Punkte */
				stand->farbe = i + 1; /* Farbe */
				stand->hervorheben = 1; /* ja, ich */
			}

		/* Punkte sortieren */
		if (punkte.anzpunkte)
			qsort(punkte.punkte, punkte.anzpunkte,
				sizeof(struct punktestand), punkte_vergleich);
	}

	/* Zeichenvorgang synchronisieren */
	zeichne_sync_anfang();

	/* Karten-Grundriss, falls notwendig, neuzeichnen */
	if (karteneu)
		zeichne_grundriss(anzlinien, linien);

	/* 3D-Frontansicht zeichnen */
	if (!alt_gueltig || karteneu ||
		!objektdaten_gleich(&blick_vor, &alt_blick_vor))
		zeichne_blickfeld(&blick_vor);
	if (alt_gueltig)
		objekt_listen_freigeben(&alt_blick_vor);
	memcpy(&alt_blick_vor, &blick_vor, sizeof alt_blick_vor);

	/* Rueckspiegel zeichnen */
	if (!alt_gueltig || karteneu ||
		!objektdaten_gleich(&blick_zurueck, &alt_blick_zurueck))
		zeichne_rueckspiegel(&blick_zurueck);
	if (alt_gueltig)
		objekt_listen_freigeben(&alt_blick_zurueck);
	memcpy(&alt_blick_zurueck, &blick_zurueck, sizeof alt_blick_zurueck);

	/* Kreise auf der Karte zeichnen; falls Spieler tot, ihn selbst nicht */
	{
		int spieler_index;

		spieler_index = spieler_aktiv ? 0 : -1;
		if (!alt_gueltig || spieler_index != alt_spieler_index ||
			anzkreise != alt_anzkreise ||
			!kartenkreise_gleich(anzkreise, kreise, alt_kreise))
			zeichne_karte(spieler_index, anzkreise, kreise);
		if (alt_gueltig)
			liste_freigeben(&alt_anzkreise, (void **)&alt_kreise);
		alt_spieler_index = spieler_index;
		alt_anzkreise = anzkreise;
		alt_kreise = kreise;
	}

	/* Kompass zeichnen */
	{
		int kompass;

		/* falls Spieler tot, Kompass ohne Nadel zeichnen */
		if (spieler_aktiv)
			kompass = alle_spieler[0].blick * (TRIGANZ / WINKANZ);
		else
			kompass = -1;

		if (!alt_gueltig || karteneu || kompass != alt_kompass)
			zeichne_kompass(kompass);

		alt_kompass = kompass;
	}

	/* Punktestand zeichnen */
	if (!alt_gueltig || karteneu || !punkte_gleich(&punkte, &alt_punkte))
		zeichne_punkte(&punkte);
	if (alt_gueltig)
		liste_freigeben(&alt_punkte.anzpunkte,
			(void **)&alt_punkte.punkte);
	memcpy(&alt_punkte, &punkte, sizeof alt_punkte);

	/* audio-visuelle Darstellung der Ereignisse */
	ereignisse_darstellen(ereignisse);

	/* Zeichenvorgang synchronisieren */
	zeichne_sync_ende();

	if (karteneu)
	{
		liste_freigeben(&anzlinien, (void **)&linien);

		/* falls Karten-Grundriss neu gezeichnet wurde, muss dies naechstes
		   Mal vorerst nicht wiederholt werden */
		karteneu = 0;
	}

	alt_gueltig = 1;
}


/*
** client_ende
**  beendet das Programm ohne Rueckfrage
*/
static void client_ende(void)
{
	/* lokale Datenstrukturen aufraeumen */
	grafik_ende();

	/* alle Netzverbindungen beenden */
	netzwerk_ende();

	exit(0);
}


/* bis hier lokaler Teil                       */
/***********************************************/
/* ab hier globaler Teil                       */


/*
** spiel_puffer_anlegen
**  ermittelt die Zeiger auf die aktuellen Puffer, in die die neuen
**  Spieldaten abgelegt werden sollen
**
** Parameter:
**  spieler_aktiv_var: Zeiger auf Zeiger auf Flag,
**                     ist der Spieler selbst im Spiel?
**  spieler_var: Zeiger auf Zeiger auf Daten des Spielers selbst
**  gegneranz_var: Zeiger auf Zeiger auf Anzahl der aktuell
**                 sichtbaren Mitspieler
**  gegner_feld: Zeiger auf Liste mit den Daten der sichtbaren Mitspieler
**  schuss_aktiv_var: Zeiger auf Zeiger auf Flag, hat der Spieler
**                    selbst einen Schuss im Spiel?
**  schuss_var: Zeiger auf Zeiger auf Daten des eigenen Schusses
**  schussanz_var: Zeiger auf Zeiger auf Anzahl der sichtbaren
**                 Schuesse der Mitspieler
**  schuesse_feld: Zeiger auf Liste mit den Daten der sichtbaren
**                 Schuesse der Mitspieler
**  abgechossen_durch_var: Zeiger auf Zeiger auf Farbe des Spieler,
**                         von dem man abgeschossen wurde
**  abschluss_spruch_var: Zeiger auf Zeiger auf Spruch des Spieler,
**                        von dem man abgeschossen wurde
**  ereignisse_feld: Zeiger auf Liste mit den neuen Ereignissen
**  punkte_feld: Zeiger auf Liste mit den Punktestaenden
**
** Seiteneffekte:
**  ereignisse_uebrig wird gesetzt
*/
void spiel_puffer_anlegen(int **spieler_aktiv_var,
	struct spieler **spieler_var, int **gegneranz_var,
	struct spieler **gegner_feld, int **schuss_aktiv_var,
	struct schuss **schuss_var, int **schussanz_var,
	struct schuss **schuesse_feld, int **abgeschossen_durch_var,
	char ***abschuss_spruch_var, char **ereignisse_feld, int **punkte_feld)
{
	/* Speicher fuer alten Spruch freigeben */
	if (abschuss_spruch[1 - puffer_nr] != NULL)
		speicher_freigeben((void **)&abschuss_spruch[1 - puffer_nr]);

	/* Zeiger auf die Puffer ermitteln; jeweils der andere als puffer_nr */
	*spieler_aktiv_var = &spieler_aktiv[1 - puffer_nr];
	*spieler_var = alle_spieler[1 - puffer_nr];
	*gegneranz_var = &gegneranz[1 - puffer_nr];
	*gegner_feld = &alle_spieler[1 - puffer_nr][1];

	*schuss_aktiv_var = &schuss_aktiv[1 - puffer_nr];
	*schuss_var = alle_schuesse[1 - puffer_nr];
	*schussanz_var = &schussanz[1 - puffer_nr];
	*schuesse_feld = &alle_schuesse[1 - puffer_nr][1];

	*abgeschossen_durch_var = &abgeschossen_durch[1 - puffer_nr];

	*abschuss_spruch_var = &abschuss_spruch[1 - puffer_nr];

	*ereignisse_feld = ereignisse[1 - puffer_nr];

	*punkte_feld = alle_punkte[1 - puffer_nr];

	/* falls die vorigen Pufferinhalte noch nicht abgearbeitet wurden,
	   alle Ereignisse retten */
	if (puffer_gueltig != puffer_nr)
	{
		int i;

		for (i = 0; i < ERG_ANZ; i++)
			ereignisse_uebrig[i] = (*ereignisse)[i];
	}
}


/*
** netzwerk_spielende
**  wird aufgerufen, wenn der Server den Client aus dem Spiel genommen hat;
**  es wird auf der Stelle eine Meldung ausgegben und das Programm beendet
*/
void netzwerk_spielende(void)
{
	static char *meldung[] = { "iMaze - Fatal Error", "",
		"Disconnected by server.", NULL };

	uebler_fehler(meldung, NULL);
}


/*
** signale_abfragen
**  wird aufgerufen, wenn die neuen Daten in die Puffer uebertragen wurden.
**  Liest die Eingabe-Signale aus
**
** Parameter:
**  signale: Feld mit allen Signalen
**
** Seiteneffekte:
**  puffer_gueltig wird gesetzt
*/
void signale_abfragen(char signale[SIGNALANZ])
{
	/* falls der vorige Pufferinhalt noch nicht abgearbeitet wurde,
	   werden die unbearbeiteten Ereignisse wieder uebernommen */
	if (puffer_gueltig != puffer_nr)
	{
		int i;

		for (i = 0; i < ERG_ANZ; i++)
			(*ereignisse)[i] |= ereignisse_uebrig[i];
	}

	/* der andere Puffer hat jetzt die aktuelle Daten */
	puffer_gueltig = 1 - puffer_nr;

	/* Abfrage der Eingabe-Signale */
	eingabe_abfragen(signale);
}


/*
** bewegung_deskriptor
**  liefert einen Deskriptor fuer die Verbindung zum Server im Spiel-Modus
**
** Rueckgabewert:
**  ein Zeiger auf den Deskriptor, NULL, falls es noch keine gibt
*/
void *bewegung_deskriptor(void)
{
	/* nur den Deskriptor vom Netzwerkteil abfragen und weiterreichen */
	return spiel_deskriptor();
}


/*
** bewegung
**  wartet auf Daten vom Server und initiiert evtl. die Neuberechnung und
**  -zeichnung aller Grafikausgaben
**
** Parameter:
**  ms: Zeit in Millisekunden, die maximal gewartet werden darf;
**      oder -1 fuer unbegrenztes Warten
**  zeichnen: Flag, ob die Grafik neu berechnet und gezeichnet werden soll
**
** Rueckgabewert:
**  1 fuer Fehler, sonst 0
**
** Seiteneffekt:
**  puffer_nr wird veraendert
*/
int bewegung(int ms, int zeichnen)
{
	{
		static int rekursion = 0; /* Flag, ob spiel_paket_erwarten laeuft */

		/* Rekursion von spiel_paket_erwarten verhindern */
		if (rekursion)
			/* kein Fehler */
			return 0;

		rekursion = 1;

		/* Daten vom Server erwarten */
		if (spiel_paket_erwarten(ms))
		{
			rekursion = 0;

			/* Fehler aufgetreten */
			return 1;
		}

		rekursion = 0;
	}

	/* falls nicht gezeichnet werden soll, fertig */
	if (!zeichnen)
		/* kein Fehler */
		return 0;

	if (puffer_gueltig == puffer_nr)
		/* in diesem Fall liegen keine neuen Daten vor; kein Fehler */
		return 0;

	/* der andere Puffer enthaelt die neuen Daten */
	puffer_nr = puffer_gueltig;

	/* Neuberechnung und -zeichnung aller Grafikausgaben mit neuen Daten */
	neuaufbau(spieler_aktiv[puffer_nr], gegneranz[puffer_nr],
		alle_spieler[puffer_nr], schuss_aktiv[puffer_nr],
		schussanz[puffer_nr], alle_schuesse[puffer_nr],
		abgeschossen_durch[puffer_nr], abschuss_spruch[puffer_nr],
		ereignisse[puffer_nr], alle_punkte[puffer_nr]);

	/* kein Fehler */
	return 0;
}


/*
** client_spiel_starten
**  wird aufgerufen, um die Verbindung zum Server aufzubauen
**
** Parameter:
**  server_name: Name des Servers
**  spruch: Spruch des Spielers oder NULL
**  kamera: Flag, ob der Spieler nur zusehen will
**
** Rueckgabewert:
**  1 fuer Fehler, sonst 0
**
** Seiteneffekte:
**  Daten werden vom Server empfangen; karteneu wird initialisiert
*/
int client_spiel_starten(char *server_name, char *spruch, int kamera)
{
	/* Verbindung zum angegebenen Server aufbauen */
	if (verbindung_aufbauen(server_name))
		return 1;

	/* Spielparameter / Labyrinth empfangen */
	if (spielparameter_empfangen("iMaze 3D client JC/HUK 1.4",
		"Anonymous User", strlen(spruch) ? spruch : NULL, kamera,
		&feldbreite, &feldlaenge, &spielfeld, NULL, 1 /* starten */))
		return 1;

	/* Spielfeld drehen */
	drehlab();

	/* Karte neu aufbauen */
	karteneu = 1;

	/* Spielerfarbe noch unbekannt */
	spieler_farbe = 0;

	/* jetzt kanns losgehen */
	if (verbindung_auf_spiel())
		return 1;

	/* kein Fehler */
	return 0;
}


/*
**  client_spiel_beenden
**   wird aufgerufen, wenn eine Spielrunde verlassen wird (disconnect)
*/
void client_spiel_beenden(void)
{
	/* Verbindung zum Server beenden */
	spiel_verlassen();
}


/*
** uebler_fehler
**  zeigt eine Fehlermeldung an und beendet nach Druecken eines Knopfes
**  das Programm
**
** Parameter:
**  meldung: Feld von Strings, die die Zeilen des auszugebenden Textes
**           beinhalten
**  knopf: Text der auf dem Knopf stehen soll;
**         falls NULL, so wird ein Standardtext verwendet
*/
void uebler_fehler(char **meldung, char *knopf)
{
	/* Fehler anzeigen */
	ueblen_fehler_anzeigen(meldung, knopf);

	/* Programm beenden */
	client_ende();
}


/*
** milder_fehler
**  merkt sich eine Fehlermeldung fuer die Routine milden_fehler_abfragen
**
** Parameter:
**  meldung: Feld von Strings, die die Zeilen des auszugebenden Textes
**           beinhalten
**  knopf: Text, der auf dem Knopf stehen soll;
**         falls NULL, so wird ein Standardtext verwendet
**
** Seiteneffekte:
**  fehlermeldung und fehlerknopf werden gesetzt
*/
void milder_fehler(char **meldung, char *knopf)
{
	int n; /* Anzahl der Fehlerstrings */
	int i; /* Index fuer Fehlerstrings */

	/* Speicher fuer vorige Fehlermeldung freigeben */
	if (fehlermeldung != NULL)
	{
		for (i = 0; fehlermeldung[i] != NULL; i++)
			speicher_freigeben((void **)&fehlermeldung[i]);
		speicher_freigeben((void **)&fehlermeldung);
	}

	/* Strings zaehlen */
	for (n = 0; meldung[n] != NULL; n++);

	/* Speicher fuer das Feld belegen und durch Nullzeiger begrenzen */
	speicher_belegen((void **)&fehlermeldung, (n + 1) * sizeof *fehlermeldung);
	fehlermeldung[n] = NULL;

	/* Fehlerstrings kopieren */
	for (i = 0; i < n; i++)
	{
		speicher_belegen((void **)&fehlermeldung[i], strlen(meldung[i]) + 1);
		strcpy(fehlermeldung[i], meldung[i]);
	}

	/* Speicher fuer vorige Knopfbeschriftung freigeben */
	if (fehlerknopf != NULL)
		speicher_freigeben((void **)&fehlerknopf);

	/* Knopfbeschriftung kopieren */
	if (knopf == NULL)
		fehlerknopf = NULL;
	else
	{
		speicher_belegen((void **)&fehlerknopf, strlen(knopf) + 1);
		strcpy(fehlerknopf, knopf);
	}
}


/*
** milden_fehler_abfragen
**  fragt den in milder_fehler gemerkten Fehlertext und -knopf ab
**
** Parameter:
**  meldung: Zeiger auf Feld von Strings, das die Zeilen des
**           auszugebenden Textes beinhalten soll
**  knopf: Zeiger auf Text, der auf dem Knopf stehen soll
**
** Seiteneffekte:
**  *meldung und *knopf werden gesetzt
*/
void milden_fehler_abfragen(char ***meldung, char **knopf)
{
	/* Rueckgabewerte setzen */
	*meldung = fehlermeldung;
	*knopf = fehlerknopf;
}


char *get_server_name(int *override)
{
	return net_get_server_name(override);
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
	/* Optionen via argv.h auswerten */
	process_args(&argc, argv);

	/* Netzwerkroutinen initialisieren */
	if (netzwerk_init())
		/* bei Fehler beenden */
		exit(1);

	/* Fenster oeffnen und lokale Datenstrukturen initialisieren */
	/* XXX argv ist hier immer leer */
	if (grafik_init(&argc, argv))
		/* bei Fehler beenden */
		exit(1);

	/* Hauptschleife */
	grafik_schleife();

	/* Programm beenden */
	client_ende();
	return 0;
}
