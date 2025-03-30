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
** Datei: X_farben.c
**
** Kommentar:
**  Definition der Default-Farben fuer X11
*/


#include "farben.h"
#include "X_farben.h"

static char sccsid[] = "@(#)X_farben.c	3.1 12/3/01";


char *defaultfarben[] =
{
	/* Farbe fuer Umrandungen */
	"black",

	/* Farbe fuer Himmel */
	"white",

	/* Farbe fuer Boden */
	"lightgrey",

	/* Farbe fuer Schatten */
	"grey",

	/* Farbe fuer Kugel-Raender */
	"black",

	/* Kompassfarbe 1 */
	"grey76",

	/* Kompassfarbe 2 */
	"grey60",

	/* Farbe fuer Kompasszeiger */
	"lightskyblue",

	/* WANDFARBENANZ Farben fuer Waende */
	"grey44",
	"grey52",
	"grey60",
	"grey68",
	"grey76",
	"grey84",
	"grey92",
	"lightpink",
	"darkseagreen",
	"lightskyblue",
	"gold",
	"lightcyan",
	"lavenderblush",
	"mediumorchid",
	"burlywood",

	/* KUGELFARBENANZ Farben fuer die Spieler und deren Schuesse */
	"red",
	"green",
	"blue",
	"yellow",
	"cyan",
	"magenta",
	"darkorange",
	"brown",
	"lightpink",
	"darkseagreen",
	"lightskyblue",
	"gold",
	"lightcyan",
	"lavenderblush",
	"mediumorchid",
	"burlywood",
	"red",
	"green",
	"blue",
	"yellow",
	"cyan",
	"magenta",
	"darkorange",
	"brown",
	"lightpink",
	"darkseagreen",
	"lightskyblue",
	"gold",
	"lightcyan",
	"lavenderblush",
	"mediumorchid",
	"burlywood",
	"red",
	"green",
	"blue",
	"yellow",
	"cyan",
	"magenta",
	"darkorange",
	"brown",
	"lightpink",
	"darkseagreen",

	/* KUGELFARBENANZ Farben fuer Augen und Mund der Spieler +
	   eine Farbe fuer TRANSPARENT */
	"black",
	"black",
	"black",
	"black",
	"black",
	"black",
	"black",
	"black",
	"black",
	"black",
	"black",
	"black",
	"black",
	"black",
	"black",
	"black",
	"black",
	"mediumblue",
	"mediumblue",
	"mediumblue",
	"mediumblue",
	"mediumblue",
	"mediumblue",
	"mediumblue",
	"mediumblue",
	"mediumblue",
	"mediumblue",
	"mediumblue",
	"mediumblue",
	"mediumblue",
	"mediumblue",
	"mediumblue",
	"mediumblue",
	"maroon",
	"maroon",
	"maroon",
	"maroon",
	"maroon",
	"maroon",
	"maroon",
	"maroon",
	"maroon",
	"maroon"
};

unsigned char sw_raster[][8] =
{
	{ 0xff, 0xaa, 0xff, 0xaa, 0xff, 0xaa, 0xff, 0xaa },
	{ 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55 },
	{ 0x55, 0x00, 0x55, 0x00, 0x55, 0x00, 0x55, 0x00 },
	{ 0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff },
	{ 0x00, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00 },
	{ 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22 },
	{ 0x22, 0xff, 0x22, 0x22, 0x22, 0xff, 0x22, 0x22 },
	{ 0x22, 0x44, 0x88, 0x11, 0x22, 0x44, 0x88, 0x11 },
	{ 0x88, 0x44, 0x22, 0x11, 0x88, 0x44, 0x22, 0x11 }
};

int sw_defaultfarben[] =
{
	/* Farbe fuer Umrandungen */
	SCHWARZ,

	/* Farbe fuer Himmel */
	WEISS,

	/* Farbe fuer Boden */
	RASTER + 2,

	/* Farbe fuer Schatten */
	RASTER + 1,

	/* Farbe fuer Kugel-Raender */
	SCHWARZ,

	/* Kompassfarbe 1 */
	RASTER + 1,

	/* Kompassfarbe 2 */
	RASTER + 2,

	/* Farbe fuer Kompasszeiger */
	WEISS,

	/* WANDFARBENANZ Farben fuer Waende */
	RASTER + 1,  /* Farbe fuer Wand */
	SCHWARZ,     /* Farbe fuer Tuer */
	WEISS,       /* ab hier ungenutzt */
	WEISS,
	WEISS,
	WEISS,
	WEISS,
	WEISS,
	WEISS,
	WEISS,
	WEISS,
	WEISS,
	WEISS,
	WEISS,
	WEISS,

	/* KUGELFARBENANZ Farben fuer die Spieler und deren Schuesse */
	RASTER,
	RASTER + 2,
	RASTER + 3,
	RASTER + 4,
	RASTER + 5,
	RASTER + 6,
	RASTER + 7,
	RASTER + 8,
	RASTER,
	RASTER + 2,
	RASTER + 3,
	RASTER + 4,
	RASTER + 5,
	RASTER + 6,
	RASTER + 7,
	RASTER + 8,
	RASTER,
	RASTER + 2,
	RASTER + 3,
	RASTER + 4,
	RASTER + 5,
	RASTER + 6,
	RASTER + 7,
	RASTER + 8,
	RASTER,
	RASTER + 2,
	RASTER + 3,
	RASTER + 4,
	RASTER + 5,
	RASTER + 6,
	RASTER + 7,
	RASTER + 8,
	RASTER,
	RASTER + 2,
	RASTER + 3,
	RASTER + 4,
	RASTER + 5,
	RASTER + 6,
	RASTER + 7,
	RASTER + 8,
	RASTER,
	RASTER + 2,

	/* KUGELFARBENANZ Farben fuer Augen und Mund der Spieler +
	   eine Farbe fuer TRANSPARENT */
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ,
	SCHWARZ
};
