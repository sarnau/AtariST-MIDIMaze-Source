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
#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>
#include <X11/Shell.h>
#include <X11/Xaw/Command.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>

#include "request.h"

static char sccsid[] = "@(#)request.c	3.1 12/3/01";


/*
**	external
*/


void open_request(Widget w, Widget button_w, char *new_name,
	XtCallbackProc yes_callback, XtPointer client_data)
{
	Widget popup_w, form_w, yes_w, no_w;
	Dimension width, height;
	Position x, y;
	static XtPopdownIDRec popdown_info;

	popup_w = XtVaCreatePopupShell(new_name, transientShellWidgetClass, w,
		NULL);

	form_w = XtVaCreateManagedWidget("popupForm", formWidgetClass, popup_w,
		NULL);

	XtVaCreateManagedWidget("label", labelWidgetClass, form_w,
		NULL);

	yes_w = XtVaCreateManagedWidget("yes", commandWidgetClass, form_w,
		NULL);

	no_w = XtVaCreateManagedWidget("no", commandWidgetClass, form_w,
		NULL);

	XtVaGetValues(button_w,
		XtNwidth, &width,
		XtNheight, &height,
		NULL);

	XtTranslateCoords(button_w, (Position)(width / 2),
		(Position)(height / 2), &x, &y);

	XtVaSetValues(popup_w,
		XtNx, x,
		XtNy, y,
		NULL);

	popdown_info.shell_widget = popup_w;
	popdown_info.enable_widget = button_w;

	XtAddCallback(yes_w, XtNcallback, XtCallbackPopdown,
		(XtPointer)&popdown_info);

	XtAddCallback(no_w, XtNcallback, XtCallbackPopdown,
		(XtPointer)&popdown_info);

	XtAddCallback(yes_w, XtNcallback, yes_callback, client_data);

	XtSetSensitive(button_w, False);
	XtPopup(popup_w, XtGrabNonexclusive);
}


void open_error_msg(Widget w, Widget button_w, char *new_name)
{
	Widget popup_w, form_w, confirm_w;
	Dimension width, height;
	Position x, y;
	static XtPopdownIDRec popdown_info;

	popup_w = XtVaCreatePopupShell(new_name, transientShellWidgetClass, w,
		NULL);

	form_w = XtVaCreateManagedWidget("popupForm", formWidgetClass, popup_w,
		NULL);

	XtVaCreateManagedWidget("label", labelWidgetClass, form_w,
		NULL);

	confirm_w = XtVaCreateManagedWidget("confirm",
			commandWidgetClass, form_w,
		NULL);

	XtVaGetValues(button_w,
		XtNwidth, &width,
		XtNheight, &height,
		NULL);

	XtTranslateCoords(button_w, (Position)(width / 2),
		(Position)(height / 2), &x, &y);

	XtVaSetValues(popup_w,
		XtNx, x,
		XtNy, y,
		NULL);

	popdown_info.shell_widget = popup_w;
	popdown_info.enable_widget = button_w;

	XtAddCallback(confirm_w, XtNcallback, XtCallbackPopdown,
		(XtPointer)&popdown_info);

	XtSetSensitive(button_w, False);
	XtPopup(popup_w, XtGrabNonexclusive);
}
