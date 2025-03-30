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
** Datei: motif_fenster.c
**
** Kommentar:
**  Fensterverwaltung auf Basis von Motif
*/


#include <stdio.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/BulletinB.h>
#include <Xm/CascadeBG.h>
#include <Xm/DialogS.h>
#include <Xm/DrawingA.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>

#include "argv.h"
#include "grafik.h"
#include "signale.h"
#include "ereignisse.h"
#include "einaus.h"
#include "client.h"
#include "speicher.h"
#include "system.h"
#include "X_grafik.h"
#include "X_tasten.h"
#include "X_daten.h"
#include "X_icon.h"
#include "Xt_appdefs.h"
#include "Xt_fenster.h"

static char sccsid[] = "@(#)motif_fenster.c	3.26 12/3/01";


#define STR_LENGTH 256

/* Pfusch! Irix doesn't know XmFONTLIST_DEFAULT_TAG */
#define DEFAULT_FONTLIST_TAG "XmFONTLIST_DEFAULT_TAG_STRING"


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


static void view_input(Widget widget, XtPointer client_data,
	XtPointer call_data)
{
	X_event_routine(XtDisplay(widget),
		((XmAnyCallbackStruct *)call_data)->event);
}


static Widget create_input_box(char *name, Widget parent_w)
{
	Widget form_w, label_w, text_w;

	form_w = XtCreateManagedWidget(name,
		xmFormWidgetClass, parent_w, NULL, 0);

	label_w = XtCreateManagedWidget("label",
		xmLabelWidgetClass, form_w, NULL, 0);

	text_w = XtCreateManagedWidget("input",
		xmTextFieldWidgetClass, form_w, NULL, 0);

	return text_w;
}


static Widget create_toggle_button(char *name, Widget parent_w)
{
	Widget form_w, toggle_w;

	form_w = XtCreateManagedWidget(name,
		xmFormWidgetClass, parent_w, NULL, 0);

	toggle_w = XmCreateToggleButton(form_w, "button", NULL, 0);
	XtManageChild(toggle_w);

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
			xmDrawingAreaWidgetClass, view->w, NULL, 0);
		view->window.display = XtDisplay(view->w);
		view->window.window = XtWindow(draw_area);
		xt_view_init(view);

		XtAddCallback(draw_area, XmNexposeCallback,
			xt_view_redraw, (XtPointer)view);
		XtAddCallback(draw_area, XmNresizeCallback,
			xt_view_resize, (XtPointer)view);
		XtAddCallback(draw_area, XmNinputCallback, view_input, NULL);

		view->is_initialized = 1;
	}
}


void xt_set_status_label(char *message)
{
	Arg arglist[1];

	XtSetArg(arglist[0], XmNlabelString,
		XmStringCreate(message, DEFAULT_FONTLIST_TAG));
	XtSetValues(status_w, arglist, 1);
}


void xt_set_option_sensitivity(int sensitive)
{
	Arg arglist[1];

	XtSetArg(arglist[0], XmNsensitive, sensitive);
	XtSetValues(connect_button_w, arglist, 1);

	XtSetArg(arglist[0], XmNeditable, sensitive);
	XtSetValues(xt_server_field_w, arglist, 1);
	XtSetValues(xt_message_field_w, arglist, 1);

	XtSetArg(arglist[0], XmNsensitive, sensitive);
	XtSetValues(xt_camera_toggle_w, arglist, 1);
}


char *xt_get_string_value(Widget w)
{
	Arg arglist[1];
	String str;

	XtSetArg(arglist[0], XmNvalue, &str);
	XtGetValues(w, arglist, 1);

	return str;
}


Boolean xt_get_toggle_value(Widget w)
{
	return XmToggleButtonGetState(w);
}


void xt_fill_main(Widget main_w)
{
	Widget form_w, menubar_w, menu_w;
	Arg arglist[2];
	char *field_value, *default_server;
	int override;

	form_w = XtCreateManagedWidget("form",
		xmFormWidgetClass, main_w, NULL, 0);

	menubar_w = XmCreateMenuBar(form_w, "menuBar", arglist, 0);
	XtManageChild(menubar_w);

	menu_w = XmCreatePulldownMenu(menubar_w, "menu", NULL, 0);

	XtAddCallback(XtCreateManagedWidget("exit",
		xmPushButtonGadgetClass, menu_w, NULL, 0),
		XmNactivateCallback, xt_exit_handler, NULL);

	XtSetArg(arglist[0], XmNsubMenuId, menu_w);
	XtCreateManagedWidget("fileMenu",
		xmCascadeButtonGadgetClass, menubar_w, arglist, 1);

	menu_w = XmCreatePulldownMenu(menubar_w, "menu", NULL, 0);

	XtAddCallback(XtCreateManagedWidget("frontView",
		xmPushButtonGadgetClass, menu_w, NULL, 0),
		XmNactivateCallback, xt_view_toggle, (XtPointer)&xt_frontview);

	XtAddCallback(XtCreateManagedWidget("rearView",
		xmPushButtonGadgetClass, menu_w, NULL, 0),
		XmNactivateCallback, xt_view_toggle, (XtPointer)&xt_rearview);

	XtAddCallback(XtCreateManagedWidget("map",
		xmPushButtonGadgetClass, menu_w, NULL, 0),
		XmNactivateCallback, xt_view_toggle, (XtPointer)&xt_mapview);

	XtAddCallback(XtCreateManagedWidget("compass",
		xmPushButtonGadgetClass, menu_w, NULL, 0),
		XmNactivateCallback, xt_view_toggle,
		(XtPointer)&xt_compassview);

	XtAddCallback(XtCreateManagedWidget("score",
		xmPushButtonGadgetClass, menu_w, NULL, 0),
		XmNactivateCallback, xt_view_toggle, (XtPointer)&xt_scoreview);

	XtSetArg(arglist[0], XmNsubMenuId, menu_w);
	XtCreateManagedWidget("toggleMenu",
		xmCascadeButtonGadgetClass, menubar_w, arglist, 1);

	menu_w = XmCreatePulldownMenu(menubar_w, "menu", NULL, 0);

	XtAddCallback(connect_button_w =
		XtCreateManagedWidget("connect",
			xmPushButtonGadgetClass, menu_w, NULL, 0),
		XmNactivateCallback, xt_connect_handler, NULL);

	XtSetArg(arglist[0], XtNsensitive, False);
	XtAddCallback(xt_disconnect_button_w =
		XtCreateManagedWidget("disconnect",
			xmPushButtonGadgetClass, menu_w, arglist, 1),
		XmNactivateCallback, xt_disconnect_handler, NULL);

	XtSetArg(arglist[0], XmNsubMenuId, menu_w);
	XtCreateManagedWidget("serverMenu",
		xmCascadeButtonGadgetClass, menubar_w, arglist, 1);

	menu_w = XmCreatePulldownMenu(menubar_w, "menu", NULL, 0);

	XtSetArg(arglist[0], XtNsensitive, False);
	XtAddCallback(xt_pause_button_w =
		XtCreateManagedWidget("pause",
			xmPushButtonGadgetClass, menu_w, arglist, 1),
		XmNactivateCallback, xt_pause_handler, NULL);

	XtSetArg(arglist[0], XtNsensitive, False);
	XtAddCallback(xt_resume_button_w =
		XtCreateManagedWidget("resume",
			xmPushButtonGadgetClass, menu_w, arglist, 1),
		XmNactivateCallback, xt_resume_handler, NULL);

	XtSetArg(arglist[0], XmNsubMenuId, menu_w);
	XtCreateManagedWidget("gameMenu",
		xmCascadeButtonGadgetClass, menubar_w, arglist, 1);

	xt_server_field_w = create_input_box("server", form_w);
	xt_message_field_w = create_input_box("message", form_w);
	xt_camera_toggle_w = create_toggle_button("camera", form_w);

	XtSetArg(arglist[0], XmNmaxLength, STR_LENGTH - 1);
	XtSetValues(xt_server_field_w, arglist, 1);
	XtSetValues(xt_message_field_w, arglist, 1);

	XtSetArg(arglist[0], XmNvalue, &field_value);
	XtGetValues(xt_server_field_w, arglist, 1);

	default_server = get_server_name(&override);
	if (!field_value[0] || override)
	{
		XtSetArg(arglist[0], XmNvalue, default_server);
		XtSetValues(xt_server_field_w, arglist, 1);
	}

	XtSetArg(arglist[0], XmNvalue, &field_value);
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

		XtSetArg(arglist[0], XmNvalue, message);
		XtSetValues(xt_message_field_w, arglist, 1);

		speicher_freigeben((void **)&message);
	}

	status_w = XtCreateManagedWidget("status",
		xmLabelWidgetClass, form_w, NULL, 0);
	xt_set_status_label("Not connected");
}


int xt_request_window(char **message, char *label1, char *label2,
	int two_buttons, char *widget_name)
{
	Widget w, q_w, form_w, sep_w, ok_w, cancel_w;
	Widget line_w;
	Arg arglist[3];
	int result_ok = 1, result_cancel = 2;

	w = XmCreateDialogShell(xt_toplevel, "dialog", xt_icon_arglist,
		sizeof xt_icon_arglist / sizeof *xt_icon_arglist);

	q_w = XtCreateWidget(widget_name, xmBulletinBoardWidgetClass,
		w, NULL, 0);

	form_w = XtCreateManagedWidget("form", xmFormWidgetClass,
		q_w, NULL, 0);

	line_w = NULL;
	while (*message != NULL)
	{
		XtSetArg(arglist[0], XmNlabelString,
			XmStringCreate(*message, DEFAULT_FONTLIST_TAG));
		XtSetArg(arglist[1], XmNtopAttachment, XmATTACH_WIDGET);
		XtSetArg(arglist[2], XmNtopWidget, line_w);

		line_w = XtCreateManagedWidget(line_w == NULL ?
			"text1" : "text", xmLabelWidgetClass,
			form_w, arglist, 3);

		message++;
	}

	XtSetArg(arglist[0], XmNtopAttachment, XmATTACH_WIDGET);
	XtSetArg(arglist[1], XmNtopWidget, line_w);
	sep_w = XtCreateManagedWidget("separator", xmSeparatorWidgetClass,
		form_w, arglist, 2);

	ok_w = XtCreateManagedWidget("default", xmPushButtonWidgetClass,
		form_w, NULL, 0);
	if (label1 != NULL)
	{
		XtSetArg(arglist[0], XmNlabelString,
			XmStringCreate(label1, DEFAULT_FONTLIST_TAG));
		XtSetValues(ok_w, arglist, 1);
	}

	if (two_buttons)
	{
		cancel_w = XtCreateManagedWidget("cancel",
			xmPushButtonWidgetClass, form_w, NULL, 0);
		if (label2 != NULL)
		{
			XtSetArg(arglist[0], XmNlabelString,
				XmStringCreate(label2, DEFAULT_FONTLIST_TAG));
			XtSetValues(cancel_w, arglist, 1);
		}

		XtCreateManagedWidget("space", xmPrimitiveWidgetClass,
			form_w, NULL, 0);
	}

	XtSetArg(arglist[0], XmNdefaultButton, ok_w);
	XtSetValues(q_w, arglist, 1);

	if (two_buttons)
	{
		XtSetArg(arglist[0], XmNcancelButton, cancel_w);
		XtSetValues(q_w, arglist, 1);
	}

	XtManageChild(q_w);
	XtRealizeWidget(w);

	if (!two_buttons)
	{
		Dimension form_width, button_width;

		XtSetArg(arglist[0], XmNwidth, &form_width);
		XtGetValues(form_w, arglist, 1);
		XtSetArg(arglist[0], XmNwidth, &button_width);
		XtGetValues(ok_w, arglist, 1);

		XtSetArg(arglist[0], XmNleftOffset,
			((int)form_width - (int)button_width) / 2);
		XtSetValues(ok_w, arglist, 1);
	}

	XtManageChild(w);

	XtAddCallback(ok_w, XmNactivateCallback,
		request_handler, (XtPointer)&result_ok);

	if (two_buttons)
		XtAddCallback(cancel_w, XmNactivateCallback,
			request_handler, (XtPointer)&result_cancel);

	XtAddCallback(w, XtNdestroyCallback, request_handler,
		two_buttons ? (XtPointer)&result_cancel :
			(XtPointer)&result_ok);

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
