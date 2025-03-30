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
** File: argv.c
**
** Comment:
**  Parses command line parameters
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "argv.h"
#include "speicher.h"

static char sccsid[] = "@(#)argv.c	3.9 12/3/01";


#define OPT_LENGTH 20


static int want_help = 0;
static struct arg_option all_opts[] =
{
	{ Arg_Simple, "h", &want_help, "help" },
	{ Arg_Include, NULL, argv_opts },
	{ Arg_End }
};


char *program_name = "(unknown)";


/* prints enough spaces, help (if not NULL) and a newline */
static void print_help(int opt_length, char *help)
{
	char help_format[20];

	if (help == NULL)
		fprintf(stderr, "\n");
	else if (opt_length < OPT_LENGTH)
	{
		sprintf(help_format, "%%%ds%%s\n", OPT_LENGTH - opt_length);
		fprintf(stderr, help_format, " ", help);
	}
	else
		fprintf(stderr, " %s\n", help);
}


static void print_opts(struct arg_option *opts)
{
	char *param_name;

	for (;;)
	{
		switch (opts->type)
		{
		case Arg_End:
			return;

		case Arg_Include:
			print_opts(opts->data);
			break;

		case Arg_Simple:
			fprintf(stderr, "\t-%s", opts->name);
			print_help(2 + strlen(opts->name), opts->help);
			break;

		case Arg_String:
		case Arg_Callback:
			param_name =
				opts->param_name == NULL ? "(parameter)" :
					opts->param_name;
			fprintf(stderr, "\t-%s %s", opts->name, param_name);
			print_help(2 + strlen(opts->name) + 1
				+ strlen(param_name), opts->help);
			break;

		default:
			abort();
		}

		opts++;
	}
}


static struct arg_option *get_arg_end(void)
{
	struct arg_option *opt;

	for (opt = argv_opts; opt->type != Arg_End; opt++);
	return opt;
}


static void usage(void)
{
	struct arg_option *opt;

	opt = get_arg_end();

	fprintf(stderr, "usage: %s [options]", program_name);
	if (opt->data != NULL)
		fprintf(stderr, " %s",
			opt->param_name == NULL ? "(parameter)" :
				opt->param_name);
	fprintf(stderr, "\n\tvalid options:\n");
	print_opts(all_opts);
	exit(1);
}


static struct arg_option *match_option(struct arg_option *opts, char *name)
{
	for (;;)
	{
		switch (opts->type)
		{
		case Arg_End:
			return NULL;

		case Arg_Include:
			{
				struct arg_option *result;

				result = match_option(opts->data, name);
				if (result != NULL)
					return result;
			}
			break;

		case Arg_Simple:
		case Arg_String:
		case Arg_Callback:
			if (!strcmp(opts->name, name))
				return opts;
			break;

		default:
			abort();
		}

		opts++;
	}
}


/* up to here local part                       */
/***********************************************/
/* from here on global part                    */


void process_args(int *argc, char **argv)
{
	int arg_i, ofs;
	char *str;
	struct arg_option *opt;

	if (argv[0] != NULL)
	{
		program_name = strrchr(argv[0], '/');
		if (program_name == NULL || program_name[1] == 0)
			program_name = argv[0];
		else
			program_name++;
	}

	arg_i = 1;
	ofs = 0;

	while (arg_i < *argc)
	{
		char name[2];

		if (want_help)
			usage();

		if (ofs == 0)
		{
			if (argv[arg_i][0] != '-')
				break;

			if (!strcmp(argv[arg_i], "-") ||
				!strcmp(argv[arg_i], "--"))
			{
				arg_i++;
				break;
			}

			ofs = 1;
		}

		if (argv[arg_i][ofs] == 0)
		{
			arg_i++;
			ofs = 0;
			continue;
		}

		/* only single char options for now */
		name[0] = argv[arg_i][ofs];
		name[1] = 0;
		opt = match_option(all_opts, name);
		if (opt == NULL)
		{
			fprintf(stderr, "%s: unknown option -%s\n\n",
				program_name, name);
			usage();
		}

		switch (opt->type)
		{
		case Arg_Simple:
			ofs++;
			*(int *)opt->data = 1;
			break; /* success */

		case Arg_String:
		case Arg_Callback:
			if (argv[arg_i][ofs + 1] == 0 && arg_i + 1 >= *argc)
			{
				fprintf(stderr,
					"%s: parameter missing for -%s\n\n",
					program_name, opt->name);
				usage();
			}

			ofs++;
			if (argv[arg_i][ofs] == 0)
			{
				arg_i++;
				ofs = 0;
			}

			str = argv[arg_i] + ofs;
			arg_i++;
			ofs = 0;

			if (opt->type == Arg_String)
				*(char **)opt->data = str;
			else /* Arg_Callback */
			{
				int (*f)();

				f = (int (*)())opt->data;
				if (f(str))
				{
					fprintf(stderr,
						"%s: bad parameter for -%s: %s\n\n",
						program_name, opt->name, str);
					usage();
				}
			}

			break; /* success */

		default:
			abort();
		}
	}

	/* there can be no partial string left */
	if (ofs != 0)
		abort();

	opt = get_arg_end();
	if (opt->data == NULL)
	{
		if (arg_i < *argc)
		{
			fprintf(stderr, "%s: too many parameters\n\n",
				program_name);
			usage();
		}
	}
	else
	{
		if (arg_i >= *argc)
		{
			fprintf(stderr, "%s: parameter missing\n\n",
				program_name);
			usage();
		}
		else if (arg_i + 1 < *argc)
		{
			fprintf(stderr, "%s: too many parameters\n\n",
				program_name);
			usage();
		}

		*(char **)opt->data = argv[arg_i++];
	}

	*argc = *argc < 1 ? 0 : 1;
	argv[*argc] = NULL;
}
