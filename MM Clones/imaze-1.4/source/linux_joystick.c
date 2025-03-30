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
** Datei: linux_joystick.c
**
** Kommentar:
**  Wertet Events vom Joystick auf Linux-Systemen aus
*/


#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <linux/joystick.h>

#include "argv.h"
#include "global.h"
#include "fehler.h"
#include "speicher.h"
#include "system.h"
#include "signale.h"
#include "grafik.h"
#include "joystick.h"

static char sccsid[] = "@(#)linux_joystick.c	3.7 12/3/01";


#ifndef DEFAULT_JOYSTICK_DEVICE
#define DEFAULT_JOYSTICK_DEVICE "/dev/js0"
#endif


static int want_joystick = 0;
static char *set_device_name = NULL;
struct arg_option joystick_opts[] =
{
	{ Arg_Simple, "j", &want_joystick, "enable joystick" },
	{ Arg_String, "J", &set_device_name, "set joystick device", "device" },
	{ Arg_End }
};


static int joystick;          /* Deskriptor fuer Joystick-Device */
static int joystick_abfragen; /* Flag: Joystick benutzen? */
static int joystick_gelesen;  /* Flag: joystick_xmax, ... initialisiert? */
static int joystick_xmin;     /* minimaler x-Wert der Joystick-Position */
static int joystick_xmax;     /* maximaler x-Wert der Joystick-Position */
static int joystick_ymin;     /* minimaler y-Wert der Joystick-Position */
static int joystick_ymax;     /* maximaler y-Wert der Joystick-Position */


/* bis hier lokaler Teil                       */
/***********************************************/
/* ab hier globaler Teil                       */


void joystick_trigger(void)
{
	/* Joystick benutzen? */
	if (joystick_abfragen)
	{
		int laenge;                         /* Laenge der gelesenen Daten */
		int j_x, j_y, j_button;             /* gelesene Daten */
		struct JS_DATA_TYPE joystick_daten; /* Position des Joysticks */

		/* Joystick abfragen */
		laenge = read(joystick, &joystick_daten, sizeof joystick_daten);

		/* Fehler? */
		if (laenge < 0 && errno != EWOULDBLOCK && errno != EAGAIN)
		{
			static char *meldung[] = { "iMaze - Joystick Error", "",
				"Can't read from joystick device:", NULL, NULL };

			meldung[3] = fehler_text();
			uebler_fehler(meldung, NULL);
		}

		/* stimmt die Laenge der gelesenen Daten? */
		if (laenge == sizeof joystick_daten)
		{
			j_x = joystick_daten.x;
			j_y = joystick_daten.y;

			j_button = joystick_daten.buttons != 0;

			/* xmin, ... schon initialisiert? */
			if (joystick_gelesen)
			{
				joystick_xmin = min(j_x, joystick_xmin);
				joystick_xmax = max(j_x, joystick_xmax);
				joystick_ymin = min(j_y, joystick_ymin);
				joystick_ymax = max(j_y, joystick_ymax);
			}
			else
			{
				joystick_xmin = joystick_xmax = j_x;
				joystick_ymin = joystick_ymax = j_y;

				joystick_gelesen = 1;
			}

			/* Joystick-Buttons abfragen */
			joystick_event(SCHUSSSIGNAL, j_button);

			/* x-Richtung abfragen */
			joystick_event(LINKSSIGNAL,
				3 * j_x < 2 * joystick_xmin + joystick_xmax);
			joystick_event(RECHTSSIGNAL,
				3 * j_x > joystick_xmin + 2 * joystick_xmax);

			/* y-Richtung abfragen */
			joystick_event(VORSIGNAL,
				3 * j_y < 2 * joystick_ymin + joystick_ymax);
			joystick_event(ZURUECKSIGNAL,
				3 * j_y > joystick_ymin + 2 * joystick_ymax);
		}
	}
}


int joystick_init(void)
{
	char *joystick_device; /* Name des Joystick-Devices */

	/* initialisieren */
	joystick_abfragen = want_joystick || set_device_name != NULL;
	joystick_device = set_device_name != NULL ? set_device_name :
		DEFAULT_JOYSTICK_DEVICE;

	/* Joystick nicht benutzen? */
	if (!joystick_abfragen)
		return 0;

	/* Joystick-Device oeffnen Erfolg oder Abbruch */
	for (;;)
	{
		static char *meldung[] = { "iMaze - Joystick Error",
			"", NULL, NULL, NULL };

		/* Joystick-Device oeffnen */
		if ((joystick = open(joystick_device, O_RDONLY | O_NDELAY, 0)) >= 0)
			break;

		/* Fehlertext erzeugen */
		speicher_belegen((void **)&meldung[2],
			strlen(joystick_device) + 20);
		sprintf(meldung[2], "Can't open %s:", joystick_device);
		meldung[3] = fehler_text();

		/* sonstiger Fehler? */
		if (errno != EBUSY)
			if (rueckfrage(meldung, "No Joystick", NULL))
			{
				joystick_abfragen = 0;
				speicher_freigeben((void **)&meldung[2]);

				return 0;
			}
			else
			{
				speicher_freigeben((void **)&meldung[2]);

				return 1;
			}
		/* Device gerade in Benutzung? */
		else if (!rueckfrage(meldung, "Try again", "No Joystick"))
		{
			joystick_abfragen = 0;
			speicher_freigeben((void **)&meldung[2]);

			return 0;
		}

		speicher_freigeben((void **)&meldung[2]);
	}

	/* Joystick-Device in nicht blockierenden Modus schalten */
	if (fcntl(joystick, F_SETFL, O_NONBLOCK))
	{
		static char *meldung[] = { "iMaze - Joystick Error", "",
			"Can't switch joystick device to non-blocking mode:",
			NULL, NULL };

		meldung[3] = fehler_text();
		uebler_fehler(meldung, NULL);
	}

	/* xmin, xmax, ... noch nicht initialisiert */
	joystick_gelesen = 0;
	return 0; /* Erfolg */
}


void joystick_close(void)
{
	/* Joystick schliessen */
	if (joystick_abfragen && close(joystick))
	{
		static char *meldung[] = { "iMaze - Joystick Error", "",
			"Can't close joystick device:", NULL, NULL };

		meldung[3] = fehler_text();
		uebler_fehler(meldung, NULL);
	}

	/* Joystick nicht mehr abfragen */
	joystick_abfragen = 0;
}
