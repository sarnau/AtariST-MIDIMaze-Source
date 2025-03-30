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
** Datei: voxware_audio.c
**
** Kommentar:
**  Plays sounds via Voxware kernel driver
*/

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/ioctl.h>

#ifdef linux
#include <linux/soundcard.h>
#else
#include <machine/soundcard.h>
#endif

#include "argv.h"
#include "ereignisse.h"
#include "fehler.h"
#include "system.h"
#include "audio.h"
#include "audio_hw.h"

static char sccsid[] = "@(#)voxware_audio.c	3.18 12/09/01";


struct arg_option audio_hw_opts[] =
{
	{ Arg_End }
};


#define DEV_AUDIO "/dev/dsp"

/* Daten pro Sekunde */
#define SAMPLING_RATE 8000

#define	LOG2_PUFFER_GROESSE 10
#define	PUFFER_GROESSE (1 << LOG2_PUFFER_GROESSE)

static int audio_deskriptor; /* Deskriptor fuer das Audio-Device */

/* tatsaechliche Puffer-Groesse */
static int puffer_groesse;


/* bis hier lokaler Teil                       */
/***********************************************/
/* ab hier globaler Teil                       */


void audio_init(void)
{
	{
		/* 2 Fragmente der Groesse PUFFER_GROESSE */
		int frag = 2 << 16 | LOG2_PUFFER_GROESSE;

		if (ioctl(audio_deskriptor, SNDCTL_DSP_SETFRAGMENT, &frag))
		{
			static char *meldung[] = { "iMaze - Sound Error", "",
				"Can't set fragment size:", NULL, NULL };

			meldung[3] = fehler_text();
			uebler_fehler(meldung, NULL);
		}
	}

	/* Puffer-Groesse abfragen */
	if (ioctl(audio_deskriptor, SNDCTL_DSP_GETBLKSIZE, &puffer_groesse))
	{
		static char *meldung[] = { "iMaze - Sound Error", "",
			"Can't check audio buffer size:", NULL, NULL };

		meldung[3] = fehler_text();
		uebler_fehler(meldung, NULL);
	}

	/* Puffer zu gross? */
	if (puffer_groesse > PUFFER_GROESSE)
		puffer_groesse = PUFFER_GROESSE;

	{
		int speed = SAMPLING_RATE;

		/* Abtastrate setzen */
		if (ioctl(audio_deskriptor, SNDCTL_DSP_SPEED, &speed))
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
}


void audio_hw_close(void)
{
	close(audio_deskriptor);
}


int audio_hw_protect(void)
{
	return 0; /* don't care about state information */
}


void audio_hw_unprotect(int state)
{
}


int audio_hw_is_busy(void)
{
	return 0; /* always try to write */
}


int audio_hw_is_multiwrite_allowed(void)
{
	return 0; /* never try to write more than once */
}


/* buffer[length], buffer[length + 1], ... might be modified! */
int audio_hw_write(unsigned char *buffer, int length)
{
	int written;

	if (length < puffer_groesse)
		memset(buffer + length, 128, puffer_groesse - length);

	written = write(audio_deskriptor, buffer, puffer_groesse);

	if (written >= length)
		return length;
	else
		return written;
}


void audio_hw_hint_waiting(void)
{
	audio_select_descriptor(audio_deskriptor, audio_hw_ready_callback, 1);
}


void audio_hw_hint_not_waiting(void)
{
	audio_select_descriptor(audio_deskriptor, (void (*)(void))NULL, 1);
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
	return 0; /* no, linear encoding used */
}
