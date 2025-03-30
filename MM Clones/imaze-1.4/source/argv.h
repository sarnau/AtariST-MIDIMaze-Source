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
** File: argv.h
**
** Comment:
**  Types for command line parsing
*/


static char sccsid_argv[] = "@(#)argv.h	3.8 12/3/01";


typedef enum { Arg_End, Arg_Include,
	Arg_Simple, Arg_String, Arg_Callback } arg_e;


struct arg_option
{
	arg_e type;
	char *name; /* NULL for Arg_End and Arg_Include */

	/* struct arg_option * for Arg_Include,
	   int * (set to 1) for Arg_Simple,
	   char ** (set to != NULL) for Arg_String and
	   int (*)(char *) for Arg_Callback,
	   NULL or char ** for Arg_End, must be NULL for nested Arg_End */
	void *data;

	char *help; /* NULL for Arg_End and Arg_Include */
	char *param_name; /* NULL for Arg_Simple and Arg_Include */
};

/* defined in the main program */
extern struct arg_option argv_opts[];

/* valid after process_args() was called */
extern char *program_name;


void process_args(int *argc, char **argv);
