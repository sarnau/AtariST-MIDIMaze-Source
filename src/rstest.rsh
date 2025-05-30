/*
 * GEM resource C output of rstest
 *
 * created by ORCS 2.18
 */

#ifndef _LONG_PTR
#  define _LONG_PTR LONG
#endif

#ifndef OS_NORMAL
#  define OS_NORMAL 0x0000
#endif
#ifndef OS_SELECTED
#  define OS_SELECTED 0x0001
#endif
#ifndef OS_CROSSED
#  define OS_CROSSED 0x0002
#endif
#ifndef OS_CHECKED
#  define OS_CHECKED 0x0004
#endif
#ifndef OS_DISABLED
#  define OS_DISABLED 0x0008
#endif
#ifndef OS_OUTLINED
#  define OS_OUTLINED 0x0010
#endif
#ifndef OS_SHADOWED
#  define OS_SHADOWED 0x0020
#endif
#ifndef OS_WHITEBAK
#  define OS_WHITEBAK 0x0040
#endif
#ifndef OS_DRAW3D
#  define OS_DRAW3D 0x0080
#endif

#ifndef OF_NONE
#  define OF_NONE 0x0000
#endif
#ifndef OF_SELECTABLE
#  define OF_SELECTABLE 0x0001
#endif
#ifndef OF_DEFAULT
#  define OF_DEFAULT 0x0002
#endif
#ifndef OF_EXIT
#  define OF_EXIT 0x0004
#endif
#ifndef OF_EDITABLE
#  define OF_EDITABLE 0x0008
#endif
#ifndef OF_RBUTTON
#  define OF_RBUTTON 0x0010
#endif
#ifndef OF_LASTOB
#  define OF_LASTOB 0x0020
#endif
#ifndef OF_TOUCHEXIT
#  define OF_TOUCHEXIT 0x0040
#endif
#ifndef OF_HIDETREE
#  define OF_HIDETREE 0x0080
#endif
#ifndef OF_INDIRECT
#  define OF_INDIRECT 0x0100
#endif
#ifndef OF_FL3DIND
#  define OF_FL3DIND 0x0200
#endif
#ifndef OF_FL3DBAK
#  define OF_FL3DBAK 0x0400
#endif
#ifndef OF_FL3DACT
#  define OF_FL3DACT 0x0600
#endif
#ifndef OF_MOVEABLE
#  define OF_MOVEABLE 0x0800
#endif
#ifndef OF_POPUP
#  define OF_POPUP 0x1000
#endif

#ifndef G_SWBUTTON
#  define G_SWBUTTON 34
#endif
#ifndef G_POPUP
#  define G_POPUP 35
#endif
#ifndef G_EDIT
#  define G_EDIT 37
#endif
#ifndef G_SHORTCUT
#  define G_SHORTCUT 38
#endif
#ifndef G_SLIST
#  define G_SLIST 39
#endif
#ifndef G_EXTBOX
#  define G_EXTBOX 40
#endif
#ifndef G_OBLINK
#  define G_OBLINK 41
#endif

#ifndef WHITEBAK
#  define WHITEBAK OS_WHITEBAK
#endif
#ifndef DRAW3D
#  define DRAW3D OS_DRAW3D
#endif
#ifndef FL3DIND
#  define FL3DIND OF_FL3DIND
#endif
#ifndef FL3DBAK
#  define FL3DBAK OF_FL3DBAK
#endif
#ifndef FL3DACT
#  define FL3DACT OF_FL3DACT
#endif

#ifndef C_UNION
#ifdef __PORTAES_H__
#  define C_UNION(x) { (_LONG_PTR)(x) }
#endif
#ifdef __GEMLIB__
#  define C_UNION(x) { (_LONG_PTR)(x) }
#endif
#ifdef __PUREC__
#  define C_UNION(x) { (_LONG_PTR)(x) }
#endif
#ifdef __ALCYON__
#  define C_UNION(x) x
#endif
#endif
#ifndef C_UNION
#  define C_UNION(x) (_LONG_PTR)(x)
#endif

#define T0OBJ 0
#define T1OBJ 23
#define T2OBJ 27
#define T3OBJ 31
#define T4OBJ 34
#define T5OBJ 37
#define T6OBJ 40
#define T7OBJ 43
#define T8OBJ 46
#define T9OBJ 101
#define T10OBJ 204
#define T11OBJ 207
#define T12OBJ 210
#define T13OBJ 217
#define T14OBJ 220
#define FREEBB 0
#define FREEIMG 0
#define FREESTR 95

BYTE *rs_strings[] = {
	(BYTE *)" \016\017 ",
	(BYTE *)" MAZE ",
	(BYTE *)"  About MIDImaze...",
	(BYTE *)"---------------------",
	(BYTE *)"  1",
	(BYTE *)"  2",
	(BYTE *)"  3",
	(BYTE *)"  4",
	(BYTE *)"  5",
	(BYTE *)"  6",
	(BYTE *)"  Load...      ^O",
	(BYTE *)"  Reset score  ^R",
	(BYTE *)"------------------",
	(BYTE *)"  Play..!      ^P",
	(BYTE *)"  Names...     ^N",
	(BYTE *)"------------------",
	(BYTE *)"  Quit         ^Q",
	(BYTE *)"This is a",
	(BYTE *)"SLAVE",
	(BYTE *)"machine.",
	(BYTE *)"Boo-boo:",
	(BYTE *)"The MIDI ring",
	(BYTE *)"has timed out.",
	(BYTE *)"Game terminated by the",
	(BYTE *)"bozo on the master machine.",
	(BYTE *)"Guru bu-bu:",
	(BYTE *)"(maze too small?)",
	(BYTE *)"Mouse control",
	(BYTE *)"selected.",
	(BYTE *)"Joystick control",
	(BYTE *)"selected.",
	(BYTE *)"You are",
	(BYTE *)"Number __",
	(BYTE *)"Select Drones:",
	(BYTE *)"Very",
	(BYTE *)"Not so",
	(BYTE *)"Dumb",
	(BYTE *)" Dumb",
	(BYTE *)"Dumb",
	(BYTE *)" 1",
	(BYTE *)" 2",
	(BYTE *)" 3",
	(BYTE *)"Plain",
	(BYTE *)"Reload Time:",
	(BYTE *)"Fast",
	(BYTE *)"Slow",
	(BYTE *)"Regen Time:",
	(BYTE *)"Slow",
	(BYTE *)"Fast",
	(BYTE *)"Revive:",
	(BYTE *)"Slow",
	(BYTE *)"Fast",
	(BYTE *)"Revive with:",
	(BYTE *)"1 Life",
	(BYTE *)"2 Lives",
	(BYTE *)"3 Lives",
	(BYTE *)"00 Machines on-line",
	(BYTE *)"Nah",
	(BYTE *)"Yeah!",
	(BYTE *)"Singles",
	(BYTE *)"Teams",
	(BYTE *)"Player # 1",
	(BYTE *)"Player # 2",
	(BYTE *)"Player # 3",
	(BYTE *)"Player # 4",
	(BYTE *)"Player # 5",
	(BYTE *)"Player # 6",
	(BYTE *)"Player # 7",
	(BYTE *)"Player # 8",
	(BYTE *)"Player # 9",
	(BYTE *)"Player #10",
	(BYTE *)"Player #11",
	(BYTE *)"Player #12",
	(BYTE *)"Player #13",
	(BYTE *)"Player #14",
	(BYTE *)"Player #15",
	(BYTE *)"Player #16",
	(BYTE *)"O-Tay!",
	(BYTE *)"Friendly Fire",
	(BYTE *)"The game is",
	(BYTE *)"suspended.",
	(BYTE *)"Player #xx",
	(BYTE *)"Name: __________",
	(BYTE *)"n",
	(BYTE *)"OK",
	(BYTE *)"MIDIMAZE 1.0",
	(BYTE *)"Copyright (c) 1987",
	(BYTE *)"XANTH F/X",
	(BYTE *)"All rights reserved.",
	(BYTE *)"Wow!",
	(BYTE *)"Compiled by Alcyon",
	(BYTE *)"This is a",
	(BYTE *)"MIDICAM",
	(BYTE *)"Keyboard control",
	(BYTE *)"selected."
};

LONG rs_frstr[] = {
	0
};

BITBLK rs_bitblk[] = {
	{ 0, 0, 0, 0, 0, 0 }
};

LONG rs_frimg[] = {
	0
};

ICONBLK rs_iconblk[] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

TEDINFO rs_tedinfo[] = {
	{ (BYTE *)81L, (BYTE *)82L, (BYTE *)83L, 3, 6, 2, 0x1180, 0x0, -1, 11,17 }
};

OBJECT rs_object[] = {
	{ -1, 1, 5, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x0L), 0x0000,0x0000, 0x005a,0x0019 },
	{ 5, 2, 2, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0000,0x0000, 0x005a,0x0201 },
	{ 1, 3, 4, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x0L), 0x0002,0x0000, 0x000a,0x0301 },
	{ 4, -1, -1, G_TITLE, OF_NONE, OS_NORMAL, C_UNION(0x0L), 0x0000,0x0000, 0x0004,0x0301 },
	{ 2, -1, -1, G_TITLE, OF_NONE, OS_NORMAL, C_UNION(0x1L), 0x0004,0x0000, 0x0006,0x0301 },
	{ 0, 6, 15, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x0L), 0x0000,0x0301, 0x0050,0x0013 },
	{ 15, 7, 14, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0xFF1100L), 0x0002,0x0000, 0x0015,0x0008 },
	{ 8, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x2L), 0x0000,0x0000, 0x0015,0x0001 },
	{ 9, -1, -1, G_STRING, OF_NONE, OS_DISABLED, C_UNION(0x3L), 0x0000,0x0001, 0x0015,0x0001 },
	{ 10, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x4L), 0x0000,0x0002, 0x0015,0x0001 },
	{ 11, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x5L), 0x0000,0x0003, 0x0015,0x0001 },
	{ 12, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x6L), 0x0000,0x0004, 0x0015,0x0001 },
	{ 13, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x7L), 0x0000,0x0005, 0x0015,0x0001 },
	{ 14, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x8L), 0x0000,0x0006, 0x0015,0x0001 },
	{ 6, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x9L), 0x0000,0x0007, 0x0015,0x0001 },
	{ 5, 16, 22, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0xFF1100L), 0x0006,0x0000, 0x0012,0x0007 },
	{ 17, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0xAL), 0x0000,0x0000, 0x0012,0x0001 },
	{ 18, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0xBL), 0x0000,0x0001, 0x0012,0x0001 },
	{ 19, -1, -1, G_STRING, OF_NONE, OS_DISABLED, C_UNION(0xCL), 0x0000,0x0002, 0x0012,0x0001 },
	{ 20, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0xDL), 0x0000,0x0003, 0x0012,0x0001 },
	{ 21, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0xEL), 0x0000,0x0004, 0x0012,0x0001 },
	{ 22, -1, -1, G_STRING, OF_NONE, OS_DISABLED, C_UNION(0xFL), 0x0000,0x0005, 0x0012,0x0001 },
	{ 15, -1, -1, G_STRING, OF_LASTOB, OS_NORMAL, C_UNION(0x10L), 0x0000,0x0006, 0x0012,0x0001 },
	{ -1, 1, 3, G_BOX, OF_NONE, OS_OUTLINED, C_UNION(0x21100L), 0x0000,0x0000, 0x000c,0x0005 },
	{ 2, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x11L), 0x0001,0x0001, 0x0009,0x0001 },
	{ 3, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x12L), 0x0003,0x0002, 0x0005,0x0001 },
	{ 0, -1, -1, G_STRING, OF_LASTOB, OS_NORMAL, C_UNION(0x13L), 0x0002,0x0003, 0x0008,0x0001 },
	{ -1, 1, 3, G_BOX, OF_NONE, OS_OUTLINED, C_UNION(0x21100L), 0x0000,0x0000, 0x0011,0x0005 },
	{ 2, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x14L), 0x0002,0x0001, 0x0009,0x0001 },
	{ 3, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x15L), 0x0002,0x0002, 0x000d,0x0001 },
	{ 0, -1, -1, G_STRING, OF_LASTOB, OS_NORMAL, C_UNION(0x16L), 0x0002,0x0003, 0x000e,0x0001 },
	{ -1, 1, 2, G_BOX, OF_NONE, OS_OUTLINED, C_UNION(0x21100L), 0x0000,0x0000, 0x001d,0x0004 },
	{ 2, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x17L), 0x0003,0x0001, 0x0016,0x0001 },
	{ 0, -1, -1, G_STRING, OF_LASTOB, OS_NORMAL, C_UNION(0x18L), 0x0001,0x0002, 0x001b,0x0001 },
	{ -1, 1, 2, G_BOX, OF_NONE, OS_OUTLINED, C_UNION(0x21100L), 0x0000,0x0000, 0x0013,0x0004 },
	{ 2, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x19L), 0x0003,0x0001, 0x010b,0x0001 },
	{ 0, -1, -1, G_STRING, OF_LASTOB, OS_NORMAL, C_UNION(0x1AL), 0x0001,0x0002, 0x0311,0x0101 },
	{ -1, 1, 2, G_BOX, OF_NONE, OS_OUTLINED, C_UNION(0x21100L), 0x0000,0x0000, 0x000f,0x0004 },
	{ 2, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x1BL), 0x0001,0x0001, 0x030d,0x0001 },
	{ 0, -1, -1, G_STRING, OF_LASTOB, OS_NORMAL, C_UNION(0x1CL), 0x0003,0x0002, 0x0009,0x0001 },
	{ -1, 1, 2, G_BOX, OF_NONE, OS_OUTLINED, C_UNION(0x21100L), 0x0000,0x0000, 0x0012,0x0004 },
	{ 2, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x1DL), 0x0001,0x0001, 0x0410,0x0001 },
	{ 0, -1, -1, G_STRING, OF_LASTOB, OS_NORMAL, C_UNION(0x1EL), 0x0005,0x0002, 0x0009,0x0001 },
	{ -1, 1, 2, G_BOX, OF_NONE, OS_OUTLINED, C_UNION(0x21100L), 0x0000,0x0000, 0x000c,0x0004 },
	{ 2, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x1FL), 0x0002,0x0001, 0x0407,0x0001 },
	{ 0, -1, -1, G_STRING, OF_LASTOB, OS_NORMAL, C_UNION(0x20L), 0x0001,0x0002, 0x0009,0x0001 },
	{ -1, 1, 50, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0000,0x0000, 0x0626,0x0214 },
	{ 46, 2, 2, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0xFE2100L), 0x0000,0x0000, 0x001c,0x0214 },
	{ 1, 3, 45, G_IBOX, OF_NONE, OS_DISABLED, C_UNION(0x21100L), 0x0000,0x0000, 0x001c,0x0214 },
	{ 26, 4, 25, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x2021100L), 0x0001,0x030b, 0x001a,0x0008 },
	{ 5, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x21L), 0x0006,0x0500, 0x000e,0x0001 },
	{ 6, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x22L), 0x0002,0x0002, 0x0004,0x0001 },
	{ 7, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x23L), 0x0512,0x0002, 0x0006,0x0001 },
	{ 8, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x24L), 0x0002,0x0003, 0x0004,0x0001 },
	{ 9, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x25L), 0x0013,0x0003, 0x0006,0x0001 },
	{ 10, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x26L), 0x000b,0x0003, 0x0004,0x0001 },
	{ 15, 11, 13, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0002,0x0204, 0x0004,0x0003 },
	{ 12, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x1FF1100L), 0x0000,0x0000, 0x0004,0x0001 },
	{ 13, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x2FF1100L), 0x0000,0x0002, 0x0004,0x0001 },
	{ 10, 14, 14, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0xFF1100L), 0x0000,0x0001, 0x0004,0x0001 },
	{ 13, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x27L), 0x0001,0x0000, 0x0002,0x0001 },
	{ 20, 16, 18, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x000b,0x0204, 0x0004,0x0003 },
	{ 17, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x1FF1100L), 0x0000,0x0000, 0x0004,0x0001 },
	{ 18, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x2FF1100L), 0x0000,0x0002, 0x0004,0x0001 },
	{ 15, 19, 19, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0xFF1100L), 0x0000,0x0001, 0x0004,0x0001 },
	{ 18, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x28L), 0x0001,0x0000, 0x0002,0x0001 },
	{ 25, 21, 23, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0014,0x0204, 0x0004,0x0003 },
	{ 22, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x1FF1100L), 0x0000,0x0000, 0x0004,0x0001 },
	{ 23, -1, -1, G_BOXCHAR, OF_TOUCHEXIT, OS_NORMAL, C_UNION(0x2FF1100L), 0x0000,0x0002, 0x0004,0x0001 },
	{ 20, 24, 24, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0xFF1100L), 0x0000,0x0001, 0x0004,0x0001 },
	{ 23, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x29L), 0x0001,0x0000, 0x0002,0x0001 },
	{ 3, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x2AL), 0x040a,0x0002, 0x0005,0x0001 },
	{ 45, 27, 35, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x21100L), 0x0001,0x0002, 0x0619,0x0308 },
	{ 28, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x2BL), 0x0002,0x0001, 0x000c,0x0001 },
	{ 31, 29, 30, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x000f,0x0001, 0x000a,0x0001 },
	{ 30, -1, -1, G_BUTTON, 0x11, OS_SELECTED, C_UNION(0x2CL), 0x0000,0x0000, 0x0004,0x0001 },
	{ 28, -1, -1, G_BUTTON, 0x11, OS_NORMAL, C_UNION(0x2DL), 0x0105,0x0000, 0x0004,0x0001 },
	{ 32, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x2EL), 0x0003,0x0002, 0x000b,0x0001 },
	{ 35, 33, 34, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x000f,0x0002, 0x000a,0x0001 },
	{ 34, -1, -1, G_BUTTON, 0x11, OS_NORMAL, C_UNION(0x2FL), 0x0105,0x0000, 0x0004,0x0001 },
	{ 32, -1, -1, G_BUTTON, 0x11, OS_SELECTED, C_UNION(0x30L), 0x0000,0x0000, 0x0004,0x0001 },
	{ 26, 36, 41, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0001,0x0003, 0x0018,0x0005 },
	{ 37, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x31L), 0x0006,0x0000, 0x0007,0x0001 },
	{ 40, 38, 39, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x000e,0x0000, 0x000a,0x0001 },
	{ 39, -1, -1, G_BUTTON, 0x11, OS_NORMAL, C_UNION(0x32L), 0x0105,0x0000, 0x0004,0x0001 },
	{ 37, -1, -1, G_BUTTON, 0x11, OS_SELECTED, C_UNION(0x33L), 0x0000,0x0000, 0x0004,0x0001 },
	{ 41, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x34L), 0x0007,0x0002, 0x000c,0x0001 },
	{ 35, 42, 44, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0001,0x0004, 0x0016,0x0001 },
	{ 43, -1, -1, G_BUTTON, 0x11, OS_NORMAL, C_UNION(0x35L), 0x0000,0x0000, 0x0006,0x0001 },
	{ 44, -1, -1, G_BUTTON, 0x11, OS_SELECTED, C_UNION(0x36L), 0x0007,0x0000, 0x0007,0x0001 },
	{ 41, -1, -1, G_BUTTON, 0x11, OS_NORMAL, C_UNION(0x37L), 0x000f,0x0000, 0x0007,0x0001 },
	{ 2, -1, -1, G_BUTTON, OF_SELECTABLE, OS_NORMAL, C_UNION(0x38L), 0x0404,0x0600, 0x0013,0x0001 },
	{ 50, 47, 47, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0xFE2100L), 0x061d,0x010f, 0x0008,0x0204 },
	{ 46, 48, 49, G_IBOX, OF_NONE, OS_DISABLED, C_UNION(0x11100L), 0x0100,0x0000, 0x0707,0x0204 },
	{ 49, -1, -1, G_BUTTON, 0x5, OS_NORMAL, C_UNION(0x39L), 0x0001,0x0402, 0x0605,0x0101 },
	{ 47, -1, -1, G_BUTTON, 0x7, OS_NORMAL, C_UNION(0x3AL), 0x0700,0x0500, 0x0006,0x0101 },
	{ 0, 51, 51, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0xFE2100L), 0x011d,0x040a, 0x0509,0x0603 },
	{ 50, 52, 52, G_IBOX, OF_NONE, OS_DISABLED, C_UNION(0x11100L), 0x0100,0x0000, 0x0409,0x0603 },
	{ 51, 53, 54, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0500,0x0400, 0x0108,0x0502 },
	{ 54, -1, -1, G_BUTTON, 0x11, OS_SELECTED, C_UNION(0x3BL), 0x0100,0x0100, 0x0008,0x0001 },
	{ 52, -1, -1, G_BUTTON, 0x71, OS_NORMAL, C_UNION(0x3CL), 0x0100,0x0501, 0x0008,0x0001 },
	{ -1, 1, 102, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x11100L), 0x0000,0x0000, 0x0012,0x0016 },
	{ 26, 2, 20, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0001,0x0001, 0x0010,0x0004 },
	{ 8, 3, 7, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0000,0x0000, 0x030f,0x0001 },
	{ 4, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x3DL), 0x0000,0x0000, 0x000a,0x0001 },
	{ 5, -1, -1, G_BOX, 0x11, OS_SELECTED, C_UNION(0x29100L), 0x000b,0x0000, 0x0001,0x0001 },
	{ 6, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x26100L), 0x010c,0x0000, 0x0001,0x0001 },
	{ 7, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x2A100L), 0x020d,0x0000, 0x0001,0x0001 },
	{ 2, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x2F100L), 0x030e,0x0000, 0x0001,0x0001 },
	{ 14, 9, 13, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0000,0x0001, 0x000f,0x0001 },
	{ 10, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x3EL), 0x0000,0x0000, 0x000a,0x0001 },
	{ 11, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x29100L), 0x000b,0x0000, 0x0001,0x0001 },
	{ 12, -1, -1, G_BOX, 0x11, OS_SELECTED, C_UNION(0x26100L), 0x010c,0x0000, 0x0001,0x0001 },
	{ 13, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x2A100L), 0x020d,0x0000, 0x0001,0x0001 },
	{ 8, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x2F100L), 0x030e,0x0000, 0x0001,0x0001 },
	{ 20, 15, 19, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0000,0x0002, 0x000f,0x0001 },
	{ 16, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x3FL), 0x0000,0x0000, 0x000a,0x0001 },
	{ 17, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x29100L), 0x000b,0x0000, 0x0001,0x0001 },
	{ 18, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x26100L), 0x010c,0x0000, 0x0001,0x0001 },
	{ 19, -1, -1, G_BOX, 0x11, OS_SELECTED, C_UNION(0x2A100L), 0x020d,0x0000, 0x0001,0x0001 },
	{ 14, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x2F100L), 0x030e,0x0000, 0x0001,0x0001 },
	{ 1, 21, 25, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0000,0x0003, 0x000f,0x0001 },
	{ 22, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x40L), 0x0000,0x0000, 0x000a,0x0001 },
	{ 23, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x29100L), 0x000b,0x0000, 0x0001,0x0001 },
	{ 24, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x26100L), 0x010c,0x0000, 0x0001,0x0001 },
	{ 25, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x2A100L), 0x020d,0x0000, 0x0001,0x0001 },
	{ 20, -1, -1, G_BOX, 0x11, OS_SELECTED, C_UNION(0x2F100L), 0x030e,0x0000, 0x0001,0x0001 },
	{ 51, 27, 45, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0001,0x0005, 0x0010,0x0004 },
	{ 33, 28, 32, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0000,0x0000, 0x000f,0x0001 },
	{ 29, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x41L), 0x0000,0x0000, 0x000a,0x0001 },
	{ 30, -1, -1, G_BOX, 0x11, OS_SELECTED, C_UNION(0x29100L), 0x000b,0x0000, 0x0001,0x0001 },
	{ 31, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x26100L), 0x010c,0x0000, 0x0001,0x0001 },
	{ 32, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x2A100L), 0x020d,0x0000, 0x0001,0x0001 },
	{ 27, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x2F100L), 0x030e,0x0000, 0x0001,0x0001 },
	{ 39, 34, 38, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0000,0x0001, 0x030f,0x0001 },
	{ 35, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x42L), 0x0000,0x0000, 0x000a,0x0001 },
	{ 36, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x29100L), 0x000b,0x0000, 0x0001,0x0001 },
	{ 37, -1, -1, G_BOX, 0x11, OS_SELECTED, C_UNION(0x26100L), 0x010c,0x0000, 0x0001,0x0001 },
	{ 38, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x2A100L), 0x020d,0x0000, 0x0001,0x0001 },
	{ 33, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x2F100L), 0x030e,0x0000, 0x0001,0x0001 },
	{ 45, 40, 44, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0000,0x0002, 0x030f,0x0001 },
	{ 41, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x43L), 0x0000,0x0000, 0x000a,0x0001 },
	{ 42, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x29100L), 0x000b,0x0000, 0x0001,0x0001 },
	{ 43, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x26100L), 0x010c,0x0000, 0x0001,0x0001 },
	{ 44, -1, -1, G_BOX, 0x11, OS_SELECTED, C_UNION(0x2A100L), 0x020d,0x0000, 0x0001,0x0001 },
	{ 39, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x2F100L), 0x030e,0x0000, 0x0001,0x0001 },
	{ 26, 46, 50, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0000,0x0003, 0x030f,0x0001 },
	{ 47, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x44L), 0x0000,0x0000, 0x000a,0x0001 },
	{ 48, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x29100L), 0x000b,0x0000, 0x0001,0x0001 },
	{ 49, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x26100L), 0x010c,0x0000, 0x0001,0x0001 },
	{ 50, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x2A100L), 0x020d,0x0000, 0x0001,0x0001 },
	{ 45, -1, -1, G_BOX, 0x11, OS_SELECTED, C_UNION(0x2F100L), 0x030e,0x0000, 0x0001,0x0001 },
	{ 76, 52, 70, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0001,0x0009, 0x0010,0x0004 },
	{ 58, 53, 57, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0000,0x0000, 0x030f,0x0001 },
	{ 54, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x45L), 0x0000,0x0000, 0x000a,0x0001 },
	{ 55, -1, -1, G_BOX, 0x11, OS_SELECTED, C_UNION(0x29100L), 0x000b,0x0000, 0x0001,0x0001 },
	{ 56, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x26100L), 0x010c,0x0000, 0x0001,0x0001 },
	{ 57, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x2A100L), 0x020d,0x0000, 0x0001,0x0001 },
	{ 52, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x2F100L), 0x030e,0x0000, 0x0001,0x0001 },
	{ 64, 59, 63, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0000,0x0001, 0x030f,0x0001 },
	{ 60, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x46L), 0x0000,0x0000, 0x000a,0x0001 },
	{ 61, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x29100L), 0x000b,0x0000, 0x0001,0x0001 },
	{ 62, -1, -1, G_BOX, 0x11, OS_SELECTED, C_UNION(0x26100L), 0x010c,0x0000, 0x0001,0x0001 },
	{ 63, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x2A100L), 0x020d,0x0000, 0x0001,0x0001 },
	{ 58, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x2F100L), 0x030e,0x0000, 0x0001,0x0001 },
	{ 70, 65, 69, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0000,0x0002, 0x030f,0x0001 },
	{ 66, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x47L), 0x0000,0x0000, 0x000a,0x0001 },
	{ 67, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x29100L), 0x000b,0x0000, 0x0001,0x0001 },
	{ 68, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x26100L), 0x010c,0x0000, 0x0001,0x0001 },
	{ 69, -1, -1, G_BOX, 0x11, OS_SELECTED, C_UNION(0x2A100L), 0x020d,0x0000, 0x0001,0x0001 },
	{ 64, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x2F100L), 0x030e,0x0000, 0x0001,0x0001 },
	{ 51, 71, 75, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0000,0x0003, 0x030f,0x0001 },
	{ 72, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x48L), 0x0000,0x0000, 0x000a,0x0001 },
	{ 73, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x29100L), 0x000b,0x0000, 0x0001,0x0001 },
	{ 74, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x26100L), 0x010c,0x0000, 0x0001,0x0001 },
	{ 75, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x2A100L), 0x020d,0x0000, 0x0001,0x0001 },
	{ 70, -1, -1, G_BOX, 0x11, OS_SELECTED, C_UNION(0x2F100L), 0x030e,0x0000, 0x0001,0x0001 },
	{ 101, 77, 95, G_BOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0001,0x000d, 0x0010,0x0004 },
	{ 83, 78, 82, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0000,0x0000, 0x030f,0x0001 },
	{ 79, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x49L), 0x0000,0x0000, 0x000a,0x0001 },
	{ 80, -1, -1, G_BOX, 0x11, OS_SELECTED, C_UNION(0x29100L), 0x000b,0x0000, 0x0001,0x0001 },
	{ 81, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x26100L), 0x010c,0x0000, 0x0001,0x0001 },
	{ 82, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x2A100L), 0x020d,0x0000, 0x0001,0x0001 },
	{ 77, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x2F100L), 0x030e,0x0000, 0x0001,0x0001 },
	{ 89, 84, 88, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0000,0x0001, 0x030f,0x0001 },
	{ 85, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x4AL), 0x0000,0x0000, 0x000a,0x0001 },
	{ 86, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x29100L), 0x000b,0x0000, 0x0001,0x0001 },
	{ 87, -1, -1, G_BOX, 0x11, OS_SELECTED, C_UNION(0x26100L), 0x010c,0x0000, 0x0001,0x0001 },
	{ 88, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x2A100L), 0x020d,0x0000, 0x0001,0x0001 },
	{ 83, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x2F100L), 0x030e,0x0000, 0x0001,0x0001 },
	{ 95, 90, 94, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0000,0x0002, 0x030f,0x0001 },
	{ 91, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x4BL), 0x0000,0x0000, 0x000a,0x0001 },
	{ 92, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x29100L), 0x000b,0x0000, 0x0001,0x0001 },
	{ 93, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x26100L), 0x010c,0x0000, 0x0001,0x0001 },
	{ 94, -1, -1, G_BOX, 0x11, OS_SELECTED, C_UNION(0x2A100L), 0x020d,0x0000, 0x0001,0x0001 },
	{ 89, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x2F100L), 0x030e,0x0000, 0x0001,0x0001 },
	{ 76, 96, 100, G_IBOX, OF_NONE, OS_NORMAL, C_UNION(0x1100L), 0x0000,0x0003, 0x030f,0x0001 },
	{ 97, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x4CL), 0x0000,0x0000, 0x000a,0x0001 },
	{ 98, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x29100L), 0x000b,0x0000, 0x0001,0x0001 },
	{ 99, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x26100L), 0x010c,0x0000, 0x0001,0x0001 },
	{ 100, -1, -1, G_BOX, 0x11, OS_NORMAL, C_UNION(0x2A100L), 0x020d,0x0000, 0x0001,0x0001 },
	{ 95, -1, -1, G_BOX, 0x11, OS_SELECTED, C_UNION(0x2F100L), 0x030e,0x0000, 0x0001,0x0001 },
	{ 102, -1, -1, G_BUTTON, 0x7, OS_NORMAL, C_UNION(0x4DL), 0x0005,0x0014, 0x0008,0x0001 },
	{ 0, -1, -1, G_BUTTON, 0x21, OS_NORMAL, C_UNION(0x4EL), 0x0002,0x0012, 0x000d,0x0001 },
	{ -1, 1, 2, G_BOX, OF_NONE, OS_OUTLINED, C_UNION(0x21100L), 0x0000,0x0000, 0x000e,0x0004 },
	{ 2, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x4FL), 0x0001,0x0001, 0x000b,0x0101 },
	{ 0, -1, -1, G_STRING, OF_LASTOB, OS_NORMAL, C_UNION(0x50L), 0x0002,0x0002, 0x000a,0x0001 },
	{ -1, 1, 2, G_BOX, OF_NONE, OS_OUTLINED, C_UNION(0x21100L), 0x0000,0x0000, 0x0015,0x0005 },
	{ 2, -1, -1, G_FBOXTEXT, OF_EDITABLE, OS_NORMAL, C_UNION(0x0L), 0x0002,0x0001, 0x0011,0x0001 },
	{ 0, -1, -1, G_BUTTON, 0x27, OS_NORMAL, C_UNION(0x54L), 0x0010,0x0003, 0x0003,0x0001 },
	{ -1, 1, 6, G_BOX, OF_NONE, OS_OUTLINED, C_UNION(0x21100L), 0x0000,0x0000, 0x001b,0x0009 },
	{ 2, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x55L), 0x0007,0x0001, 0x000c,0x0001 },
	{ 3, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x56L), 0x0004,0x0002, 0x0012,0x0001 },
	{ 4, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x57L), 0x0009,0x0003, 0x0009,0x0001 },
	{ 5, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x58L), 0x0003,0x0004, 0x0014,0x0001 },
	{ 6, -1, -1, G_BUTTON, 0x7, OS_NORMAL, C_UNION(0x59L), 0x000b,0x0007, 0x0005,0x0001 },
	{ 0, -1, -1, G_STRING, OF_LASTOB, OS_NORMAL, C_UNION(0x5AL), 0x0003,0x0005, 0x0012,0x0001 },
	{ -1, 1, 2, G_BOX, OF_NONE, OS_OUTLINED, C_UNION(0x21100L), 0x0000,0x0000, 0x000d,0x0004 },
	{ 2, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x5BL), 0x0002,0x0001, 0x0009,0x0001 },
	{ 0, -1, -1, G_STRING, OF_LASTOB, OS_NORMAL, C_UNION(0x5CL), 0x0003,0x0002, 0x0007,0x0001 },
	{ -1, 1, 2, G_BOX, OF_NONE, OS_OUTLINED, C_UNION(0x21100L), 0x0000,0x0000, 0x0012,0x0004 },
	{ 2, -1, -1, G_STRING, OF_NONE, OS_NORMAL, C_UNION(0x5DL), 0x0001,0x0001, 0x0410,0x0001 },
	{ 0, -1, -1, G_STRING, OF_LASTOB, OS_NORMAL, C_UNION(0x5EL), 0x0005,0x0002, 0x0009,0x0001 }
};

_LONG_PTR rs_trindex[] = {
	0L,
	23L,
	27L,
	31L,
	34L,
	37L,
	40L,
	43L,
	46L,
	101L,
	204L,
	207L,
	210L,
	217L,
	220L
};

#ifndef __foobar_defined
#define __foobar_defined 1
struct foobar {
	WORD 	dummy;
	WORD 	*image;
};
#endif
struct foobar rs_imdope[] = {
	{ 0, 0 }
};



#define NUM_STRINGS 95
#define NUM_FRSTR 0
#define NUM_UD 0
#define NUM_IMAGES 0
#define NUM_BB 0
#define NUM_FRIMG 0
#define NUM_IB 0
#define NUM_CIB 0
#define NUM_TI 1
#define NUM_OBS 223
#define NUM_TREE 15

BYTE pname[] = "RSTEST.RSC";
