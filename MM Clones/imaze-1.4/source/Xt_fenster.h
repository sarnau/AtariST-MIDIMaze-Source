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
** Datei: Xt_fenster.h
**
** Kommentar:
**  Interna von Xt_fenster.c, motif_fenster.c und athena_fenster.c
*/


static char sccsid_Xt_fenster[] = "@(#)Xt_fenster.h	3.7 12/3/01";


struct xt_view
{
	char *name;
	Widget w;
	void *X_data;
	struct fenster window;
	void *data;
	int is_open, is_initialized;
	int was_open;
	void (*init)(void **X_daten, struct fenster *fenster);
	void (*redraw)(void *X_daten, struct fenster *fenster, void *daten);
};


extern XtAppContext xt_app_context;
extern Arg xt_icon_arglist[];

extern Widget xt_toplevel;

extern Widget xt_disconnect_button_w;
extern Widget xt_pause_button_w, xt_resume_button_w;
extern Widget xt_server_field_w, xt_message_field_w, xt_camera_toggle_w;

extern struct xt_view xt_frontview;
extern struct xt_view xt_rearview;
extern struct xt_view xt_compassview;
extern struct xt_view xt_scoreview;
extern struct xt_view xt_mapview;


/* in Xt_fenster.c */

void xt_view_init(struct xt_view *view);
void xt_view_resize(Widget widget, XtPointer client_data, XtPointer call_data);
void xt_view_redraw(Widget widget, XtPointer client_data, XtPointer call_data);
void xt_view_destroy(Widget widget, XtPointer client_data,
	XtPointer call_data);
void xt_view_toggle(Widget widget, XtPointer client_data, XtPointer call_data);
void xt_disconnect_handler(Widget widget, XtPointer client_data,
	XtPointer call_data);
void xt_connect_handler(Widget widget, XtPointer client_data,
	XtPointer call_data);
void xt_pause_handler(Widget widget, XtPointer client_data,
	XtPointer call_data);
void xt_resume_handler(Widget widget, XtPointer client_data,
	XtPointer call_data);
void xt_exit_handler(Widget widget, XtPointer client_data,
	XtPointer call_data);


/* in motif_fenster.c/athena_fenster.c */

void xt_popup_view(struct xt_view *view);
void xt_set_status_label(char *message);
void xt_set_option_sensitivity(int sensitive);
char *xt_get_string_value(Widget w);
Boolean xt_get_toggle_value(Widget w);
void xt_fill_main(Widget main_w);
int xt_request_window(char **message, char *label1, char *label2,
	int two_buttons, char *widget_name);
