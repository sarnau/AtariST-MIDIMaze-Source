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
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>

#include "lab_edit_w.h"
#include "request.h"
#include "xlabed_appdefs.h"

static char sccsid[] = "@(#)xlabed.c	3.3 12/04/01";


/*
**	variables
*/

static char magic[] = "iMazeLab1\n";


static Widget name_w, comment_w, edit_w;
static Widget load_w, save_w, clear_w, quit_w;


/*
**	local
*/


static void load_file(char *name)
{
	FILE *f;
	char magic_buffer[sizeof magic];
	char comment[1024];
	u_char data[4];
	int width, height, x, y;
	Dimension old_width, old_height;

	LabEditClear(edit_w);

	XtVaSetValues(comment_w,
		XtNstring, "",
		NULL);

	if ((f = fopen(name, "r")) == NULL)
	{
		open_error_msg(edit_w, load_w, "errorLoadOpen");
		return;
	}

	if (fgets(magic_buffer, sizeof magic, f) == NULL ||
		strcmp(magic_buffer, magic) || fread(data, 1, 2, f) != 2)
	{
		fclose(f);
		open_error_msg(edit_w, load_w, "errorLoadFormat");
		return;
	}

	XtVaSetValues(name_w,
		XtNstring, name,
		NULL);

	width = data[0];
	height = data[1];

	XtVaGetValues(edit_w,
		XtNhCells, &old_width,
		XtNvCells, &old_height,
		NULL);

	XtVaSetValues(edit_w,
		XtNhCells, (Dimension)width,
		XtNvCells, (Dimension)height,
		NULL);

	for (y = 0; y < height; y++)
		for (x = 0; x < width; x++)
		{
			if (fread(data, 1, 4, f) != 4)
			{
				fclose(f);

				XtVaSetValues(edit_w,
					XtNhCells, old_width,
					XtNvCells, old_height,
					NULL);

				open_error_msg(edit_w, load_w,
					"errorLoadFormat");
				return;
			}

			LabEditSetWall(edit_w, x, y, NORTH, data[0] & 15);
			LabEditSetWall(edit_w, x, y, WEST, data[1] & 15);
			LabEditSetWall(edit_w, x, y, SOUTH, data[2] & 15);
			LabEditSetWall(edit_w, x, y, EAST, data[3] & 15);
		}

	if (fgets(comment, sizeof comment, f) == NULL)
		comment[0] = 0;

	fclose(f);

	XtVaSetValues(comment_w,
		XtNstring, comment,
		NULL);
}


static void do_load(Widget w, XtPointer client_data, XtPointer call_data)
{
	String name;

	XtVaGetValues(name_w,
		XtNstring, &name,
		NULL);

	if (!*name)
	{
		XBell(XtDisplay(w), 0);
		return;
	}

	load_file(name);
}


static void load_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
	String name;

	XtVaGetValues(name_w,
		XtNstring, &name,
		NULL);

	if (!*name)
	{
		XBell(XtDisplay(w), 0);
		return;
	}

	if (!LabEditIsModified(edit_w))
		do_load(w, (XtPointer)0, (XtPointer)0);
	else
		open_request(edit_w, w, "confirmLoad",
			do_load, (XtPointer)0);
}


static void save_file(char *name)
{
	FILE *f;
	String comment;
	u_char data[4];
	int x, y;
	Dimension width, height;

	XtVaGetValues(comment_w,
		XtNstring, &comment,
		NULL);

	XtVaGetValues(edit_w,
		XtNhCells, &width,
		XtNvCells, &height,
		NULL);

	if ((f = fopen(name, "w")) == NULL)
	{
		open_error_msg(edit_w, save_w, "errorSaveOpen");
		return;
	}

	data[0] = width;
	data[1] = height;

	if (fwrite(magic, 1, sizeof magic - 1, f) != sizeof magic - 1 ||
		fwrite(data, 1, 2, f) != 2)
	{
		fclose(f);
		open_error_msg(edit_w, save_w, "errorSaveWrite");
		return;
	}

	for (y = 0; y < height; y++)
		for (x = 0; x < width; x++)
		{
			int i;

			data[0] = LabEditGetWall(edit_w, x, y, NORTH);
			data[1] = LabEditGetWall(edit_w, x, y, WEST);
			data[2] = LabEditGetWall(edit_w, x, y, SOUTH);
			data[3] = LabEditGetWall(edit_w, x, y, EAST);

			for (i = 0; i < 4; i++)
				if (data[i] >= 8)
					data[i] |= 0x40; /* door */
				else if (data[i] > 0)
					data[i] |= 0xc0; /* wall */

			if (fwrite(data, 1, 4, f) != 4)
			{
				fclose(f);
				open_error_msg(edit_w, save_w,
					"errorSaveWrite");
				return;
			}
		}

	if (fwrite(comment, 1, strlen(comment), f) != strlen(comment))
	{
		fclose(f);
		open_error_msg(edit_w, save_w, "errorSaveWrite");
		return;
	}

	fclose(f);

	LabEditSetUnmodified(edit_w);
}


static void do_save(Widget w, XtPointer client_data, XtPointer call_data)
{
	String name;

	XtVaGetValues(name_w,
		XtNstring, &name,
		NULL);

	if (!*name)
	{
		XBell(XtDisplay(w), 0);
		return;
	}

	save_file(name);
}


static void save_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
	String name;
	struct stat stat_buffer;

	XtVaGetValues(name_w,
		XtNstring, &name,
		NULL);

	if (!*name)
	{
		XBell(XtDisplay(w), 0);
		return;
	}

	if (stat(name, &stat_buffer) < 0)
		do_save(w, (XtPointer)0, (XtPointer)0);
	else
		open_request(edit_w, w, "confirmOverwrite",
			do_save, (XtPointer)0);
}


static void do_clear(Widget w, XtPointer client_data, XtPointer call_data)
{
	XtVaSetValues(name_w,
		XtNstring, "",
		NULL);

	XtVaSetValues(comment_w,
		XtNstring, "",
		NULL);

	LabEditClear(edit_w);
}


static void clear_callback(Widget w, XtPointer client_data,
	XtPointer call_data)
{
	if (!LabEditIsModified(edit_w))
		do_clear((Widget)0, (XtPointer)0, (XtPointer)0);
	else
		open_request(edit_w, w, "confirmClear",
			do_clear, (XtPointer)0);
}


static void do_quit(Widget w, XtPointer client_data, XtPointer call_data)
{
	exit(0);
}


static void quit_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
	if (!LabEditIsModified(edit_w))
		do_quit((Widget)0, (XtPointer)0, (XtPointer)0);
	else
		open_request(edit_w, w, "quitClear",
			do_quit, (XtPointer)0);
}


/*
**	global
*/


int main(int argc, char **argv)
{
	static char override_newline[] =
		"#override \n\
		<Key>Return: \n\
		<Key>KP_Enter: \n\
		Ctrl<Key>m: \n\
		Ctrl<Key>j:";
	XtAppContext app_context;
	Widget top_w, form_w;
	XtTranslations newline_translations;

	top_w = XtVaAppInitialize(&app_context, "XLabEd",
		NULL, 0, &argc, argv, fallback_app_resources,
		NULL);

	form_w = XtVaCreateManagedWidget("form", formWidgetClass, top_w,
		NULL);

	load_w = XtVaCreateManagedWidget("load", commandWidgetClass, form_w,
		NULL);

	save_w = XtVaCreateManagedWidget("save", commandWidgetClass, form_w,
		NULL);

	clear_w = XtVaCreateManagedWidget("clear", commandWidgetClass, form_w,
		NULL);

	quit_w = XtVaCreateManagedWidget("quit", commandWidgetClass, form_w,
		NULL);

	XtVaCreateManagedWidget("fileNameLabel", labelWidgetClass, form_w,
		NULL);

	XtVaCreateManagedWidget("commentLabel", labelWidgetClass, form_w,
		NULL);

	name_w = XtVaCreateManagedWidget("fileName",
		asciiTextWidgetClass, form_w,
		XtNeditType, XawtextEdit,
		NULL);

	comment_w = XtVaCreateManagedWidget("comment",
		asciiTextWidgetClass, form_w,
		XtNeditType, XawtextEdit,
		NULL);

	newline_translations = XtParseTranslationTable(override_newline);
	XtOverrideTranslations(name_w, newline_translations);
	XtOverrideTranslations(comment_w, newline_translations);

	edit_w = XtVaCreateManagedWidget("editArea",
		labEditWidgetClass, form_w,
		NULL);

	XtAddCallback(load_w, XtNcallback, load_callback, (XtPointer)0);
	XtAddCallback(save_w, XtNcallback, save_callback, (XtPointer)0);
	XtAddCallback(clear_w, XtNcallback, clear_callback, (XtPointer)0);
	XtAddCallback(quit_w, XtNcallback, quit_callback, (XtPointer)0);

	XtRealizeWidget(top_w);

	if (argc > 1)
		load_file(argv[1]);

	XtAppMainLoop(app_context);

	return 0;
}
