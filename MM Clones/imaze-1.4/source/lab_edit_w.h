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

#ifndef lab_edit_w_h
#define lab_edit_w_h

#include <X11/Core.h>

static char sccsid_lab_edit_w[] = "@(#)lab_edit_w.h	3.1 12/3/01";


#define XtNwallColor1 "wallColor1"
#define XtNwallColor2 "wallColor2"
#define XtNwallColor3 "wallColor3"
#define XtNwallColor4 "wallColor4"
#define XtNwallColor5 "wallColor5"
#define XtNwallColor6 "wallColor6"
#define XtNwallColor7 "wallColor7"
#define XtNdoorColor1 "doorColor1"
#define XtNdoorColor2 "doorColor2"
#define XtNdoorColor3 "doorColor3"
#define XtNdoorColor4 "doorColor4"
#define XtNdoorColor5 "doorColor5"
#define XtNdoorColor6 "doorColor6"
#define XtNdoorColor7 "doorColor7"
#define XtNdoorColor8 "doorColor8"
#define XtNhCells "hCells"
#define XtNvCells "vCells"

#define XtCWallColor "WallColor"
#define XtCDoorColor "DoorColor"
#define XtCHCells "HCells"
#define XtCVCells "VCells"


extern WidgetClass labEditWidgetClass;

typedef struct _LabEditClassRec *LabEditWidgetClass;
typedef struct _LabEditRec *LabEditWidget;


#define NORTH	0
#define WEST	1
#define SOUTH	2
#define EAST	3


extern void LabEditSetWall(Widget arg_w, int x, int y, int dir, int value);
extern int LabEditGetWall(Widget arg_w, int x, int y, int dir);
extern void LabEditStartAddWall(Widget arg_w, XEvent *event, String *params,
	Cardinal *n_params);
extern void LabEditStartRemoveWall(Widget arg_w, XEvent *event, String *params,
	Cardinal *n_params);
extern void LabEditExtendWall(Widget arg_w, XEvent *event, String *params,
	Cardinal *n_params);
extern void LabEditFinishWall(Widget arg_w, XEvent *event, String *params,
	Cardinal *n_params);
extern void LabEditAbortWall(Widget arg_w, XEvent *event, String *params,
	Cardinal *n_params);
extern void LabEditToggleDoor(Widget arg_w, XEvent *event, String *params,
	Cardinal *n_params);
extern void LabEditClear(Widget arg_w);
extern void LabEditSetUnmodified(Widget arg_w);
extern int LabEditIsModified(Widget arg_w);

#endif
