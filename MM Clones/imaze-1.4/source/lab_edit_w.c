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
#include <X11/StringDefs.h>
#include <X11/IntrinsicP.h>

#include "lab_edit_wp.h"
#include "global.h"

static char sccsid[] = "@(#)lab_edit_w.c	3.3 12/04/01";


/*
**	macros
*/

#define DEFAULT_WALL 5

#define WALL_WIDTH 3
#define DOOR_CORNER_WIDTH (2 * WALL_WIDTH)

#define DOOR_COLOR1 (N_WALL_COLORS + 1)

#define IS_DOOR_COLOR(x) ((x) >= DOOR_COLOR1)


/*
**	local prototypes
*/

static void LabEditInit(Widget request_w, Widget new_w, ArgList arg_list,
	Cardinal *n_args);
static void LabEditDestroy(Widget arg_w);
static Boolean LabEditSetValues(Widget current_w, Widget request_w,
	Widget new_w, ArgList arg_list, Cardinal *n_args);
static void LabEditExpose(Widget arg_w, XEvent *event, Region arg_r);


/*
**	variables
*/

static XtResource resources[] =
{
	{
		XtNforeground,
		XtCForeground,
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(LabEditRec, labEdit.foreground_pixel),
		XtRString,
		XtDefaultForeground
	},
	{
		XtNwallColor1,
		XtCWallColor,
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(LabEditRec, labEdit.wall_colors[0]),
		XtRString,
		XtDefaultForeground
	},
	{
		XtNwallColor2,
		XtCWallColor,
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(LabEditRec, labEdit.wall_colors[1]),
		XtRString,
		XtDefaultForeground
	},
	{
		XtNwallColor3,
		XtCWallColor,
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(LabEditRec, labEdit.wall_colors[2]),
		XtRString,
		XtDefaultForeground
	},
	{
		XtNwallColor4,
		XtCWallColor,
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(LabEditRec, labEdit.wall_colors[3]),
		XtRString,
		XtDefaultForeground
	},
	{
		XtNwallColor5,
		XtCWallColor,
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(LabEditRec, labEdit.wall_colors[4]),
		XtRString,
		XtDefaultForeground
	},
	{
		XtNwallColor6,
		XtCWallColor,
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(LabEditRec, labEdit.wall_colors[5]),
		XtRString,
		XtDefaultForeground
	},
	{
		XtNwallColor7,
		XtCWallColor,
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(LabEditRec, labEdit.wall_colors[6]),
		XtRString,
		XtDefaultForeground
	},
	{
		XtNdoorColor1,
		XtCDoorColor,
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(LabEditRec, labEdit.door_colors[0]),
		XtRString,
		XtDefaultForeground
	},
	{
		XtNdoorColor2,
		XtCDoorColor,
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(LabEditRec, labEdit.door_colors[1]),
		XtRString,
		XtDefaultForeground
	},
	{
		XtNdoorColor3,
		XtCDoorColor,
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(LabEditRec, labEdit.door_colors[2]),
		XtRString,
		XtDefaultForeground
	},
	{
		XtNdoorColor4,
		XtCDoorColor,
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(LabEditRec, labEdit.door_colors[3]),
		XtRString,
		XtDefaultForeground
	},
	{
		XtNdoorColor5,
		XtCDoorColor,
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(LabEditRec, labEdit.door_colors[4]),
		XtRString,
		XtDefaultForeground
	},
	{
		XtNdoorColor6,
		XtCDoorColor,
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(LabEditRec, labEdit.door_colors[5]),
		XtRString,
		XtDefaultForeground
	},
	{
		XtNdoorColor7,
		XtCDoorColor,
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(LabEditRec, labEdit.door_colors[6]),
		XtRString,
		XtDefaultForeground
	},
	{
		XtNdoorColor8,
		XtCDoorColor,
		XtRPixel,
		sizeof(Pixel),
		XtOffsetOf(LabEditRec, labEdit.door_colors[7]),
		XtRString,
		XtDefaultForeground
	},
	{
		XtNhCells,
		XtCHCells,
		XtRDimension,
		sizeof(Dimension),
		XtOffsetOf(LabEditRec, labEdit.h_cells),
		XtRImmediate,
		(XtPointer)16
	},
	{
		XtNvCells,
		XtCVCells,
		XtRDimension,
		sizeof(Dimension),
		XtOffsetOf(LabEditRec, labEdit.v_cells),
		XtRImmediate,
		(XtPointer)16
	}
};

static XtActionsRec actions[] =
{
	{ "start_add_wall", LabEditStartAddWall },
	{ "start_remove_wall", LabEditStartRemoveWall },
	{ "extend_wall", LabEditExtendWall },
	{ "finish_wall", LabEditFinishWall },
	{ "abort_wall", LabEditAbortWall },
	{ "toggle_door", LabEditToggleDoor }
};

static char translations[] =
	"Button1<BtnDown>: abort_wall() \n\
	Button2<BtnDown>:  abort_wall() \n\
	Button3<BtnDown>:  abort_wall() \n\
	<Btn1Down>:        start_add_wall() \n\
	<Btn2Down>:        toggle_door() \n\
	<Btn3Down>:        start_remove_wall() \n\
	<Btn1Motion>:      extend_wall() \n\
	<Btn3Motion>:      extend_wall() \n\
	<Btn1Up>:          finish_wall() \n\
	<Btn3Up>:          finish_wall()";

LabEditClassRec labEditClassRec =
{
	/* core_class */
	{
	/* superclass */		(WidgetClass)&coreClassRec,
	/* class_name */		"LabEdit",
	/* widget_size */		sizeof(LabEditRec),
	/* class_initialize */		NULL,
	/* class_part_initialize */	NULL,
	/* class_inited */		FALSE,
	/* initialize */		LabEditInit,
	/* initialize_hook */		NULL,
	/* realize */			XtInheritRealize,
	/* actions */			actions,
	/* num_actions */		XtNumber(actions),
	/* resources */			resources,
	/* num_resources */		XtNumber(resources),
	/* xrm_class */			NULLQUARK,
	/* compress_motion */		TRUE,
	/* compress_exposure */		XtExposeCompressMultiple,
	/* compress_enterleave */	TRUE,
	/* visible_interest */		FALSE,
	/* destroy */			LabEditDestroy,
	/* resize */			NULL, /***/
	/* expose */			LabEditExpose,
	/* set_values */		LabEditSetValues,
	/* set_values_hook */		NULL,
	/* set_values_almost */		XtInheritSetValuesAlmost,
	/* get_values_hook */		NULL,
	/* accept_focus */		NULL, /***/
	/* version */			XtVersion,
	/* callback_private */		NULL,
	/* tm_table */			translations,
	/* query_geometry */		XtInheritQueryGeometry,
	/* display_accelerator */	XtInheritDisplayAccelerator,
	/* extension */			NULL
	},

	/* labEdit_class */
	{
	/* [none] */			0
	}
};

WidgetClass labEditWidgetClass = (WidgetClass)&labEditClassRec;


/*
**	local
*/


static int random_door_color(void)
{
	return zufall() % N_DOOR_COLORS + DOOR_COLOR1;
}


static void LabEditInit(Widget request_w, Widget new_w, ArgList arg_list,
	Cardinal *n_args)
{
	LabEditWidget w;
	XGCValues gc_set;
	int h, v, x, i;

	w = (LabEditWidget)new_w;

	w->labEdit.modified = 0;

	gc_set.foreground = w->labEdit.foreground_pixel;
	w->labEdit.foreground_gc = XtGetGC(new_w, GCForeground, &gc_set);

	for (i = 0; i < N_WALL_COLORS; i++)
	{
		gc_set.foreground = w->labEdit.wall_colors[i];
		w->labEdit.wall_gcs[i] = XtGetGC(new_w, GCForeground, &gc_set);
	}

	for (i = 0; i < N_DOOR_COLORS; i++)
	{
		gc_set.foreground = w->labEdit.door_colors[i];
		w->labEdit.door_gcs[i] = XtGetGC(new_w, GCForeground, &gc_set);
	}

	h = w->labEdit.h_cells;
	v = w->labEdit.v_cells;

	w->labEdit.walls = (void *)XtMalloc(h * sizeof *w->labEdit.walls);
	for (x = 0; x < h; x++)
		w->labEdit.walls[x] =
			(void *)XtMalloc(v * sizeof **w->labEdit.walls);

	LabEditClear(new_w);
}


static void LabEditDestroy(Widget arg_w)
{
	LabEditWidget w;
	int h, x, i;

	w = (LabEditWidget)arg_w;

	XtReleaseGC(arg_w, w->labEdit.foreground_gc);

	for (i = 0; i < N_WALL_COLORS; i++)
		XtReleaseGC(arg_w, w->labEdit.wall_gcs[i]);

	for (i = 0; i < N_DOOR_COLORS; i++)
		XtReleaseGC(arg_w, w->labEdit.door_gcs[i]);

	h = w->labEdit.h_cells;

	for (x = 0; x < h; x++)
		XtFree((char *)w->labEdit.walls[x]);
	XtFree((char *)w->labEdit.walls);
}


static Boolean LabEditSetValues(Widget current_w, Widget request_w,
	Widget new_w, ArgList arg_list, Cardinal *n_args)
{
	LabEditWidget w, old_w;
	int h, v, x;

	old_w = (LabEditWidget)current_w;
	w = (LabEditWidget)new_w;

	if (w->labEdit.h_cells == old_w->labEdit.h_cells &&
		w->labEdit.v_cells == old_w->labEdit.v_cells)
		return False;


	/* destroy old lab data */

	h = old_w->labEdit.h_cells;

	for (x = 0; x < h; x++)
		XtFree((char *)w->labEdit.walls[x]);
	XtFree((char *)w->labEdit.walls);


	/* create again with new size */

	h = w->labEdit.h_cells;
	v = w->labEdit.v_cells;

	w->labEdit.walls = (void *)XtMalloc(h * sizeof *w->labEdit.walls);
	for (x = 0; x < h; x++)
		w->labEdit.walls[x] =
			(void *)XtMalloc(v * sizeof **w->labEdit.walls);

	LabEditClear(new_w);


	return True;
}


static void draw_wall(LabEditWidget w, GC gc, int x1, int y1, int x2, int y2,
	int dir)
{
	static int dx[4] = { -1, 0, 1, 0 };
	static int dy[4] = { 0, 1, 0, -1 };
	XSegment s[WALL_WIDTH];
	int i;

	for (i = 0; i < WALL_WIDTH; i++)
	{
		s[i].x1 = x1 + i * (dy[dir] + dx[dir]) + (i ? 0 : dx[dir]);
		s[i].y1 = y1 + i * (-dx[dir] + dy[dir]) + (i ? 0 : dy[dir]);
		s[i].x2 = x2 + i * (dy[dir] - dx[dir]) - dx[dir];
		s[i].y2 = y2 + i * (-dx[dir] - dy[dir]) - dy[dir];
	}

	XDrawSegments(XtDisplay(w), XtWindow(w), gc, s, WALL_WIDTH);
}


static void draw_door(LabEditWidget w, GC gc, int x1, int y1, int x2, int y2,
	int dir)
{
	static int dx[4] = { -1, 0, 1, 0 };
	static int dy[4] = { 0, 1, 0, -1 };
	XSegment s[2 * WALL_WIDTH];
	int i;

	for (i = 0; i < WALL_WIDTH; i++)
	{
		s[i].x1 = x1 + i * dy[dir] + DOOR_CORNER_WIDTH * dx[dir];
		s[i].y1 = y1 - i * dx[dir] + DOOR_CORNER_WIDTH * dy[dir];
		s[i].x2 = x2 + i * dy[dir] - DOOR_CORNER_WIDTH * dx[dir];
		s[i].y2 = y2 - i * dx[dir] - DOOR_CORNER_WIDTH * dy[dir];
	}

	XDrawSegments(XtDisplay(w), XtWindow(w), gc, s, WALL_WIDTH);

	for (i = 0; i < WALL_WIDTH; i++)
	{
		s[i].x1 = x1 + i * (dy[dir] + dx[dir]) + (i ? 0 : dx[dir]);
		s[i].y1 = y1 + i * (-dx[dir] + dy[dir]) + (i ? 0 : dy[dir]);
		s[i].x2 = x1 + i * dy[dir] + (DOOR_CORNER_WIDTH - 1) * dx[dir];
		s[i].y2 = y1 - i * dx[dir] + (DOOR_CORNER_WIDTH - 1) * dy[dir];

		s[i + WALL_WIDTH].x1 = x2 + i * dy[dir] -
			(DOOR_CORNER_WIDTH - 1) * dx[dir];
		s[i + WALL_WIDTH].y1 = y2 - i * dx[dir] -
			(DOOR_CORNER_WIDTH - 1) * dy[dir];
		s[i + WALL_WIDTH].x2 = x2 + i * (dy[dir] - dx[dir]) - dx[dir];
		s[i + WALL_WIDTH].y2 = y2 + i * (-dx[dir] - dy[dir]) - dy[dir];
	}

	XDrawSegments(XtDisplay(w), XtWindow(w),
		w->labEdit.foreground_gc, s, 2 * WALL_WIDTH);
}


static void draw_wall_or_door(LabEditWidget w, int x1, int y1, int x2, int y2,
	int dir, int color)
{
	if (color >= 8)
		draw_door(w, w->labEdit.door_gcs[color - 8],
			x1, y1, x2, y2, dir);
	else if (color > 0)
		draw_wall(w, w->labEdit.wall_gcs[color - 1],
			x1, y1, x2, y2, dir);
}


static int calculate_wall_color(LabEditWidget w, int x, int y, int dir)
{
	int l, r;

	if (!w->labEdit.wall_pending ||
		!w->labEdit.wall_is_vert && dir != NORTH && dir != SOUTH ||
		w->labEdit.wall_is_vert && dir != WEST && dir != EAST ||
		dir == NORTH && y != w->labEdit.wall_y0 ||
		dir == SOUTH && y + 1 != w->labEdit.wall_y0 ||
		dir == WEST && x != w->labEdit.wall_x0 ||
		dir == EAST && x + 1 != w->labEdit.wall_x0)
		return w->labEdit.walls[x][y][dir];

	l = w->labEdit.wall_length;
	if (w->labEdit.wall_is_vert)
		r = y - w->labEdit.wall_y0;
	else
		r = x - w->labEdit.wall_x0;

	if (l < 0 && (r < l || r > 0) ||
		l >= 0 && (r > l || r < 0))
		return w->labEdit.walls[x][y][dir];
	else
		return w->labEdit.wall_color;
}


static void LabEditExpose(Widget arg_w, XEvent *event, Region arg_r)
{
	LabEditWidget w;
	int x, y, h, v, width, height;
	int x1, y1, x2, y2;

	w = (LabEditWidget)arg_w;
	if (!XtIsRealized(arg_w))
		return;

	h = w->labEdit.h_cells;
	v = w->labEdit.v_cells;

	width = w->core.width;
	height = w->core.height;

	for (x = 0; x < h; x++)
	{
		x1 = width * x / h;
		x2 = width * (x + 1) / h - 1;

		if (x1 >= event->xexpose.x + event->xexpose.width ||
			x2 < event->xexpose.x)
			continue;

		for (y = 0; y < v; y++)
		{
			XPoint p[4];

			y1 = height * y / v;
			y2 = height * (y + 1) / v - 1;

			if (y1 >= event->xexpose.y + event->xexpose.height ||
				y2 < event->xexpose.y)
				continue;

			p[0].x = x1;
			p[0].y = y1;
			p[1].x = x2;
			p[1].y = y1;
			p[2].x = x1;
			p[2].y = y2;
			p[3].x = x2;
			p[3].y = y2;

			XDrawPoints(XtDisplay(w), XtWindow(w),
				w->labEdit.foreground_gc,
				p, 4, CoordModeOrigin);

			draw_wall_or_door(w, x2, y1, x1, y1, NORTH,
				calculate_wall_color(w, x, y, NORTH));

			draw_wall_or_door(w, x1, y1, x1, y2, WEST,
				calculate_wall_color(w, x, y, WEST));

			draw_wall_or_door(w, x1, y2, x2, y2, SOUTH,
				calculate_wall_color(w, x, y, SOUTH));

			draw_wall_or_door(w, x2, y2, x2, y1, EAST,
				calculate_wall_color(w, x, y, EAST));
		}
	}
}


static void schedule_blocks(LabEditWidget w, int x, int y, int n_h, int n_v)
{
	int h, v, width, height;
	int x1, y1, x2, y2;

	h = w->labEdit.h_cells;
	v = w->labEdit.v_cells;

	width = w->core.width;
	height = w->core.height;

	x1 = width * x / h;
	x2 = width * (x + n_h) / h;
	y1 = height * y / v;
	y2 = height * (y + n_v) / v;

	XClearArea(XtDisplay(w), XtWindow(w), x1, y1, x2 - x1, y2 - y1, True);
}


static void refresh_pending_wall(LabEditWidget w)
{
	int dx, dy;
	int x, y, l;

	if (!w->labEdit.wall_pending)
		return;

	x = w->labEdit.wall_x0;
	y = w->labEdit.wall_y0;

	if (w->labEdit.wall_is_vert)
	{
		dx = 0;
		dy = 1;
	}
	else
	{
		dx = 1;
		dy = 0;
	}

	l = w->labEdit.wall_length;
	if (l < 0)
	{
		x += dx * w->labEdit.wall_length;
		y += dy * w->labEdit.wall_length;

		l = -l;
	}

	if (w->labEdit.wall_is_vert)
		schedule_blocks(w, x - 1, y, 2, l + 1);
	else
		schedule_blocks(w, x, y - 1, l + 1, 2);
}


static int find_wall(LabEditWidget w, XEvent *event, int *x_p, int *y_p,
	int *is_vert_p)
{
	int h, v, width, height;
	int a, b;

	if (!XtIsRealized((Widget)w))
		return 0;

	h = w->labEdit.h_cells;
	v = w->labEdit.v_cells;

	width = w->core.width;
	height = w->core.height;

	if (event->xbutton.x < 0 || event->xbutton.x >= width ||
		event->xbutton.y < 0 || event->xbutton.y >= height)
		return 0;

	a = (event->xbutton.x * h * height +
		(height - 1 - event->xbutton.y) * v * width) / width / height;
	b = (event->xbutton.x * h * height +
		event->xbutton.y * v * width) / width / height;

	if (*is_vert_p = (a + b) % 2)
	{
		/* vertical */

		*x_p = (a + b + 1 - v) / 2;
		*y_p = (b - a + v - 1) / 2;

		if (*x_p > 0 && *x_p < h && *y_p >= 0 && *y_p < v)
			return 1;
	}
	else
	{
		/* horizontal */

		*x_p = (a + b - v) / 2;
		*y_p = (b - a + v) / 2;

		if (*x_p >= 0 && *x_p < h && *y_p > 0 && *y_p < v)
			return 1;
	}

	return 0;
}


static void change_wall(LabEditWidget w, XEvent *event)
{
	w->labEdit.wall_length = 0;
	w->labEdit.wall_pending = find_wall(w, event, &w->labEdit.wall_x0,
		&w->labEdit.wall_y0, &w->labEdit.wall_is_vert);
}


/*
**	global
*/


void LabEditSetWall(Widget arg_w, int x, int y, int dir, int value)
{
	LabEditWidget w;

	w = (LabEditWidget)arg_w;

	if (x < 0 || x >= w->labEdit.h_cells ||
		y < 0 || y >= w->labEdit.v_cells ||
		dir < 0 || dir > 4)
		return;

	w->labEdit.walls[x][y][dir] = value;

	if (XtIsRealized(arg_w))
		schedule_blocks(w, x, y, 1, 1);
}


int LabEditGetWall(Widget arg_w, int x, int y, int dir)
{
	LabEditWidget w;

	w = (LabEditWidget)arg_w;

	if (x < 0 || x >= w->labEdit.h_cells ||
		y < 0 || y >= w->labEdit.v_cells ||
		dir < 0 || dir > 4)
		return 0; /* shouldn't happen */

	return w->labEdit.walls[x][y][dir];
}


void LabEditStartAddWall(Widget arg_w, XEvent *event, String *params,
	Cardinal *n_params)
{
	LabEditWidget w;

	w = (LabEditWidget)arg_w;

	LabEditAbortWall(arg_w, event, params, n_params);

	w->labEdit.wall_color = DEFAULT_WALL;
	change_wall(w, event);
	refresh_pending_wall(w);
}


void LabEditStartRemoveWall(Widget arg_w, XEvent *event, String *params,
	Cardinal *n_params)
{
	LabEditWidget w;

	w = (LabEditWidget)arg_w;

	LabEditAbortWall(arg_w, event, params, n_params);

	w->labEdit.wall_color = 0;
	change_wall(w, event);
	refresh_pending_wall(w);
}


void LabEditExtendWall(Widget arg_w, XEvent *event, String *params,
	Cardinal *n_params)
{
	LabEditWidget w;
	int h, v, width, height, old_l, new_l;
	int bx, by;

	w = (LabEditWidget)arg_w;

	if (!XtIsRealized(arg_w))
		return;

	if (!w->labEdit.wall_pending)
		return;

	h = w->labEdit.h_cells;
	v = w->labEdit.v_cells;

	width = w->core.width;
	height = w->core.height;

	old_l = w->labEdit.wall_length;

	if (event->xbutton.x < 0 || event->xbutton.x >= width ||
		event->xbutton.y < 0 || event->xbutton.y >= height)
		return;

	bx = event->xbutton.x * h / width;
	by = event->xbutton.y * v / height;

	if (w->labEdit.wall_is_vert)
		new_l = by - w->labEdit.wall_y0;
	else
		new_l = bx - w->labEdit.wall_x0;

	if (new_l == old_l)
		return;

	if (old_l > 0 && new_l < old_l || old_l < 0 && new_l > old_l)
		refresh_pending_wall(w);

	w->labEdit.wall_length = new_l;
	refresh_pending_wall(w);
}


void LabEditFinishWall(Widget arg_w, XEvent *event, String *params,
	Cardinal *n_params)
{
	LabEditWidget w;
	int x, y, l, dx, dy;
	int is_vert, color;

	w = (LabEditWidget)arg_w;

	if (!XtIsRealized(arg_w))
		return;

	if (!w->labEdit.wall_pending)
		return;

	x = w->labEdit.wall_x0;
	y = w->labEdit.wall_y0;
	l = w->labEdit.wall_length;
	color = w->labEdit.wall_color;

	if (is_vert = w->labEdit.wall_is_vert)
	{
		dx = 0;
		dy = 1;
	}
	else
	{
		dx = 1;
		dy = 0;
	}

	if (l < 0)
	{
		dx = -dx;
		dy = -dy;
		l = -l;
	}

	for (l++; l--; x += dx, y += dy)
		if (is_vert)
		{
			w->labEdit.walls[x][y][WEST] = color;
			w->labEdit.walls[x - 1][y][EAST] = color;
		}
		else
		{
			w->labEdit.walls[x][y][NORTH] = color;
			w->labEdit.walls[x][y - 1][SOUTH] = color;
		}

	refresh_pending_wall(w);
	w->labEdit.wall_pending = 0;
	w->labEdit.modified = 1;
}


void LabEditAbortWall(Widget arg_w, XEvent *event, String *params,
	Cardinal *n_params)
{
	LabEditWidget w;

	w = (LabEditWidget)arg_w;

	refresh_pending_wall(w);
	w->labEdit.wall_pending = 0;
}


void LabEditToggleDoor(Widget arg_w, XEvent *event, String *params,
	Cardinal *n_params)
{
	LabEditWidget w;
	int x, y, is_vert;
	char *wall1, *wall2;

	w = (LabEditWidget)arg_w;

	if (!XtIsRealized(arg_w))
		return;

	LabEditAbortWall(arg_w, event, params, n_params);

	if (!find_wall(w, event, &x, &y, &is_vert))
		return;

	if (is_vert)
	{
		wall1 = &w->labEdit.walls[x - 1][y][EAST];
		wall2 = &w->labEdit.walls[x][y][WEST];
	}
	else
	{
		wall1 = &w->labEdit.walls[x][y - 1][SOUTH];
		wall2 = &w->labEdit.walls[x][y][NORTH];
	}

	if (IS_DOOR_COLOR(*wall1))
		if (IS_DOOR_COLOR(*wall2))
		{
			*wall2 = DEFAULT_WALL;
			schedule_blocks(w, x, y, 1, 1);
			return;
		}
		else
		{
			*wall2 = random_door_color();
			*wall1 = DEFAULT_WALL;
		}
	else if (IS_DOOR_COLOR(*wall2))
		*wall1 = random_door_color();
	else
	{
		*wall1 = random_door_color();
		*wall2 = random_door_color();
	}

	if (is_vert)
		schedule_blocks(w, x - 1, y, 2, 1);
	else
		schedule_blocks(w, x, y - 1, 1, 2);

	w->labEdit.modified = 1;
}


void LabEditClear(Widget arg_w)
{
	LabEditWidget w;
	int h, v, x, y, i;

	w = (LabEditWidget)arg_w;

	w->labEdit.modified = 0;

	h = w->labEdit.h_cells;
	v = w->labEdit.v_cells;

	for (x = 0; x < h; x++)
	{

		for (y = 0; y < v; y++)
			for (i = 0; i < 4; i++)
				w->labEdit.walls[x][y][i] = 0;

		w->labEdit.walls[x][0][NORTH] = DEFAULT_WALL;
		w->labEdit.walls[x][v - 1][SOUTH] = DEFAULT_WALL;
	}

	for (y = 0; y < v; y++)
	{
		w->labEdit.walls[0][y][WEST] = DEFAULT_WALL;
		w->labEdit.walls[h - 1][y][EAST] = DEFAULT_WALL;
	}

	w->labEdit.wall_pending = 0;

	if (!XtIsRealized(arg_w))
		return;

	schedule_blocks(w, 0, 0, h, v);
}


void LabEditSetUnmodified(Widget arg_w)
{
	LabEditWidget w;

	w = (LabEditWidget)arg_w;

	w->labEdit.modified = 0;
}


int LabEditIsModified(Widget arg_w)
{
	LabEditWidget w;

	w = (LabEditWidget)arg_w;

	return w->labEdit.modified;
}
