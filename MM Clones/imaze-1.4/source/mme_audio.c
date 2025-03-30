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
** File: mme_audio.c
**
** Comment:
**  Plays sounds on DECs via the MME server
*/

#include <stdlib.h>
#include <string.h>
#include <mme/mme_api.h>

#include "argv.h"
#include "fehler.h"
#include "system.h"
#include "ereignisse.h"
#include "audio.h"
#include "audio_hw.h"

static char sccsid[] = "@(#)mme_audio.c	3.12 12/03/01";


static int device_name_callback(char *name);


struct arg_option audio_hw_opts[] =
{
	{ Arg_Callback, "D", (void *)device_name_callback, "set MME device",
		"number" },
	{ Arg_End }
};


/* samples per second */
#define SAMPLING_RATE 8000

#define	SAMPLE_BLOCK_SIZE (SAMPLING_RATE / 10)

static int audio_descriptor; /* descriptor for the pipe to the mmeserver */

static int blocks_pending; /* number of blocks sent to the mmeserver*/
static int next_block; /* index for block/block_header */

static UINT device = WAVE_MAPPER;

static HWAVEOUT wo_handle;
static HPSTR block[2];
static LPWAVEHDR block_header[2];


static void audio_error(char *text)
{
	static char *message[] = { "iMaze - Sound Error", "", NULL, NULL };

	message[2] = text;
	uebler_fehler(message, NULL);
}


static void wave_callback(HANDLE wo_handle, UINT msg, DWORD instance,
	LPARAM par1, LPARAM par2)
{
	if (msg == MM_WOM_DONE)
		blocks_pending--;
}


static void audio_callback(void)
{
	while (mmeCheckForCallbacks())
		mmeProcessCallbacks();
}


static int device_name_callback(char *name)
{
	char *end_p;
	unsigned long dev;

	dev = strtoul(name, &end_p, 10);
	if (dev >= 10000 || *end_p != 0)
		return 1; /* error */

	device = dev;
	return 0; /* success */
}


/* up to here local part                       */
/***********************************************/
/* from here on global part                    */


void audio_init(void)
{
	blocks_pending = 0;
}


void audio_hw_start(void)
{
}


void audio_hw_close(void)
{
	audio_select_descriptor(audio_descriptor, (void (*)(void))NULL, 0);

	if (blocks_pending)
	{
		if (waveOutReset(wo_handle) != MMSYSERR_NOERROR)
			audio_error("Can't reset MME");
	}

	if (waveOutClose(wo_handle) != MMSYSERR_NOERROR)
		audio_error("Can't close MME");
}


int audio_hw_protect(void)
{
	/* not needed, we don't use signals */
	return 0;
}


void audio_hw_unprotect(int state)
{
	/* not needed, we don't use signals */
}


int audio_hw_is_busy(void)
{
	return blocks_pending > 1; /* don't write more than two blocks */
}


int audio_hw_is_multiwrite_allowed(void)
{
	return 1; /* always try to write more than once */
}


int audio_hw_write(unsigned char *buffer, int length)
{
	MMRESULT err;

	if (length < 1 || blocks_pending > 1)
		return -1;

	if (length > SAMPLE_BLOCK_SIZE)
		length = SAMPLE_BLOCK_SIZE;

	memcpy(block[next_block], buffer, length);
	block_header[next_block]->dwBufferLength = length;

	if ((err = waveOutWrite(wo_handle, block_header[next_block],
		sizeof *block_header[next_block])) != MMSYSERR_NOERROR)
		audio_error("Can't send to MME");

	blocks_pending++;
	next_block = (next_block + 1) % 2;

	audio_callback(); /* don't forget events */

	return length;
}


void audio_hw_hint_waiting(void)
{
}


void audio_hw_hint_not_waiting(void)
{
}


int audio_open(char ***message_p)
{
	LPWAVEFORMAT wo_format;
	MMRESULT err;
	static char *message[] = { "iMaze - Sound Error", "", NULL, NULL };

	if (waveOutGetNumDevs() == 0)
	{
		message[2] = "No MME server or audio device present";
		*message_p = message;

		return 0; /* fail */
	}

	wo_format = mmeAllocMem(sizeof *wo_format);
	if (wo_format == NULL)
		audio_error("Can't allocate MME memory");

	memset(wo_format, 0, sizeof *wo_format);
	wo_format->wFormatTag = WAVE_FORMAT_MULAW;
	wo_format->nChannels = 1;
	wo_format->nSamplesPerSec = SAMPLING_RATE;
	wo_format->nAvgBytesPerSec = SAMPLING_RATE;
	wo_format->nBlockAlign = 1;

	if ((err = waveOutOpen(&wo_handle, device, wo_format,
		wave_callback, 0, CALLBACK_FUNCTION | WAVE_OPEN_SHAREABLE)) !=
		MMSYSERR_NOERROR)
	{
		message[2] = "Can't open MME device";
		*message_p = message;

		return err == MMSYSERR_ALLOCATED ? 2 : 0;
	}

	audio_descriptor = mmeServerFileDescriptor();

	block[0] = mmeAllocBuffer(SAMPLE_BLOCK_SIZE);
	if (block[0] == NULL)
		audio_error("Can't allocate MME memory");

	block[1] = mmeAllocBuffer(SAMPLE_BLOCK_SIZE);
	if (block[1] == NULL)
		audio_error("Can't allocate MME memory");

	block_header[0] = mmeAllocMem(sizeof *block_header[0]);
	if (block_header[0] == NULL)
		audio_error("Can't allocate MME memory");

	block_header[1] = mmeAllocMem(sizeof *block_header[1]);
	if (block_header[1] == NULL)
		audio_error("Can't allocate MME memory");

	memset(block_header[0], 0, sizeof *block_header[0]);
	block_header[0]->lpData = block[0];
	block_header[0]->dwBufferLength = SAMPLE_BLOCK_SIZE;
	block_header[0]->dwFlags = 0;
	block_header[0]->dwLoops = 0;

	memset(block_header[1], 0, sizeof *block_header[1]);
	block_header[1]->lpData = block[1];
	block_header[1]->dwBufferLength = SAMPLE_BLOCK_SIZE;
	block_header[1]->dwFlags = 0;
	block_header[1]->dwLoops = 0;

	blocks_pending = 0;
	next_block = 0;
	audio_select_descriptor(audio_descriptor, audio_callback, 0);

	audio_callback(); /* don't forget events */

	return 1; /* success */
}


int audio_hw_get_buffer_size(void)
{
	return SAMPLE_BLOCK_SIZE;
}


int audio_hw_uses_ulaw(void)
{
	return 1; /* yes */
}
