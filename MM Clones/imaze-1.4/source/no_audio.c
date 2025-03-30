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
** Datei: no_audio.c
**
** Kommentar:
**  Dummy for no sound support
*/

#include "argv.h"
#include "ereignisse.h"
#include "audio.h"
#include "audio_hw.h"

static char sccsid[] = "@(#)no_audio.c	3.8 12/3/01";


struct arg_option audio_hw_opts[] =
{
	{ Arg_End }
};


/* bis hier lokaler Teil                       */
/***********************************************/
/* ab hier globaler Teil                       */


void audio_init(void)
{
}


int audio_open(char ***meldung)
{
	return -1; /* no sound support */
}


/* these are never called: */

void audio_hw_start(void)
{
}


void audio_hw_close(void)
{
}


int audio_hw_protect(void)
{
	return 0;
}


void audio_hw_unprotect(int state)
{
}


int audio_hw_is_busy(void)
{
	return 1;
}


int audio_hw_is_multiwrite_allowed(void)
{
	return 0;
}


int audio_hw_write(unsigned char *buffer, int length)
{
	return -1;
}


void audio_hw_hint_waiting(void)
{
}


void audio_hw_hint_not_waiting(void)
{
}


int audio_hw_get_buffer_size(void)
{
	return 1024;
}


int audio_hw_uses_ulaw(void)
{
	return 0;
}
