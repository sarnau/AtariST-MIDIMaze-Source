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

#include <X11/Intrinsic.h>

#include "xlabed_appdefs.h"

static char sccsid[] = "@(#)xlabed_appdefs.c	3.2 12/04/01";


String fallback_app_resources[] =
{
	"XLabEd*wallColor1: grey44",
	"XLabEd*wallColor2: grey52",
	"XLabEd*wallColor3: grey60",
	"XLabEd*wallColor4: grey68",
	"XLabEd*wallColor5: grey76",
	"XLabEd*wallColor6: grey84",
	"XLabEd*wallColor7: grey92",

	"XLabEd*doorColor1: lightpink",
	"XLabEd*doorColor2: darkseagreen",
	"XLabEd*doorColor3: lightskyblue",
	"XLabEd*doorColor4: gold",
	"XLabEd*doorColor5: lightcyan",
	"XLabEd*doorColor6: lavenderblush",
	"XLabEd*doorColor7: mediumorchid",
	"XLabEd*doorColor8: burlywood",

	"XLabEd*save.fromHoriz: load",
	"XLabEd*clear.fromHoriz: save",
	"XLabEd*quit.fromHoriz: clear",

	"XLabEd*load.label: Load",
	"XLabEd*save.label: Save",
	"XLabEd*clear.label: Clear",
	"XLabEd*quit.label: Quit",

	"XLabEd*fileNameLabel.fromVert: load",
	"XLabEd*fileName.fromVert: load",
	"XLabEd*form.Label.borderWidth: 0",
	"XLabEd*form.Label.internalHeight: 3",
	"XLabEd*form.Text.width: 500",
	"XLabEd*form.Text.resizable: True",
	"XLabEd*form.Text.right: ChainRight",
	"XLabEd*form.Text.fromHoriz: commentLabel",
	"XLabEd*commentLabel.fromVert: fileNameLabel",
	"XLabEd*comment.fromVert: fileName",

	"XLabEd*editArea.fromVert: comment",

	"XLabEd*fileNameLabel*label: File:",
	"XLabEd*commentLabel*label: Comment:",

	"XLabEd*editArea.width: 600",
	"XLabEd*editArea.height: 600",
	"XLabEd*editArea.hCells: 12",
	"XLabEd*editArea.vCells: 12",
	"XLabEd*editArea.background: lightgrey",

	"XLabEd*popupForm.label.borderWidth: 0",
	"XLabEd*popupForm.Command.fromVert: label",
	"XLabEd*popupForm.no.fromHoriz: yes",
	"XLabEd*yes.label: Yes",
	"XLabEd*no.label: No",
	"XLabEd*confirm.label: OK",

	"XLabEd*confirmLoad.title: Discard?",
	"XLabEd*confirmClear.title: Discard?",
	"XLabEd*quitClear.title: Discard?",
	"XLabEd*confirmOverwrite.title: Overwrite?",

	"XLabEd*confirmLoad.popupForm.label.label: Discard changes?",
	"XLabEd*confirmClear.popupForm.label.label: Discard changes?",
	"XLabEd*quitClear.popupForm.label.label: Discard changes?",
	"XLabEd*confirmOverwrite.popupForm.label.label: Overwrite file?",

	"XLabEd*errorLoadOpen.title: Load error",
	"XLabEd*errorLoadOpen.popupForm.label.label: Can't open file",

	"XLabEd*errorLoadFormat.title: Load error",
	"XLabEd*errorLoadFormat.popupForm.label.label: File has invalid format",

	"XLabEd*errorSaveOpen.title: Save error",
	"XLabEd*errorSaveOpen.popupForm.label.label: Can't open file",

	"XLabEd*errorSaveWrite.title: Save error",
	"XLabEd*errorSaveWrite.popupForm.label.label: Can't write to file",

	NULL
};
