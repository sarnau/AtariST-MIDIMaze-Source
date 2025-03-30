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
** Datei: athena_fenster.c
**
** Kommentar:
**  Fensterverwaltung auf Basis der Athena Widgets
*/


#include <stdio.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>
#include <X11/Xaw/AsciiText.h>
#include <X11/Xaw/Form.h>
#include <X11/Xaw/Label.h>
#include <X11/Xaw/MenuButton.h>
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeBSB.h>
#include <X11/Xaw/Toggle.h>

#include "argv.h"
#include "grafik.h"
#include "signale.h"
#include "ereignisse.h"
#include "einaus.h"
#include "client.h"
#include "system.h"
#include "speicher.h"
#include "X_grafik.h"
#include "X_tasten.h"
#include "X_daten.h"
#include "X_icon.h"
#include "Xt_appdefs.h"
#include "Xt_fenster.h"

static char sccsid[] = "@(#)athena_fenster.c	3.18 12/06/01";


#define STR_LENGTH 256


XtAppContext xt_app_context;
Arg xt_icon_arglist[1];

Widget xt_toplevel;
static Widget status_w;
static Widget connect_button_w;
Widget xt_disconnect_button_w;
Widget xt_pause_button_w, xt_resume_button_w;
Widget xt_server_field_w, xt_message_field_w, xt_camera_toggle_w;

struct xt_view xt_frontview;
struct xt_view xt_rearview;
struct xt_view xt_compassview;
struct xt_view xt_scoreview;
struct xt_view xt_mapview;

static int request_result;


static void request_handler(Widget widget, XtPointer client_data,
	XtPointer call_data)
{
	request_result = *(int *)client_data;
}


static void view_redraw(Widget widget, XtPointer client_data, XEvent *event,
	Boolean *continue_to_dispatch)
{
	xt_view_redraw(widget, client_data, NULL);
	*continue_to_dispatch = True;
}


static void view_resize(Widget widget, XtPointer client_data, XEvent *event,
	Boolean *continue_to_dispatch)
{
	if (event->type == ConfigureNotify)
		xt_view_resize(widget, client_data, NULL);

	*continue_to_dispatch = True;
}


static void view_input(Widget widget, XtPointer client_data, XEvent *event,
	Boolean *continue_to_dispatch)
{
	X_event_routine(XtDisplay(widget), event);
	*continue_to_dispatch = True;
}


static Widget create_input_box(char *name, Widget parent_w)
{
	Widget form_w, label_w, text_w;

	form_w = XtCreateManagedWidget(name,
		formWidgetClass, parent_w, NULL, 0);

	label_w = XtCreateManagedWidget("label",
		labelWidgetClass, form_w, NULL, 0);

	text_w = XtVaCreateManagedWidget("input",
		asciiTextWidgetClass, form_w,
		XtNeditType, XawtextEdit,
		NULL);

	return text_w;
}


static Widget create_toggle_button(char *name, Widget parent_w)
{
	Widget form_w, toggle_w;

	form_w = XtCreateManagedWidget(name,
		formWidgetClass, parent_w, NULL, 0);

	toggle_w = XtCreateManagedWidget("button",
		toggleWidgetClass, form_w, NULL, 0);

	return toggle_w;
}


/* bis hier lokaler Teil                       */
/***********************************************/
/* ab hier globaler Teil                       */


void xt_popup_view(struct xt_view *view)
{
	if (view->is_open)
		return;

	if (!view->is_initialized)
	{
		view->w = XtCreatePopupShell(view->name,
			topLevelShellWidgetClass, xt_toplevel, xt_icon_arglist,
			sizeof xt_icon_arglist / sizeof *xt_icon_arglist);

		XtAddCallback(view->w, XtNdestroyCallback,
			xt_view_destroy, (XtPointer)view);
	}

	XtPopup(view->w, XtGrabNone);
	view->is_open = 1;

	if (!view->is_initialized)
	{
		Widget draw_area;

		draw_area = XtCreateManagedWidget("draw",
			widgetClass, view->w, NULL, 0);
		view->window.display = XtDisplay(view->w);
		view->window.window = XtWindow(draw_area);
		xt_view_init(view);

		XtAddEventHandler(draw_area, ExposureMask,
			False, view_redraw, (XtPointer)view);
		XtAddEventHandler(draw_area, StructureNotifyMask,
			False, view_resize, (XtPointer)view);
		XtAddEventHandler(draw_area, KeyPressMask | KeyReleaseMask |
			FocusChangeMask, False, view_input, NULL);

		view->is_initialized = 1;
	}
}


void xt_set_status_label(char *message)
{
	Arg arglist[1];

	XtSetArg(arglist[0], XtNlabel, message);
	XtSetValues(status_w, arglist, 1);
}


void xt_set_option_sensitivity(int sensitive)
{
	Arg arglist[1];

	XtSetArg(arglist[0], XtNsensitive, sensitive);
	XtSetValues(connect_button_w, arglist, 1);

	XtSetArg(arglist[0], XtNsensitive, sensitive);
	XtSetValues(xt_server_field_w, arglist, 1);
	XtSetValues(xt_message_field_w, arglist, 1);

	XtSetArg(arglist[0], XtNsensitive, sensitive);
	XtSetValues(xt_camera_toggle_w, arglist, 1);
}


char *xt_get_string_value(Widget w)
{
	Arg arglist[1];
	String str;

	XtSetArg(arglist[0], XtNstring, &str);
	XtGetValues(w, arglist, 1);

	return str;
}


Boolean xt_get_toggle_value(Widget w)
{
	Arg arglist[1];
	Boolean toggle;

	XtSetArg(arglist[0], XtNstate, &toggle);
	XtGetValues(w, arglist, 1);

	return toggle;
}


void xt_fill_main(Widget main_w)
{
	Widget form_w, menu_w;
	Arg arglist[2];
	char *field_value, *default_server;
	int override;

	form_w = XtCreateManagedWidget("form",
		formWidgetClass, main_w, NULL, 0);

	XtSetArg(arglist[0], XtNmenuName, "fileMenu");
	menu_w = XtCreateManagedWidget("fileMenuButton",
		menuButtonWidgetClass, form_w, arglist, 1);
	menu_w = XtCreateWidget("fileMenu",
		simpleMenuWidgetClass, xt_toplevel, NULL, 0);

	XtAddCallback(XtCreateManagedWidget("exit",
		smeBSBObjectClass, menu_w, NULL, 0),
		XtNcallback, xt_exit_handler, NULL);

	XtSetArg(arglist[0], XtNmenuName, "toggleMenu");
	menu_w = XtCreateManagedWidget("toggleMenuButton",
		menuButtonWidgetClass, form_w, arglist, 1);
	menu_w = XtCreateWidget("toggleMenu",
		simpleMenuWidgetClass, xt_toplevel, NULL, 0);

	XtAddCallback(XtCreateManagedWidget("frontView",
		smeBSBObjectClass, menu_w, NULL, 0),
		XtNcallback, xt_view_toggle, (XtPointer)&xt_frontview);

	XtAddCallback(XtCreateManagedWidget("rearView",
		smeBSBObjectClass, menu_w, NULL, 0),
		XtNcallback, xt_view_toggle, (XtPointer)&xt_rearview);

	XtAddCallback(XtCreateManagedWidget("map",
		smeBSBObjectClass, menu_w, NULL, 0),
		XtNcallback, xt_view_toggle, (XtPointer)&xt_mapview);

	XtAddCallback(XtCreateManagedWidget("compass",
		smeBSBObjectClass, menu_w, NULL, 0),
		XtNcallback, xt_view_toggle, (XtPointer)&xt_compassview);

	XtAddCallback(XtCreateManagedWidget("score",
		smeBSBObjectClass, menu_w, NULL, 0),
		XtNcallback, xt_view_toggle, (XtPointer)&xt_scoreview);

	XtSetArg(arglist[0], XtNmenuName, "serverMenu");
	menu_w = XtCreateManagedWidget("serverMenuButton",
		menuButtonWidgetClass, form_w, arglist, 1);
	menu_w = XtCreateWidget("serverMenu",
		simpleMenuWidgetClass, xt_toplevel, NULL, 0);

	XtAddCallback(connect_button_w =
		XtCreateManagedWidget("connect",
			smeBSBObjectClass, menu_w, NULL, 0),
		XtNcallback, xt_connect_handler, NULL);

	XtSetArg(arglist[0], XtNsensitive, False);
	XtAddCallback(xt_disconnect_button_w =
		XtCreateManagedWidget("disconnect",
			smeBSBObjectClass, menu_w, arglist, 1),
		XtNcallback, xt_disconnect_handler, NULL);

	XtSetArg(arglist[0], XtNmenuName, "gameMenu");
	menu_w = XtCreateManagedWidget("gameMenuButton",
		menuButtonWidgetClass, form_w, arglist, 1);
	menu_w = XtCreateWidget("gameMenu",
		simpleMenuWidgetClass, xt_toplevel, NULL, 0);

	XtSetArg(arglist[0], XtNsensitive, False);
	XtAddCallback(xt_pause_button_w =
		XtCreateManagedWidget("pause",
			smeBSBObjectClass, menu_w, arglist, 1),
		XtNcallback, xt_pause_handler, NULL);

	XtSetArg(arglist[0], XtNsensitive, False);
	XtAddCallback(xt_resume_button_w =
		XtCreateManagedWidget("resume",
			smeBSBObjectClass, menu_w, arglist, 1),
		XtNcallback, xt_resume_handler, NULL);

	xt_server_field_w = create_input_box("server", form_w);
	xt_message_field_w = create_input_box("message", form_w);
	xt_camera_toggle_w = create_toggle_button("camera", form_w);

	XtSetArg(arglist[0], XtNstring, &field_value);
	XtGetValues(xt_server_field_w, arglist, 1);

	default_server = get_server_name(&override);
	if (!field_value[0] || override)
	{
		XtSetArg(arglist[0], XtNstring, default_server);
		XtSetValues(xt_server_field_w, arglist, 1);
	}

	XtSetArg(arglist[0], XtNstring, &field_value);
	XtGetValues(xt_message_field_w, arglist, 1);

	if (!field_value[0])
	{
		char *message;

		if ((message = benutzer_name()) == NULL)
		{
			speicher_belegen((void **)&message, 10);
			strcpy(message, "Gotcha!");
		}
		else
		{
			speicher_vergroessern((void **)&message,
				strlen(message) + 20);
			strcat(message, " shouts: Gotcha!");
		}

		XtSetArg(arglist[0], XtNstring, message);
		XtSetValues(xt_message_field_w, arglist, 1);

		speicher_freigeben((void **)&message);
	}

	XtSetArg(arglist[0], XtNlabel, "Not connected");
	status_w = XtCreateManagedWidget("status",
		labelWidgetClass, form_w,
		arglist, 1);
}


int xt_request_window(char **message, char *label1, char *label2,
	int two_buttons, char *widget_name)
{
	Position x, y;
	Widget w, form_w, ok_w, cancel_w;
	Widget line_w;
	Arg arglist[5];
	int n;
	int result_ok = 1, result_cancel = 2;

	n = 0;
	XtSetArg(arglist[n], XtNx, &x); n++;
	XtSetArg(arglist[n], XtNy, &y); n++;
	XtGetValues(xt_toplevel, arglist, n);

	n = 0;
	XtSetArg(arglist[n], XtNx, x + 40); n++;
	XtSetArg(arglist[n], XtNy, y + 20); n++;
	w = XtCreatePopupShell(widget_name, transientShellWidgetClass,
		xt_toplevel, arglist, n);

	form_w = XtCreateManagedWidget("form", formWidgetClass,
		w, NULL, 0);

	line_w = NULL;
	while (*message != NULL)
	{
		n = 0;
		if ((*message)[0] == 0)
			XtSetArg(arglist[n], XtNlabel, " ");
		else
			XtSetArg(arglist[n], XtNlabel, *message);
		n++;
		if (line_w != NULL)
		{
			XtSetArg(arglist[n], XtNfromVert, line_w); n++;
		}

		line_w = XtCreateManagedWidget(line_w == NULL ?
			"text1" : "text", labelWidgetClass,
			form_w, arglist, n);

		message++;
	}

	n = 0;
	if (label1 != NULL)
	{
		XtSetArg(arglist[n], XtNlabel, label1); n++;
	}
	if (line_w != NULL)
	{
		XtSetArg(arglist[n], XtNfromVert, line_w); n++;
	}
	ok_w = XtCreateManagedWidget("default", commandWidgetClass,
		form_w, arglist, n);

	if (two_buttons)
	{
		n = 0;
		if (label2 != NULL)
		{
			XtSetArg(arglist[n], XtNlabel, label2); n++;
		}
		if (line_w != NULL)
		{
			XtSetArg(arglist[n], XtNfromVert, line_w); n++;
		}
		XtSetArg(arglist[n], XtNfromHoriz, ok_w); n++;
		cancel_w = XtCreateManagedWidget("cancel",
			commandWidgetClass, form_w, arglist, n);
	}

	XtAddCallback(ok_w, XtNcallback,
		request_handler, (XtPointer)&result_ok);

	if (two_buttons)
		XtAddCallback(cancel_w, XtNcallback,
			request_handler, (XtPointer)&result_cancel);

	XtAddCallback(w, XtNdestroyCallback, request_handler,
		two_buttons ? (XtPointer)&result_cancel :
			(XtPointer)&result_ok);

	XtPopup(w, XtGrabNonexclusive);

	request_result = 0;
	while (!request_result)
	{
		XEvent ev;

		XtAppNextEvent(xt_app_context, &ev);
		XtDispatchEvent(&ev);
	}

	XtRemoveCallback(w, XtNdestroyCallback, request_handler,
		two_buttons ? (XtPointer)&result_cancel :
			(XtPointer)&result_ok);
	XtDestroyWidget(w);
	return request_result;
}
