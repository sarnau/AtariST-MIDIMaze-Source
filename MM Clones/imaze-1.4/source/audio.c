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
** Datei: audio.c
**
** Kommentar:
**  Plays sounds
*/

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/ioctl.h>

#include "argv.h"
#include "fehler.h"
#include "speicher.h"
#include "system.h"
#include "ereignisse.h"
#include "grafik.h"
#include "audio.h"
#include "audio_hw.h"

static char sccsid[] = "@(#)audio.c	3.33 12/09/01";


static int want_no_audio = 0;
static char *set_audio_directory = NULL;
struct arg_option audio_opts[] =
{
	{ Arg_Simple, "q", &want_no_audio, "disable sound" },
	{ Arg_String, "S", &set_audio_directory,
		"specify directory containing .au files", "directory" },
	{ Arg_Include, NULL, audio_hw_opts },
	{ Arg_End }
};


/* default directory for sound files */
#ifndef DEFAULT_SOUND_DIR
#define DEFAULT_SOUND_DIR "."
#endif


/* Daten pro Sekunde */
#define SAMPLING_RATE 8000

#define	SCHLUSS_STILLE (SAMPLING_RATE / 200)

/* maximale Anzahl von Sounds, die gemischt werden */
#define MAX_MISCH 10

static int use_ulaw;

/* Parameter fuer die linear -> u-law Wandlung */
static int ulaw_parameter[16][3] =
{
	1152, 258, 128 * 1157,
	960, 900, 64 * 964,
	480, 0, 32 * 482,
	224, 17, 16 * 225,
	1, 0, 8 * 1,
	1, 0, 4 * 1,
	18, 3, 2 * 19,
	1, 0, 1 * 1,
	-1, 0, 1 * 1,
	-1, 0, 2 * 1,
	-1, 0, 4 * 1,
	-104, 9, 8 * 105,
	-224, 17, 16 * 225,
	-224, 33, 32 * 225,
	-256, 129, 64 * 257,
	-256, 0, 128 * 257
};

/* linear -> u-law Tabelle */
static unsigned char ulaw[8192];

/* u-law -> linear Tabelle */
static short linear[256] =
{
	-32256, -31228, -30200, -29172, -28143, -27115, -26087, -25059,
	-24031, -23002, -21974, -20946, -19918, -18889, -17861, -16833,
	-16062, -15548, -15033, -14519, -14005, -13491, -12977, -12463,
	-11949, -11435, -10920, -10406, -9892, -9378, -8864, -8350,
	-7964, -7707, -7450, -7193, -6936, -6679, -6422, -6165,
	-5908, -5651, -5394, -5137, -4880, -4623, -4365, -4108,
	-3916, -3787, -3659, -3530, -3402, -3273, -3144, -3016,
	-2887, -2759, -2630, -2502, -2373, -2245, -2116, -1988,
	-1891, -1827, -1763, -1698, -1634, -1570, -1506, -1441,
	-1377, -1313, -1249, -1184, -1120, -1056, -992, -927,
	-879, -847, -815, -783, -751, -718, -686, -654,
	-622, -590, -558, -526, -494, -461, -429, -397,
	-373, -357, -341, -325, -309, -293, -277, -261,
	-245, -228, -212, -196, -180, -164, -148, -132,
	-120, -112, -104, -96, -88, -80, -72, -64,
	-56, -48, -40, -32, -24, -16, -8, 0,
	32256, 31228, 30200, 29172, 28143, 27115, 26087, 25059,
	24031, 23002, 21974, 20946, 19918, 18889, 17861, 16833,
	16062, 15548, 15033, 14519, 14005, 13491, 12977, 12463,
	11949, 11435, 10920, 10406, 9892, 9378, 8864, 8350,
	7964, 7707, 7450, 7193, 6936, 6679, 6422, 6165,
	5908, 5651, 5394, 5137, 4880, 4623, 4365, 4108,
	3916, 3787, 3659, 3530, 3402, 3273, 3144, 3016,
	2887, 2759, 2630, 2502, 2373, 2245, 2116, 1988,
	1891, 1827, 1763, 1698, 1634, 1570, 1506, 1441,
	1377, 1313, 1249, 1184, 1120, 1056, 992, 927,
	879, 847, 815, 783, 751, 718, 686, 654,
	622, 590, 558, 526, 494, 461, 429, 397,
	373, 357, 341, 325, 309, 293, 277, 261,
	245, 228, 212, 196, 180, 164, 148, 132,
	120, 112, 104, 96, 88, 80, 72, 64,
	56, 48, 40, 32, 24, 16, 8, 0
};

/* Namen der Ereignisse fuer Dateinamen */
static char *ereignis_namen[ERG_ANZ + 1] =
{
	"failure",
	"action",
	"shoot",
	"miss",
	"injured",
	"dead",
	"wound",
	"kill",
	"pause",
	"suspended",
	"awake",
	"bounce",

	"title"
};

/* Puffer fuer die verschiedenen Sounds */
static struct
{
	int laenge;    /* Laenge des Sounds */
	short *inhalt; /* Inhalt des Sounds (linear) */
} sounds[ERG_ANZ + 1];

/* Puffer fuer die zur Zeit abzuspielenden Sounds */
static struct
{
	int laenge;      /* restliche Laenge */
	short *position; /* restlicher Inhalt (linear) */
} abspiel[MAX_MISCH];

static int abspiel_anz;      /* Anzahl der zur Zeit abzuspielenden Sounds */


/* Puffer fuer zu spielende Audio-Daten */
static unsigned char *audio_puffer;

/* Puffer fuer die linearen Audio-Daten */
static int *linear_puffer;

/* tatsaechliche Puffer-Groesse */
static int puffer_groesse;


/*
** init_ulaw
**  initialisiert die linear -> u-law Wandlungstabelle
**
** Seiteneffekte:
**  ulaw wird initialisiert
*/
static void init_ulaw(void)
{
	int i, j, k, offset, von, bis;

	for (j = k = 0; k < 16; k++)
		for (i = 0;; i++)
		{
			int wert;

			wert = ulaw_parameter[k][0] * (i + ulaw_parameter[k][1]) /
				ulaw_parameter[k][2];
			if (!i)
			{
				if (k < 8)
				{
					von = 16 * k;
					bis = von + 16 - (k == 7);
				}
				else
				{
					von = 383 - 16 * k;
					bis = von - 16;
				}
				offset = von - wert;
			}
			wert += offset;
			if (wert == bis)
				break;
			ulaw[j++] = wert;
		}
}


/*
** audio_handler
**  berechnet und spielt Audio-Daten
**
** Parameter:
**  neu: Flag, ob neue Ereignisse ausgeloest wurden
**
** Seiteneffekte:
**  abspiel und abspiel_anz werden veraendert
*/
static void audio_handler(int neu)
{
	static int audio_puffer_laenge = 0; /* Laenge der noch
	                                       abzuspielenden Audio-Daten */
	int protect_state;

	protect_state = audio_hw_protect();

	for (;;)
	{
		/* keine Daten mehr im Puffer? */
		if (!audio_puffer_laenge)
		{
			int i; /* Index fuer die
			          abzuspielenden Sounds */

			/* alle aktuell abzuspielenden Sounds bearbeiten */
			for (i = 0; i < abspiel_anz; i++)
			{
				int laenge; /* restliche Laenge des Sounds; hoechstens
				               soviel wie ein Puffer-Inhalt */
				int j;      /* Index fuer den Audio-Puffer */

				/* Laenge berechnen */
				laenge = abspiel[i].laenge > puffer_groesse ?
					puffer_groesse : abspiel[i].laenge;

				/* war Audio-Puffer kuerzer als jetzt
				   abzuspielende Daten? */
				if (laenge > audio_puffer_laenge)
				{
					/* Puffer initialisieren */
					for (j = audio_puffer_laenge; j < laenge; j++)
						linear_puffer[j] = 0;

					/* Laenge anpassen */
					audio_puffer_laenge = laenge;
				}

				/* lineare Daten aufaddieren */
				for (j = 0; j < laenge; j++)
					linear_puffer[j] += abspiel[i].position[j];

				/* Sound zuende? */
				if (!(abspiel[i].laenge -= laenge))
				{
					/* Sound muss nicht mehr weiter abgespielt werden */
					abspiel_anz--;
					abspiel[i].laenge = abspiel[abspiel_anz].laenge;
					abspiel[i].position = abspiel[abspiel_anz].position;

					/* Schleifenindex fuer die Sounds korrigieren */
					i--;

					/* naechster Sound */
					continue;
				}

				/* Position der als naechstes abzuspielenden Daten */
				abspiel[i].position += laenge;
			}

			/* Audio-Puffer nicht leer? */
			if (audio_puffer_laenge)
				for (i = 0; i < audio_puffer_laenge; i++)
				{
					int wert; /* linearer Audio-Wert */

					wert = linear_puffer[i];

					/* Wert begrenzen */
					if (wert < -32768)
						wert = -32768;
					else if (wert > 32767)
						wert = 32767;

					/* Wert konvertieren (u-law oder ein Byte linear) */
					if (use_ulaw)
						audio_puffer[i] =
							ulaw[(wert + 32768) >> 3];
					else
						audio_puffer[i] =
							(wert + 32768) >> 8;
				}
		}

		/* wenn Audio-Puffer leer, Schleife beenden */
		if (!audio_puffer_laenge)
			break;

		/* Puffer des Audio-Devices noch nicht abgespielt? */
		if (audio_hw_is_busy())
		{
			/* dann noch keine neuen Daten spielen */
			audio_hw_unprotect(protect_state);
			return;
		}

		{
			int laenge; /* Laenge der geschriebenen Daten */

			/* Audio-Daten schreiben */
			laenge = audio_hw_write(audio_puffer,
				audio_puffer_laenge);

			/* Fehler? */
			if (laenge < 0)
			{
				audio_hw_unprotect(protect_state);

				/* audio_hw_ready_callback aufrufen, wenn
				   weitere Daten geschrieben werden koennen */
				audio_hw_hint_waiting();

				/* fertig */
				return;
			}

			/* Puffer verkuerzen */
			audio_puffer_laenge -= laenge;
			if (audio_puffer_laenge)
			{
				int i; /* Index */

				/* Daten nach vorne verschieben */
				for (i = audio_puffer_laenge; i--;)
					audio_puffer[i] = audio_puffer[i + laenge];

				audio_hw_unprotect(protect_state);

				/* fertig */
				return;
			}

			/* Puffer noch nicht leer, neu versuchen */
		}

		if (!audio_hw_is_multiwrite_allowed())
			break;
	}

	audio_hw_unprotect(protect_state);

	/* xv_audio_handler nicht mehr aufrufen */
	if (!audio_puffer_laenge)
		audio_hw_hint_not_waiting();
}


static int audio_load(char *audio_verzeichnis)
{
	int i;            /* Index fuer Ereignisse */
	int geladen;      /* mindestens eine Sound-Datei gefunden? */
	char *datei_name; /* Dateiname fuer Sound-Dateien */

	if (use_ulaw = audio_hw_uses_ulaw())
		init_ulaw(); /* linear -> u-law Tabelle initialisieren */

	for (i = 0; i <= ERG_ANZ; i++)
		sounds[i].laenge = 0;

	/* Speicherplatz fuer Dateinamen belegen */
	speicher_belegen((void **)&datei_name,
		strlen(audio_verzeichnis) + 40);

	for (i = geladen = 0; i <= ERG_ANZ; i++)
	{
		int laenge;     /* Laenge der Sound-Datei */
		int deskriptor; /* Deskriptor fuer die Sound-Datei */

		/* Ereignis-Name definiert oder Nummer verwenden? */
		if (ereignis_namen[i] != NULL)
			sprintf(datei_name, "%s/%s.au", audio_verzeichnis,
				ereignis_namen[i]);
		else
			sprintf(datei_name, "%s/%d.au", audio_verzeichnis, i);

		/* Datei oeffnen */
		if ((deskriptor = open(datei_name, O_RDONLY)) >= 0)
		{
			/* Laenge abfragen */
			laenge = lseek(deskriptor, 0, SEEK_END);
			lseek(deskriptor, 0, SEEK_SET);

			/* Datei lang genug? */
			if (laenge >= 7)
			{
				unsigned char *puffer; /* Puffer fuer Sound */
				int offset;            /* Position der Sounddaten */
				int j;                 /* Index fuer Sounddaten */

				/* temporaeren Puffer belegen */
				speicher_belegen((void **)&puffer, laenge);

				/* Daten lesen */
				read(deskriptor, puffer, laenge);

				/* au-Header vorhanden? */
				if (!strncmp((char *)puffer, ".snd", 4))
				{
					offset = (int)puffer[4] << 24 |
						(int)puffer[5] << 16 |
						(int)puffer[6] << 8 |
						(int)puffer[7];

					/* offset ungueltig? */
					if (offset < 0 || offset >= laenge)
						continue;
				}
				else
					offset = 0;

				/* offset ueberspringen, SCHLUSS_STILLE anhaengen */
				laenge -= offset - SCHLUSS_STILLE;

				/* Speicher fuer Sounddaten belegen */
				speicher_belegen((void **)&sounds[i].inhalt,
					sizeof *sounds[i].inhalt * laenge);

				/* Daten wandeln */
				for (j = 0; j < laenge - SCHLUSS_STILLE; j++)
					sounds[i].inhalt[j] = linear[puffer[offset + j]];

				/* Schlussstille anhaengen */
				for (; j < laenge; j++)
					sounds[i].inhalt[j] = 0;

				/* Laenge speichern */
				sounds[i].laenge = laenge;

				/* temporaeren Puffer anlegen */
				speicher_freigeben((void **)&puffer);

				/* es wurden Sound-Daten gefunden */
				geladen++;
			}

			/* Datei schliessen */
			close(deskriptor);
		}
	}

	/* Speicher fuer Dateiname freigeben */
	speicher_freigeben((void **)&datei_name);

	return geladen;
}


static void audio_play_title(void)
{
	/* Titelmusik abspielen */
	if (sounds[ERG_ANZ].laenge)
	{
		abspiel_anz = 1;
		abspiel[0].laenge = sounds[ERG_ANZ].laenge;
		abspiel[0].position = sounds[ERG_ANZ].inhalt;
	}
	else
		abspiel_anz = 0;

	puffer_groesse = audio_hw_get_buffer_size();

	speicher_belegen((void **)&audio_puffer,
		sizeof *audio_puffer * puffer_groesse);

	speicher_belegen((void **)&linear_puffer,
		sizeof *linear_puffer * puffer_groesse);

	audio_hw_start();
}


/* bis hier lokaler Teil                       */
/***********************************************/
/* ab hier globaler Teil                       */


void audio_play(char ereignisse[ERG_ANZ])
{
	char erg_nr;          /* Index fuer Ereignisse */
	int neu;              /* neues Ereignis aufgetreten? */
	int protect_state;

	protect_state = audio_hw_protect();

	for (erg_nr = neu = 0; erg_nr < ERG_ANZ; erg_nr++)

		/* Ereignis aufgetreten und passender Sound vorhanden? */
		if (ereignisse[erg_nr] && sounds[erg_nr].laenge)
		{
			/* noch Platz fuer neue Sounds? */
			if (abspiel_anz < MAX_MISCH)
			{
				/* Startposition des Sounds setzen */
				abspiel[abspiel_anz].laenge = sounds[erg_nr].laenge;
				abspiel[abspiel_anz].position = sounds[erg_nr].inhalt;

				/* ein Sound mehr zum Abspielen */
				abspiel_anz++;

				/* neue Ereignisse aufgetreten, neue Sounds */
				neu = 1;
			}
		}

	audio_hw_unprotect(protect_state);

	audio_handler(neu); /* Sound starten */
}


void audio_close(void)
{
	int i; /* Index fuer Ereignisse */

	/* Audio-Device schliessen */
	audio_hw_close();

	/* Speicher freigeben */
	for (i = 0; i <= ERG_ANZ; i++)
		if (sounds[i].laenge)
			speicher_freigeben((void **)&sounds[i].inhalt);
}


void audio_hw_ready_callback(void)
{
	audio_handler(0);
}


/*
**	audio_directory = "": use default
*/
audio_e audio_start(char *audio_directory)
{
	int want;

	want = !want_no_audio;
	if (set_audio_directory != NULL)
		audio_directory = set_audio_directory;
	else if (audio_directory[0] == 0)
		audio_directory = DEFAULT_SOUND_DIR;

	if (!want)
		return Audio_Failed;

	/* try to open audio device */
	for (;;)
	{
		char **message;
		int try_again;

		switch (audio_open(&message))
		{
		case -1: /* no audio support */
			return Audio_Failed;

		case 1: /* successful open */
			goto successful_open;

		case 0: /* permanent error */
			try_again = 0;
			break;

		default: /* temporary inavailability */
			try_again = 1;
		}

		/* other error? */
		if (!try_again)
			if (rueckfrage(message, "No Sound", NULL))
				return Audio_Failed;
			else
				return Audio_Abort;
		/* audio device busy? */
		else if (!rueckfrage(message, "Try again", "No Sound"))
			return Audio_Failed;
	}

successful_open:
	audio_init();

	/* no audio input file? */
	if (!audio_load(audio_directory))
	{
		static char *message[] = { "iMaze - Sound Error", "",
			"No audio file found in:", NULL, NULL };

		message[3] = audio_directory;

		if (!rueckfrage(message, "No Sound", NULL))
			return Audio_Abort;

		audio_close();
		return Audio_Failed;
	}

	audio_play_title();
	return Audio_Started;
}
