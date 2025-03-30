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
** Datei: sun_audio.c
**
** Kommentar:
**  Plays sounds on Suns
*/

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/ioctl.h>

#ifdef SUN_AUDIOIO_H
#include <sun/audioio.h>
#else
#include <sys/audioio.h>
#endif

#include <stropts.h>

#include "argv.h"
#include "fehler.h"
#include "system.h"
#include "ereignisse.h"
#include "audio.h"
#include "audio_hw.h"

static char sccsid[] = "@(#)sun_audio.c	3.18 12/05/01";


struct arg_option audio_hw_opts[] =
{
	{ Arg_End }
};


#define DEV_AUDIO "/dev/audio"
#define AUDIO_CTL "/dev/audioctl"


/* Daten pro Sekunde */
#define SAMPLING_RATE 8000

#define	PUFFER_GROESSE (SAMPLING_RATE / 10)

static int audio_deskriptor; /* Deskriptor fuer das Audio-Device */

/* tatsaechliche Puffer-Groesse */
static int puffer_groesse;

static int audio_ctl_deskriptor; /* Deskriptor fuer das
                                    Audio-Kontroll-Device */
static int eof_zaehler;          /* Anzahl bisher abgespielter
                                    Audio-Pakete */


/* bis hier lokaler Teil                       */
/***********************************************/
/* ab hier globaler Teil                       */


void audio_init(void)
{
	puffer_groesse = PUFFER_GROESSE;

	{
		/* Audio-Kontroll-Device oeffnen */

		struct audio_info info; /* Status des Audio-Devices */

		/* Audio-Kontroll-Device oeffnen */
		if ((audio_ctl_deskriptor = open(AUDIO_CTL, O_RDWR, 0)) < 0)
		{
			static char *meldung[] = { "iMaze - Sound Error", "",
				NULL, NULL, NULL };
			char text[80];

			sprintf(text, "Can't open %s:", AUDIO_CTL);
			meldung[2] = text;
			meldung[3] = fehler_text();
			uebler_fehler(meldung, NULL);
		}

		/* Audio-Status abfragen */
		if (ioctl(audio_ctl_deskriptor, AUDIO_GETINFO, &info))
		{
			static char *meldung[] = { "iMaze - Sound Error", "",
				"Can't check audio play counter:", NULL, NULL };

			meldung[3] = fehler_text();
			uebler_fehler(meldung, NULL);
		}

		/* Startwert des Audio-Paket-Zaehlers merken */
		eof_zaehler = info.play.eof;
	}

	{
		struct audio_info info; /* Audio-Status */

		/* Status initialisieren */
		AUDIO_INITINFO(&info);

		/* Abtastrate setzen */
		info.play.sample_rate = SAMPLING_RATE;
		if (ioctl(audio_deskriptor, AUDIO_SETINFO, &info))
		{
			static char *meldung[] = { "iMaze - Sound Error", "",
				"Can't set audio sample rate:", NULL, NULL };

			meldung[3] = fehler_text();
			uebler_fehler(meldung, NULL);
		}
	}
}


void audio_hw_start(void)
{
	deskriptor_nicht_blockieren(audio_deskriptor);

	/* Handler fuer Audio-Signal installieren */
	audio_handle_signal(SIGPOLL, audio_hw_ready_callback);

	/* Signale durch das Audio-Kontroll-Device zulassen */
	if (ioctl(audio_ctl_deskriptor, I_SETSIG, S_MSG))
	{
		static char *meldung[] = { "iMaze - Sound Error", "",
			"Can't enable signals on audio control device:",
			NULL, NULL };

		meldung[3] = fehler_text();
		uebler_fehler(meldung, NULL);
	}
}


void audio_hw_close(void)
{
	close(audio_ctl_deskriptor);

	audio_handle_signal(SIGPOLL, (void (*)(void))NULL);

	close(audio_deskriptor);
}


int audio_hw_protect(void)
{
	int was_unprotected;

	if (was_unprotected = !signal_blockiert(SIGPOLL))
		signal_blockieren(SIGPOLL);

	return was_unprotected;
}


void audio_hw_unprotect(int state)
{
	if (state) /* was unprotected */
		signal_freigeben(SIGPOLL);
}


int audio_hw_is_busy(void)
{
	/* Audio-Paket-Zaehler abfragen */
	struct audio_info info; /* Status des Audio-Kontroll-Devices */

	/* Status abfragen */
	if (ioctl(audio_ctl_deskriptor, AUDIO_GETINFO, &info))
	{
		static char *meldung[] = { "iMaze - Sound Error", "",
			"Can't check audio play counter:", NULL, NULL };

		meldung[3] = fehler_text();
		uebler_fehler(meldung, NULL);
	}

	/* Puffer des Audio-Devices noch nicht abgespielt? */
	return eof_zaehler != info.play.eof;
}


int audio_hw_is_multiwrite_allowed(void)
{
	return 1; /* always try to write more than once */
}


int audio_hw_write(unsigned char *buffer, int length)
{
	/* Audio-Paket-Zaehler erhoehen */
	eof_zaehler++;

	/* Signal ausloesen, wenn fertig */
	write(audio_deskriptor, buffer, 0);

	/* Daten in das Audio-Device schreiben */
	return write(audio_deskriptor, buffer, length);
}


void audio_hw_hint_waiting(void)
{
}


void audio_hw_hint_not_waiting(void)
{
}


int audio_open(char ***meldung)
{
	static char *message[] = { "iMaze - Sound Error", "",
		NULL, NULL, NULL };
	static char text[80];

	if ((audio_deskriptor = open(DEV_AUDIO, O_WRONLY | O_NDELAY, 0)) >= 0)
		return 1;

	sprintf(text, "Can't open %s:", DEV_AUDIO);
	message[2] = text;
	message[3] = fehler_text();

	*meldung = message;

	return errno == EBUSY ? 2 : 0;
}


int audio_hw_get_buffer_size(void)
{
	return puffer_groesse;
}


int audio_hw_uses_ulaw(void)
{
	return 1; /* yes */
}
