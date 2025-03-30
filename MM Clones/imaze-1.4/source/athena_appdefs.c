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
** Datei: athena_appdefs.c
**
** Kommentar:
**  Application Defaults fuer Athena Widgets
*/


#include <X11/Intrinsic.h>

#include "Xt_appdefs.h"

static char sccsid[] = "@(#)athena_appdefs.c	3.7 12/08/01";


String fallback_app_resources[] =
{
	"IMaze.TransientShell.title: iMaze: Notice",
	"IMaze.TransientShell.iconName: iMaze: Notice",
	"IMaze*question*default.label: Continue",
	"IMaze*error*default.label: Bad Luck",
	"IMaze*question*cancel.label: Quit",

	"IMaze*foreground: black",
	"IMaze*background: grey75",
	"IMaze*topShadowColor: grey90",
	"IMaze*bottomShadowColor: grey40",
	"IMaze*highlightColor: black",

	"IMaze*FontList: -*-lucida-medium-r-*-*-12-*-*-*-p-*-*-*; -*-helvetica-medium-r-*-*-12-*-*-*-p-*-*-*:",

	"IMaze.TransientShell.form.Label.borderWidth: 0",
	"IMaze.TransientShell.form.Label.internalWidth: 0",
	"IMaze.TransientShell.form.Label.internalHeight: 0",
	"IMaze.TransientShell.form.text.vertDistance: 0",
	"IMaze.TransientShell.form.cancel.horizDistance: 10",

	"IMaze*fileMenuButton.label: File",
	"IMaze*fileMenu.exit.label: Exit",

	"IMaze*toggleMenuButton.label: Window",
	"IMaze*toggleMenuButton.fromHoriz: fileMenuButton",
	"IMaze*toggleMenu.frontView.label: Front View",
	"IMaze*toggleMenu.rearView.label: Rear View",
	"IMaze*toggleMenu.map.label: Map",
	"IMaze*toggleMenu.compass.label: Compass",
	"IMaze*toggleMenu.score.label: Score",

	"IMaze*serverMenuButton.label: Server",
	"IMaze*serverMenuButton.fromHoriz: toggleMenuButton",
	"IMaze*serverMenu.connect.label: Connect",
	"IMaze*serverMenu.disconnect.label: Disconnect",

	"IMaze*gameMenuButton.label: Game",
	"IMaze*gameMenuButton.fromHoriz: serverMenuButton",
	"IMaze*gameMenu.pause.label: Pause",
	"IMaze*gameMenu.resume.label: Resume",

	"IMaze*input.scrollHorizontal: always",
	"IMaze*input.bottomMargin: 16",

	"IMaze*form.Form.label.justify: left",
	"IMaze*form.Form.label.width: 40",
	"IMaze*form.Form.input.width: 150",
	"IMaze*form.Form.Label.borderWidth: 0",
	"IMaze*form.Form.Label.internalWidth: 0",
	"IMaze*form.Form.borderWidth: 0",
	"IMaze*form.Form.internalWidth: 0",
	"IMaze*form.Form.internalHeight: 0",
	"IMaze*form.Form.label.horizDistance: 0",
	"IMaze*form.Form.button.horizDistance: 0",

	"IMaze*form.Label.borderWidth: 0",
	"IMaze*form.Label.internalWidth: 0",

	"IMaze*form.status.borderWidth: 0",
	"IMaze*form.status.width: 200",
	"IMaze*form.status.justify: left",

	"IMaze*server.fromVert: fileMenuButton",
	"IMaze*server.label.label: Server:",
	"IMaze*server.input.fromHoriz: label",

	"IMaze*message.fromVert: server",
	"IMaze*message.label.label: Message:",
	"IMaze*message.input.fromHoriz: label",

	"IMaze*camera.fromVert: message",
	"IMaze*camera.button.label: Camera mode",

	"IMaze*status.fromVert: camera",

	"IMaze.title: iMaze - Version 1.4",
	"IMaze.iconName: iMaze",
	"IMaze.x: 8",
	"IMaze.y: 408",
	"IMaze.width: 336",

	"IMaze.frontView.title: iMaze: Front View",
	"IMaze.frontView.iconName: iMaze: Front View",
	"IMaze.frontView.x: 360",
	"IMaze.frontView.y: 304",
	"IMaze.frontView.width: 420",
	"IMaze.frontView.height: 280",

	"IMaze.rearView.title: iMaze: Rear View",
	"IMaze.rearView.iconName: iMaze: Rear View",
	"IMaze.rearView.x: 360",
	"IMaze.rearView.y: 32",
	"IMaze.rearView.width: 210",
	"IMaze.rearView.height: 140",

	"IMaze.map.title: iMaze: Map",
	"IMaze.map.iconName: iMaze: Map",
	"IMaze.map.x: 8",
	"IMaze.map.y: 32",
	"IMaze.map.width: 336",
	"IMaze.map.height: 336",

	"IMaze.compass.title: iMaze: Compass",
	"IMaze.compass.iconName: iMaze: Compass",
	"IMaze.compass.x: 586",
	"IMaze.compass.y: 32",
	"IMaze.compass.width: 114",
	"IMaze.compass.height: 114",

	"IMaze.score.title: iMaze: Score",
	"IMaze.score.iconName: iMaze: Score",
	"IMaze.score.x: 796",
	"IMaze.score.y: 304",
	"IMaze.score.width: 96",
	"IMaze.score.height: 302",

	NULL
};
