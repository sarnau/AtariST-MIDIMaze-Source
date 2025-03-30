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
** Datei: X_icon.c
**
** Kommentar:
**  Erzeugt eine Icon-Pixmap
*/


#include <stdio.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "X_icon.h"

static char sccsid[] = "@(#)X_icon.c	3.4 12/3/01";


#define W 48
#define H 48


static char *icon_data[H] =
{
"                                                ",
"                                                ",
"                    ++++++++                    ",
"                 +++        +++                 ",
"              +++              +++              ",
"             +                    +             ",
"           ++  +      +++          ++           ",
"          +   +++    ++++++          +          ",
"         +  +++++    +++++++          +         ",
"        +  ++++++   ++++++++           +        ",
"       +  +++++++   +++++++++           +       ",
"      +  ++++++++   +++++++++            +      ",
"      +  +++++++   ++++++++++            +      ",
"     +  ++++++++   +++++++++++            +     ",
"    +  +++++++++   +++++++++++             +    ",
"    +  ++++++++    +++++++++++             +    ",
"    +  ++++++++    +++++++++++             +    ",
"   +  +++++++++    +++++++++++              +   ",
"   +  ++++++++     +++++++++++              +   ",
"   +  +++++++      +++++++++++              +   ",
"  +   +++++++       +++++++++                +  ",
"  +   ++++++        +++++++++                +  ",
"  +    +++++         +++++++                 +  ",
"  +    ++++           +++++                  +  ",
"  +                                          +  ",
"  +                                          +  ",
"  +                                          +  ",
"  +                                          +  ",
"   +                                        +   ",
"   +                                        +   ",
"   +                                        +   ",
"    +                           ++         +    ",
"    +                           ++         +    ",
"    +                          ++          +    ",
"     + +                      +++         +     ",
"      +++                    +++         +      ",
"      + ++                  +++          +      ",
"       + ++                +++          +       ",
"        + +++            ++++          +        ",
"         + ++++       +++++           +         ",
"          +  ++++++++++++            +          ",
"           ++  ++++++++            ++           ",
"             +                    +             ",
"              +++              +++              ",
"                 +++        +++                 ",
"                    ++++++++                    ",
"                                                ",
"                                                ",
};


static Pixmap create_pixmap(Display *display, int screen, char **pixel_data,
	unsigned long black, unsigned long white, unsigned depth)
{
	Visual *visual;
	Pixmap pix;
	GC gc;
	XGCValues gc_values;
	XImage *image;
	unsigned char data[W * H / 8];
	int x, y;

	visual = DefaultVisual(display, screen);

	pix = XCreatePixmap(display, RootWindow(display, screen), W, H, depth);

	gc_values.foreground = black;
	gc_values.background = white;
	gc = XCreateGC(display, pix, GCForeground | GCBackground, &gc_values);

	image = XCreateImage(display, visual, 1, XYBitmap,
		0, (void *)data, W, H, 8, W / 8);

	for (y = 0; y < H; y++)
		for (x = 0; x < W; x++)
			XPutPixel(image, x, y, pixel_data[y][x] != ' ');

	XPutImage(display, pix, gc, image, 0, 0, 0, 0, W, H);
	XFree(image);
	XFreeGC(display, gc);

	return pix;
}


Pixmap X_create_icon_pixmap(Display *display, int screen)
{
	return create_pixmap(display, screen, icon_data,
		BlackPixel(display, screen), WhitePixel(display, screen),
		DefaultDepth(display, screen));
}
