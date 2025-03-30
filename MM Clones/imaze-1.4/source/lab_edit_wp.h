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

#ifndef lab_edit_wp_h
#define lab_edit_wp_h

#include "lab_edit_w.h"

static char sccsid_lab_edit_wp[] = "@(#)lab_edit_wp.h	3.1 12/3/01";


/* NEVER change these: */
#define N_WALL_COLORS 7
#define N_DOOR_COLORS 8


typedef struct
{
	int empty;
} LabEditClassPart;

typedef struct _LabEditClassRec
{
	CoreClassPart core_class;
	LabEditClassPart labEdit_class;
} LabEditClassRec;

extern LabEditClassRec labEditClassRec;

typedef struct
{
	Pixel foreground_pixel;
	Pixel wall_colors[N_WALL_COLORS];
	Pixel door_colors[N_DOOR_COLORS];
	Dimension h_cells, v_cells;

	GC foreground_gc;
	GC wall_gcs[N_WALL_COLORS];
	GC door_gcs[N_DOOR_COLORS];
	char (**walls)[4];
	int wall_x0, wall_y0, wall_is_vert;
	int wall_length, wall_pending, wall_color;
	int modified;
} LabEditPart;

typedef struct _LabEditRec
{
	CorePart core;
	LabEditPart labEdit;
} LabEditRec;

#endif
