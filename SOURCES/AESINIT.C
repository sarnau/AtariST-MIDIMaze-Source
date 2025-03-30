/************************************************************
 *** Embedded RSC file and the AES initialization/exit code
 ************************************************************/
#include "GLOBALS.H"

/* Strings used by the resource tree */
static char *rs_strings[NUM_STRINGS] = { " \x0e\x0f ", /* This is the Atari symbol */
                                        " MAZE ",
                                        "  About MIDImaze... ",
                                        "--------------------",
                                        "1",
                                        "2",
                                        "3",
                                        "4",
                                        "5",
                                        "6",
                                        " Load... ",
                                        " Reset score",
                                        "-------------",
                                        " Play..!",
                                        " Quit","-------------",
                                        " Names...",
                                        "This is a",
                                        "SLAVE",
                                        "machine.",
                                        "Boo-boo:",
                                        "The MIDI ring",
                                        "has timed out.",
                                        "Game terminated by the",
                                        "bozo on the master machine.",
                                        "Guru bu-bu:",
                                        "(maze too small?)",
                                        "Mouse control",
                                        "selected.",
                                        "Joystick control",
                                        "selected.",
                                        "You are",
                                        "Number __",
                                        "Select Drones:",
                                        "Very",
                                        "Not so",
                                        "Dumb",
                                        " Dumb",
                                        "Dumb",
                                        " 0",
                                        " 0",
                                        " 0",
                                        "Plain",
                                        "Reload Time:",
                                        "Fast",
                                        "Slow",
                                        "Regen Time:",
                                        "Slow",
                                        "Fast",
                                        "Revive:",
                                        "Slow",
                                        "Fast",
                                        "Revive with:",
                                        "1 Life",
                                        "2 Lives",
                                        "3 Lives",
                                        "00 Machines on-line",
                                        "Nah",
                                        "Yeah!",
                                        "Singles",
                                        "Teams",
                                        "Player # 1",
                                        "Player # 2",
                                        "Player # 3",
                                        "Player # 4",
                                        "Player # 5",
                                        "Player # 6",
                                        "Player # 7",
                                        "Player # 8",
                                        "Player # 9",
                                        "Player #10",
                                        "Player #11",
                                        "Player #12",
                                        "Player #13",
                                        "Player #14",
                                        "Player #15",
                                        "Player #16",
                                        "O-Tay!",
                                        "Friendly Fire",
                                        "The game is",
                                        "suspended.",
                                        "Player #xx",
                                        "Name: __________",
                                        "nnnnnnnnnn",
                                        "OK",
                                        "MIDIMAZE 1.0",
                                        "Copyright (c) 1987",
                                        "XANTH F/X",
                                        "All rights reserved.",
                                        "Wow!",
                                        "This is a",
                                        "MIDICAM" };

/* All resource objects */
static OBJECT rs_object[NUM_OBS] = { 
        /* TREE 0 (0) */
        /*   0 [0] */ {  -1,    1,    5,      G_IBOX,                                NONE,   NORMAL, 0x00000000, 0x0000, 0x0000, 0x005a, 0x0019 },
        /*   1 [1] */ {   5,    2,    2,       G_BOX,                                NONE,   NORMAL, 0x00001100, 0x0000, 0x0000, 0x005a, 0x0201 },
        /*   2 [2] */ {   1,    3,    4,      G_IBOX,                                NONE,   NORMAL, 0x00000000, 0x0002, 0x0000, 0x000a, 0x0301 },
        /*   3 [3] */ {   4,   -1,   -1,     G_TITLE,                                NONE,   NORMAL,          0, 0x0000, 0x0000, 0x0004, 0x0301 } /* " \x0e\x0f " */,
        /*   4 [4] */ {   2,   -1,   -1,     G_TITLE,                                NONE,   NORMAL,          1, 0x0004, 0x0000, 0x0006, 0x0301 } /* " MAZE " */,
        /*   5 [5] */ {   0,    6,   15,      G_IBOX,                                NONE,   NORMAL, 0x00000000, 0x0000, 0x0301, 0x0050, 0x0013 },
        /*   6 [6] */ {  15,    7,   14,       G_BOX,                                NONE,   NORMAL, 0x00ff1100, 0x0002, 0x0000, 0x0014, 0x0008 },
        /*   7 [7] */ {   8,   -1,   -1,    G_STRING,                                NONE,   NORMAL,          2, 0x0000, 0x0000, 0x0014, 0x0001 } /* "  About MIDImaze... " */,
        /*   8 [8] */ {   9,   -1,   -1,    G_STRING,                                NONE, DISABLED,          3, 0x0000, 0x0001, 0x0014, 0x0001 } /* "--------------------" */,
        /*   9 [9] */ {  10,   -1,   -1,    G_STRING,                                NONE,   NORMAL,          4, 0x0000, 0x0002, 0x0014, 0x0001 } /* "1" */,
        /*  10 [10] */ {  11,   -1,   -1,    G_STRING,                                NONE,   NORMAL,          5, 0x0000, 0x0003, 0x0014, 0x0001 } /* "2" */,
        /*  11 [11] */ {  12,   -1,   -1,    G_STRING,                                NONE,   NORMAL,          6, 0x0000, 0x0004, 0x0014, 0x0001 } /* "3" */,
        /*  12 [12] */ {  13,   -1,   -1,    G_STRING,                                NONE,   NORMAL,          7, 0x0000, 0x0005, 0x0014, 0x0001 } /* "4" */,
        /*  13 [13] */ {  14,   -1,   -1,    G_STRING,                                NONE,   NORMAL,          8, 0x0000, 0x0006, 0x0014, 0x0001 } /* "5" */,
        /*  14 [14] */ {   6,   -1,   -1,    G_STRING,                                NONE,   NORMAL,          9, 0x0000, 0x0007, 0x0014, 0x0001 } /* "6" */,
        /*  15 [15] */ {   5,   16,   22,       G_BOX,                                NONE,   NORMAL, 0x00ff1100, 0x0006, 0x0000, 0x000d, 0x0007 },
        /*  16 [16] */ {  17,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         10, 0x0000, 0x0000, 0x000d, 0x0001 } /* " Load... " */,
        /*  17 [17] */ {  18,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         11, 0x0000, 0x0002, 0x000d, 0x0001 } /* " Reset score" */,
        /*  18 [18] */ {  19,   -1,   -1,    G_STRING,                                NONE, DISABLED,         12, 0x0000, 0x0001, 0x000d, 0x0001 } /* "-------------" */,
        /*  19 [19] */ {  20,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         13, 0x0000, 0x0004, 0x000d, 0x0001 } /* " Play..!" */,
        /*  20 [20] */ {  21,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         14, 0x0000, 0x0006, 0x000d, 0x0001 } /* " Quit" */,
        /*  21 [21] */ {  22,   -1,   -1,    G_STRING,                                NONE, DISABLED,         15, 0x0000, 0x0005, 0x000d, 0x0001 } /* "-------------" */,
        /*  22 [22] */ {  15,   -1,   -1,    G_STRING,                              LASTOB,   NORMAL,         16, 0x0000, 0x0003, 0x000d, 0x0001 } /* " Names..." */,
        /* TREE 1 (23) */
        /*  23 [0] */ {  -1,    1,    3,       G_BOX,                                NONE, OUTLINED, 0x00021100, 0x0000, 0x0000, 0x000c, 0x0005 },
        /*  24 [1] */ {   2,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         17, 0x0001, 0x0001, 0x0009, 0x0001 } /* "This is a" */,
        /*  25 [2] */ {   3,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         18, 0x0003, 0x0002, 0x0005, 0x0001 } /* "SLAVE" */,
        /*  26 [3] */ {   0,   -1,   -1,    G_STRING,                              LASTOB,   NORMAL,         19, 0x0002, 0x0003, 0x0008, 0x0001 } /* "machine." */,
        /* TREE 2 (27) */
        /*  27 [0] */ {  -1,    1,    3,       G_BOX,                                NONE, OUTLINED, 0x00021100, 0x0000, 0x0000, 0x0011, 0x0005 },
        /*  28 [1] */ {   2,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         20, 0x0002, 0x0001, 0x0009, 0x0001 } /* "Boo-boo:" */,
        /*  29 [2] */ {   3,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         21, 0x0002, 0x0002, 0x000d, 0x0001 } /* "The MIDI ring" */,
        /*  30 [3] */ {   0,   -1,   -1,    G_STRING,                              LASTOB,   NORMAL,         22, 0x0002, 0x0003, 0x000e, 0x0001 } /* "has timed out." */,
        /* TREE 3 (31) */
        /*  31 [0] */ {  -1,    1,    2,       G_BOX,                                NONE, OUTLINED, 0x00021100, 0x0000, 0x0000, 0x001d, 0x0004 },
        /*  32 [1] */ {   2,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         23, 0x0003, 0x0001, 0x0016, 0x0001 } /* "Game terminated by the" */,
        /*  33 [2] */ {   0,   -1,   -1,    G_STRING,                              LASTOB,   NORMAL,         24, 0x0001, 0x0002, 0x001b, 0x0001 } /* "bozo on the master machine." */,
        /* TREE 4 (34) */
        /*  34 [0] */ {  -1,    1,    2,       G_BOX,                                NONE, OUTLINED, 0x00021100, 0x0000, 0x0000, 0x0013, 0x0004 },
        /*  35 [1] */ {   2,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         25, 0x0003, 0x0001, 0x010b, 0x0001 } /* "Guru bu-bu:" */,
        /*  36 [2] */ {   0,   -1,   -1,    G_STRING,                              LASTOB,   NORMAL,         26, 0x0001, 0x0002, 0x0311, 0x0101 } /* "(maze too small?)" */,
        /* TREE 5 (37) */
        /*  37 [0] */ {  -1,    1,    2,       G_BOX,                                NONE, OUTLINED, 0x00021100, 0x0000, 0x0000, 0x000f, 0x0004 },
        /*  38 [1] */ {   2,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         27, 0x0001, 0x0001, 0x030d, 0x0001 } /* "Mouse control" */,
        /*  39 [2] */ {   0,   -1,   -1,    G_STRING,                              LASTOB,   NORMAL,         28, 0x0003, 0x0002, 0x0009, 0x0001 } /* "selected." */,
        /* TREE 6 (40) */
        /*  40 [0] */ {  -1,    1,    2,       G_BOX,                                NONE, OUTLINED, 0x00021100, 0x0000, 0x0000, 0x0012, 0x0004 },
        /*  41 [1] */ {   2,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         29, 0x0001, 0x0001, 0x0410, 0x0001 } /* "Joystick control" */,
        /*  42 [2] */ {   0,   -1,   -1,    G_STRING,                              LASTOB,   NORMAL,         30, 0x0005, 0x0002, 0x0009, 0x0001 } /* "selected." */,
        /* TREE 7 (43) */
        /*  43 [0] */ {  -1,    1,    2,       G_BOX,                                NONE, OUTLINED, 0x00021100, 0x0000, 0x0000, 0x000c, 0x0004 },
        /*  44 [1] */ {   2,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         31, 0x0002, 0x0001, 0x0407, 0x0001 } /* "You are" */,
        /*  45 [2] */ {   0,   -1,   -1,    G_STRING,                              LASTOB,   NORMAL,         32, 0x0001, 0x0002, 0x0009, 0x0001 } /* "Number __" */,
        /* TREE 8 (46) */
        /*  46 [0] */ {  -1,    1,   50,       G_BOX,                                NONE,   NORMAL, 0x00001100, 0x0000, 0x0000, 0x0626, 0x0214 },
        /*  47 [1] */ {  46,    2,    2,      G_IBOX,                                NONE,   NORMAL, 0x00fe2100, 0x0000, 0x0000, 0x001c, 0x0214 },
        /*  48 [2] */ {   1,    3,   45,      G_IBOX,                                NONE, DISABLED, 0x00021100, 0x0000, 0x0000, 0x001c, 0x0214 },
        /*  49 [3] */ {  26,    4,   25,      G_IBOX,                                NONE,   NORMAL, 0x02021100, 0x0001, 0x030b, 0x001a, 0x0008 },
        /*  50 [4] */ {   5,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         33, 0x0006, 0x0500, 0x000e, 0x0001 } /* "Select Drones:" */,
        /*  51 [5] */ {   6,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         34, 0x0002, 0x0002, 0x0004, 0x0001 } /* "Very" */,
        /*  52 [6] */ {   7,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         35, 0x0512, 0x0002, 0x0006, 0x0001 } /* "Not so" */,
        /*  53 [7] */ {   8,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         36, 0x0002, 0x0003, 0x0004, 0x0001 } /* "Dumb" */,
        /*  54 [8] */ {   9,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         37, 0x0013, 0x0003, 0x0006, 0x0001 } /* " Dumb" */,
        /*  55 [9] */ {  10,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         38, 0x000b, 0x0003, 0x0004, 0x0001 } /* "Dumb" */,
        /*  56 [10] */ {  15,   11,   13,      G_IBOX,                                NONE,   NORMAL, 0x00001100, 0x0002, 0x0204, 0x0004, 0x0003 },
        /*  57 [11] */ {  12,   -1,   -1,   G_BOXCHAR,                           TOUCHEXIT,   NORMAL, 0x01ff1100, 0x0000, 0x0000, 0x0004, 0x0001 },
        /*  58 [12] */ {  13,   -1,   -1,   G_BOXCHAR,                           TOUCHEXIT,   NORMAL, 0x02ff1100, 0x0000, 0x0002, 0x0004, 0x0001 },
        /*  59 [13] */ {  10,   14,   14,       G_BOX,                                NONE,   NORMAL, 0x00ff1100, 0x0000, 0x0001, 0x0004, 0x0001 },
        /*  60 [14] */ {  13,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         39, 0x0001, 0x0000, 0x0002, 0x0001 } /* " 0" */,
        /*  61 [15] */ {  20,   16,   18,      G_IBOX,                                NONE,   NORMAL, 0x00001100, 0x000b, 0x0204, 0x0004, 0x0003 },
        /*  62 [16] */ {  17,   -1,   -1,   G_BOXCHAR,                           TOUCHEXIT,   NORMAL, 0x01ff1100, 0x0000, 0x0000, 0x0004, 0x0001 },
        /*  63 [17] */ {  18,   -1,   -1,   G_BOXCHAR,                           TOUCHEXIT,   NORMAL, 0x02ff1100, 0x0000, 0x0002, 0x0004, 0x0001 },
        /*  64 [18] */ {  15,   19,   19,       G_BOX,                                NONE,   NORMAL, 0x00ff1100, 0x0000, 0x0001, 0x0004, 0x0001 },
        /*  65 [19] */ {  18,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         40, 0x0001, 0x0000, 0x0002, 0x0001 } /* " 0" */,
        /*  66 [20] */ {  25,   21,   23,      G_IBOX,                                NONE,   NORMAL, 0x00001100, 0x0014, 0x0204, 0x0004, 0x0003 },
        /*  67 [21] */ {  22,   -1,   -1,   G_BOXCHAR,                           TOUCHEXIT,   NORMAL, 0x01ff1100, 0x0000, 0x0000, 0x0004, 0x0001 },
        /*  68 [22] */ {  23,   -1,   -1,   G_BOXCHAR,                           TOUCHEXIT,   NORMAL, 0x02ff1100, 0x0000, 0x0002, 0x0004, 0x0001 },
        /*  69 [23] */ {  20,   24,   24,       G_BOX,                                NONE,   NORMAL, 0x00ff1100, 0x0000, 0x0001, 0x0004, 0x0001 },
        /*  70 [24] */ {  23,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         41, 0x0001, 0x0000, 0x0002, 0x0001 } /* " 0" */,
        /*  71 [25] */ {   3,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         42, 0x040a, 0x0002, 0x0005, 0x0001 } /* "Plain" */,
        /*  72 [26] */ {  45,   27,   35,      G_IBOX,                                NONE,   NORMAL, 0x00021100, 0x0001, 0x0002, 0x0619, 0x0308 },
        /*  73 [27] */ {  28,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         43, 0x0002, 0x0001, 0x000c, 0x0001 } /* "Reload Time:" */,
        /*  74 [28] */ {  31,   29,   30,      G_IBOX,                                NONE,   NORMAL, 0x00001100, 0x000f, 0x0001, 0x000a, 0x0001 },
        /*  75 [29] */ {  30,   -1,   -1,    G_BUTTON,                  SELECTABLE|RBUTTON, SELECTED,         44, 0x0000, 0x0000, 0x0004, 0x0001 } /* "Fast" */,
        /*  76 [30] */ {  28,   -1,   -1,    G_BUTTON,                  SELECTABLE|RBUTTON,   NORMAL,         45, 0x0105, 0x0000, 0x0004, 0x0001 } /* "Slow" */,
        /*  77 [31] */ {  32,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         46, 0x0003, 0x0002, 0x000b, 0x0001 } /* "Regen Time:" */,
        /*  78 [32] */ {  35,   33,   34,      G_IBOX,                                NONE,   NORMAL, 0x00001100, 0x000f, 0x0002, 0x000a, 0x0001 },
        /*  79 [33] */ {  34,   -1,   -1,    G_BUTTON,                  SELECTABLE|RBUTTON,   NORMAL,         47, 0x0105, 0x0000, 0x0004, 0x0001 } /* "Slow" */,
        /*  80 [34] */ {  32,   -1,   -1,    G_BUTTON,                  SELECTABLE|RBUTTON, SELECTED,         48, 0x0000, 0x0000, 0x0004, 0x0001 } /* "Fast" */,
        /*  81 [35] */ {  26,   36,   41,      G_IBOX,                                NONE,   NORMAL, 0x00001100, 0x0001, 0x0003, 0x0018, 0x0005 },
        /*  82 [36] */ {  37,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         49, 0x0006, 0x0000, 0x0007, 0x0001 } /* "Revive:" */,
        /*  83 [37] */ {  40,   38,   39,      G_IBOX,                                NONE,   NORMAL, 0x00001100, 0x000e, 0x0000, 0x000a, 0x0001 },
        /*  84 [38] */ {  39,   -1,   -1,    G_BUTTON,                  SELECTABLE|RBUTTON,   NORMAL,         50, 0x0105, 0x0000, 0x0004, 0x0001 } /* "Slow" */,
        /*  85 [39] */ {  37,   -1,   -1,    G_BUTTON,                  SELECTABLE|RBUTTON, SELECTED,         51, 0x0000, 0x0000, 0x0004, 0x0001 } /* "Fast" */,
        /*  86 [40] */ {  41,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         52, 0x0007, 0x0002, 0x000c, 0x0001 } /* "Revive with:" */,
        /*  87 [41] */ {  35,   42,   44,      G_IBOX,                                NONE,   NORMAL, 0x00001100, 0x0001, 0x0004, 0x0016, 0x0001 },
        /*  88 [42] */ {  43,   -1,   -1,    G_BUTTON,                  SELECTABLE|RBUTTON,   NORMAL,         53, 0x0000, 0x0000, 0x0006, 0x0001 } /* "1 Life" */,
        /*  89 [43] */ {  44,   -1,   -1,    G_BUTTON,                  SELECTABLE|RBUTTON, SELECTED,         54, 0x0007, 0x0000, 0x0007, 0x0001 } /* "2 Lives" */,
        /*  90 [44] */ {  41,   -1,   -1,    G_BUTTON,                  SELECTABLE|RBUTTON,   NORMAL,         55, 0x000f, 0x0000, 0x0007, 0x0001 } /* "3 Lives" */,
        /*  91 [45] */ {   2,   -1,   -1,    G_BUTTON,                          SELECTABLE,   NORMAL,         56, 0x0404, 0x0600, 0x0013, 0x0001 } /* "00 Machines on-line" */,
        /*  92 [46] */ {  50,   47,   47,      G_IBOX,                                NONE,   NORMAL, 0x00fe2100, 0x061d, 0x010f, 0x0008, 0x0204 },
        /*  93 [47] */ {  46,   48,   49,      G_IBOX,                                NONE, DISABLED, 0x00011100, 0x0100, 0x0000, 0x0707, 0x0204 },
        /*  94 [48] */ {  49,   -1,   -1,    G_BUTTON,                     SELECTABLE|EXIT,   NORMAL,         57, 0x0001, 0x0402, 0x0605, 0x0101 } /* "Nah" */,
        /*  95 [49] */ {  47,   -1,   -1,    G_BUTTON,             SELECTABLE|DEFAULT|EXIT,   NORMAL,         58, 0x0700, 0x0500, 0x0006, 0x0101 } /* "Yeah!" */,
        /*  96 [50] */ {   0,   51,   51,      G_IBOX,                                NONE,   NORMAL, 0x00fe2100, 0x011d, 0x040a, 0x0509, 0x0603 },
        /*  97 [51] */ {  50,   52,   52,      G_IBOX,                                NONE, DISABLED, 0x00011100, 0x0100, 0x0000, 0x0409, 0x0603 },
        /*  98 [52] */ {  51,   53,   54,      G_IBOX,                                NONE,   NORMAL, 0x00001100, 0x0500, 0x0400, 0x0108, 0x0502 },
        /*  99 [53] */ {  54,   -1,   -1,    G_BUTTON,                  SELECTABLE|RBUTTON, SELECTED,         59, 0x0100, 0x0100, 0x0008, 0x0001 } /* "Singles" */,
        /* 100 [54] */ {  52,   -1,   -1,    G_BUTTON, SELECTABLE|RBUTTON|LASTOB|TOUCHEXIT,   NORMAL,         60, 0x0100, 0x0501, 0x0008, 0x0001 } /* "Teams" */,
        /* TREE 9 (101) */
        /* 101 [0] */ {  -1,    1,  102,       G_BOX,                                NONE,   NORMAL, 0x00011100, 0x0000, 0x0000, 0x0012, 0x0016 },
        /* 102 [1] */ {  26,    2,   20,       G_BOX,                                NONE,   NORMAL, 0x00001100, 0x0001, 0x0001, 0x0010, 0x0004 },
        /* 103 [2] */ {   8,    3,    7,      G_IBOX,                                NONE,   NORMAL, 0x00001100, 0x0000, 0x0000, 0x030f, 0x0001 },
        /* 104 [3] */ {   4,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         61, 0x0000, 0x0000, 0x000a, 0x0001 } /* "Player # 1" */,
        /* 105 [4] */ {   5,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON, SELECTED, 0x00029100, 0x000b, 0x0000, 0x0001, 0x0001 },
        /* 106 [5] */ {   6,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x00026100, 0x010c, 0x0000, 0x0001, 0x0001 },
        /* 107 [6] */ {   7,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x0002a100, 0x020d, 0x0000, 0x0001, 0x0001 },
        /* 108 [7] */ {   2,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x0002f100, 0x030e, 0x0000, 0x0001, 0x0001 },
        /* 109 [8] */ {  14,    9,   13,      G_IBOX,                                NONE,   NORMAL, 0x00001100, 0x0000, 0x0001, 0x000f, 0x0001 },
        /* 110 [9] */ {  10,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         62, 0x0000, 0x0000, 0x000a, 0x0001 } /* "Player # 2" */,
        /* 111 [10] */ {  11,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x00029100, 0x000b, 0x0000, 0x0001, 0x0001 },
        /* 112 [11] */ {  12,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON, SELECTED, 0x00026100, 0x010c, 0x0000, 0x0001, 0x0001 },
        /* 113 [12] */ {  13,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x0002a100, 0x020d, 0x0000, 0x0001, 0x0001 },
        /* 114 [13] */ {   8,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x0002f100, 0x030e, 0x0000, 0x0001, 0x0001 },
        /* 115 [14] */ {  20,   15,   19,      G_IBOX,                                NONE,   NORMAL, 0x00001100, 0x0000, 0x0002, 0x000f, 0x0001 },
        /* 116 [15] */ {  16,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         63, 0x0000, 0x0000, 0x000a, 0x0001 } /* "Player # 3" */,
        /* 117 [16] */ {  17,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x00029100, 0x000b, 0x0000, 0x0001, 0x0001 },
        /* 118 [17] */ {  18,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x00026100, 0x010c, 0x0000, 0x0001, 0x0001 },
        /* 119 [18] */ {  19,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON, SELECTED, 0x0002a100, 0x020d, 0x0000, 0x0001, 0x0001 },
        /* 120 [19] */ {  14,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x0002f100, 0x030e, 0x0000, 0x0001, 0x0001 },
        /* 121 [20] */ {   1,   21,   25,      G_IBOX,                                NONE,   NORMAL, 0x00001100, 0x0000, 0x0003, 0x000f, 0x0001 },
        /* 122 [21] */ {  22,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         64, 0x0000, 0x0000, 0x000a, 0x0001 } /* "Player # 4" */,
        /* 123 [22] */ {  23,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x00029100, 0x000b, 0x0000, 0x0001, 0x0001 },
        /* 124 [23] */ {  24,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x00026100, 0x010c, 0x0000, 0x0001, 0x0001 },
        /* 125 [24] */ {  25,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x0002a100, 0x020d, 0x0000, 0x0001, 0x0001 },
        /* 126 [25] */ {  20,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON, SELECTED, 0x0002f100, 0x030e, 0x0000, 0x0001, 0x0001 },
        /* 127 [26] */ {  51,   27,   45,       G_BOX,                                NONE,   NORMAL, 0x00001100, 0x0001, 0x0005, 0x0010, 0x0004 },
        /* 128 [27] */ {  33,   28,   32,      G_IBOX,                                NONE,   NORMAL, 0x00001100, 0x0000, 0x0000, 0x000f, 0x0001 },
        /* 129 [28] */ {  29,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         65, 0x0000, 0x0000, 0x000a, 0x0001 } /* "Player # 5" */,
        /* 130 [29] */ {  30,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON, SELECTED, 0x00029100, 0x000b, 0x0000, 0x0001, 0x0001 },
        /* 131 [30] */ {  31,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x00026100, 0x010c, 0x0000, 0x0001, 0x0001 },
        /* 132 [31] */ {  32,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x0002a100, 0x020d, 0x0000, 0x0001, 0x0001 },
        /* 133 [32] */ {  27,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x0002f100, 0x030e, 0x0000, 0x0001, 0x0001 },
        /* 134 [33] */ {  39,   34,   38,      G_IBOX,                                NONE,   NORMAL, 0x00001100, 0x0000, 0x0001, 0x030f, 0x0001 },
        /* 135 [34] */ {  35,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         66, 0x0000, 0x0000, 0x000a, 0x0001 } /* "Player # 6" */,
        /* 136 [35] */ {  36,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x00029100, 0x000b, 0x0000, 0x0001, 0x0001 },
        /* 137 [36] */ {  37,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON, SELECTED, 0x00026100, 0x010c, 0x0000, 0x0001, 0x0001 },
        /* 138 [37] */ {  38,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x0002a100, 0x020d, 0x0000, 0x0001, 0x0001 },
        /* 139 [38] */ {  33,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x0002f100, 0x030e, 0x0000, 0x0001, 0x0001 },
        /* 140 [39] */ {  45,   40,   44,      G_IBOX,                                NONE,   NORMAL, 0x00001100, 0x0000, 0x0002, 0x030f, 0x0001 },
        /* 141 [40] */ {  41,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         67, 0x0000, 0x0000, 0x000a, 0x0001 } /* "Player # 7" */,
        /* 142 [41] */ {  42,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x00029100, 0x000b, 0x0000, 0x0001, 0x0001 },
        /* 143 [42] */ {  43,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x00026100, 0x010c, 0x0000, 0x0001, 0x0001 },
        /* 144 [43] */ {  44,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON, SELECTED, 0x0002a100, 0x020d, 0x0000, 0x0001, 0x0001 },
        /* 145 [44] */ {  39,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x0002f100, 0x030e, 0x0000, 0x0001, 0x0001 },
        /* 146 [45] */ {  26,   46,   50,      G_IBOX,                                NONE,   NORMAL, 0x00001100, 0x0000, 0x0003, 0x030f, 0x0001 },
        /* 147 [46] */ {  47,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         68, 0x0000, 0x0000, 0x000a, 0x0001 } /* "Player # 8" */,
        /* 148 [47] */ {  48,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x00029100, 0x000b, 0x0000, 0x0001, 0x0001 },
        /* 149 [48] */ {  49,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x00026100, 0x010c, 0x0000, 0x0001, 0x0001 },
        /* 150 [49] */ {  50,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x0002a100, 0x020d, 0x0000, 0x0001, 0x0001 },
        /* 151 [50] */ {  45,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON, SELECTED, 0x0002f100, 0x030e, 0x0000, 0x0001, 0x0001 },
        /* 152 [51] */ {  76,   52,   70,       G_BOX,                                NONE,   NORMAL, 0x00001100, 0x0001, 0x0009, 0x0010, 0x0004 },
        /* 153 [52] */ {  58,   53,   57,      G_IBOX,                                NONE,   NORMAL, 0x00001100, 0x0000, 0x0000, 0x030f, 0x0001 },
        /* 154 [53] */ {  54,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         69, 0x0000, 0x0000, 0x000a, 0x0001 } /* "Player # 9" */,
        /* 155 [54] */ {  55,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON, SELECTED, 0x00029100, 0x000b, 0x0000, 0x0001, 0x0001 },
        /* 156 [55] */ {  56,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x00026100, 0x010c, 0x0000, 0x0001, 0x0001 },
        /* 157 [56] */ {  57,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x0002a100, 0x020d, 0x0000, 0x0001, 0x0001 },
        /* 158 [57] */ {  52,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x0002f100, 0x030e, 0x0000, 0x0001, 0x0001 },
        /* 159 [58] */ {  64,   59,   63,      G_IBOX,                                NONE,   NORMAL, 0x00001100, 0x0000, 0x0001, 0x030f, 0x0001 },
        /* 160 [59] */ {  60,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         70, 0x0000, 0x0000, 0x000a, 0x0001 } /* "Player #10" */,
        /* 161 [60] */ {  61,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x00029100, 0x000b, 0x0000, 0x0001, 0x0001 },
        /* 162 [61] */ {  62,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON, SELECTED, 0x00026100, 0x010c, 0x0000, 0x0001, 0x0001 },
        /* 163 [62] */ {  63,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x0002a100, 0x020d, 0x0000, 0x0001, 0x0001 },
        /* 164 [63] */ {  58,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x0002f100, 0x030e, 0x0000, 0x0001, 0x0001 },
        /* 165 [64] */ {  70,   65,   69,      G_IBOX,                                NONE,   NORMAL, 0x00001100, 0x0000, 0x0002, 0x030f, 0x0001 },
        /* 166 [65] */ {  66,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         71, 0x0000, 0x0000, 0x000a, 0x0001 } /* "Player #11" */,
        /* 167 [66] */ {  67,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x00029100, 0x000b, 0x0000, 0x0001, 0x0001 },
        /* 168 [67] */ {  68,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x00026100, 0x010c, 0x0000, 0x0001, 0x0001 },
        /* 169 [68] */ {  69,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON, SELECTED, 0x0002a100, 0x020d, 0x0000, 0x0001, 0x0001 },
        /* 170 [69] */ {  64,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x0002f100, 0x030e, 0x0000, 0x0001, 0x0001 },
        /* 171 [70] */ {  51,   71,   75,      G_IBOX,                                NONE,   NORMAL, 0x00001100, 0x0000, 0x0003, 0x030f, 0x0001 },
        /* 172 [71] */ {  72,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         72, 0x0000, 0x0000, 0x000a, 0x0001 } /* "Player #12" */,
        /* 173 [72] */ {  73,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x00029100, 0x000b, 0x0000, 0x0001, 0x0001 },
        /* 174 [73] */ {  74,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x00026100, 0x010c, 0x0000, 0x0001, 0x0001 },
        /* 175 [74] */ {  75,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x0002a100, 0x020d, 0x0000, 0x0001, 0x0001 },
        /* 176 [75] */ {  70,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON, SELECTED, 0x0002f100, 0x030e, 0x0000, 0x0001, 0x0001 },
        /* 177 [76] */ { 101,   77,   95,       G_BOX,                                NONE,   NORMAL, 0x00001100, 0x0001, 0x000d, 0x0010, 0x0004 },
        /* 178 [77] */ {  83,   78,   82,      G_IBOX,                                NONE,   NORMAL, 0x00001100, 0x0000, 0x0000, 0x030f, 0x0001 },
        /* 179 [78] */ {  79,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         73, 0x0000, 0x0000, 0x000a, 0x0001 } /* "Player #13" */,
        /* 180 [79] */ {  80,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON, SELECTED, 0x00029100, 0x000b, 0x0000, 0x0001, 0x0001 },
        /* 181 [80] */ {  81,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x00026100, 0x010c, 0x0000, 0x0001, 0x0001 },
        /* 182 [81] */ {  82,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x0002a100, 0x020d, 0x0000, 0x0001, 0x0001 },
        /* 183 [82] */ {  77,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x0002f100, 0x030e, 0x0000, 0x0001, 0x0001 },
        /* 184 [83] */ {  89,   84,   88,      G_IBOX,                                NONE,   NORMAL, 0x00001100, 0x0000, 0x0001, 0x030f, 0x0001 },
        /* 185 [84] */ {  85,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         74, 0x0000, 0x0000, 0x000a, 0x0001 } /* "Player #14" */,
        /* 186 [85] */ {  86,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x00029100, 0x000b, 0x0000, 0x0001, 0x0001 },
        /* 187 [86] */ {  87,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON, SELECTED, 0x00026100, 0x010c, 0x0000, 0x0001, 0x0001 },
        /* 188 [87] */ {  88,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x0002a100, 0x020d, 0x0000, 0x0001, 0x0001 },
        /* 189 [88] */ {  83,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x0002f100, 0x030e, 0x0000, 0x0001, 0x0001 },
        /* 190 [89] */ {  95,   90,   94,      G_IBOX,                                NONE,   NORMAL, 0x00001100, 0x0000, 0x0002, 0x030f, 0x0001 },
        /* 191 [90] */ {  91,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         75, 0x0000, 0x0000, 0x000a, 0x0001 } /* "Player #15" */,
        /* 192 [91] */ {  92,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x00029100, 0x000b, 0x0000, 0x0001, 0x0001 },
        /* 193 [92] */ {  93,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x00026100, 0x010c, 0x0000, 0x0001, 0x0001 },
        /* 194 [93] */ {  94,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON, SELECTED, 0x0002a100, 0x020d, 0x0000, 0x0001, 0x0001 },
        /* 195 [94] */ {  89,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x0002f100, 0x030e, 0x0000, 0x0001, 0x0001 },
        /* 196 [95] */ {  76,   96,  100,      G_IBOX,                                NONE,   NORMAL, 0x00001100, 0x0000, 0x0003, 0x030f, 0x0001 },
        /* 197 [96] */ {  97,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         76, 0x0000, 0x0000, 0x000a, 0x0001 } /* "Player #16" */,
        /* 198 [97] */ {  98,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x00029100, 0x000b, 0x0000, 0x0001, 0x0001 },
        /* 199 [98] */ {  99,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x00026100, 0x010c, 0x0000, 0x0001, 0x0001 },
        /* 200 [99] */ { 100,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON,   NORMAL, 0x0002a100, 0x020d, 0x0000, 0x0001, 0x0001 },
        /* 201 [100] */ {  95,   -1,   -1,       G_BOX,                  SELECTABLE|RBUTTON, SELECTED, 0x0002f100, 0x030e, 0x0000, 0x0001, 0x0001 },
        /* 202 [101] */ { 102,   -1,   -1,    G_BUTTON,             SELECTABLE|DEFAULT|EXIT,   NORMAL,         77, 0x0005, 0x0014, 0x0008, 0x0001 } /* "O-Tay!" */,
        /* 203 [102] */ {   0,   -1,   -1,    G_BUTTON,                   SELECTABLE|LASTOB,   NORMAL,         78, 0x0002, 0x0012, 0x000d, 0x0001 } /* "Friendly Fire" */,
        /* TREE 10 (204) */
        /* 204 [0] */ {  -1,    1,    2,       G_BOX,                                NONE, OUTLINED, 0x00021100, 0x0000, 0x0000, 0x000e, 0x0004 },
        /* 205 [1] */ {   2,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         79, 0x0001, 0x0001, 0x000b, 0x0101 } /* "The game is" */,
        /* 206 [2] */ {   0,   -1,   -1,    G_STRING,                              LASTOB,   NORMAL,         80, 0x0002, 0x0002, 0x000a, 0x0001 } /* "suspended." */,
        /* TREE 11 (207) */
        /* 207 [0] */ {  -1,    1,    2,       G_BOX,                                NONE, OUTLINED, 0x00021100, 0x0000, 0x0000, 0x0015, 0x0005 },
        /* 208 [1] */ {   2,   -1,   -1,  G_FBOXTEXT,                            EDITABLE,   NORMAL, 0x00000000, 0x0002, 0x0001, 0x0011, 0x0001 },
        /* 209 [2] */ {   0,   -1,   -1,    G_BUTTON,      SELECTABLE|DEFAULT|EXIT|LASTOB,   NORMAL,         84, 0x0010, 0x0003, 0x0003, 0x0001 } /* "OK" */,
        /* TREE 12 (210) */
        /* 210 [0] */ {  -1,    1,    5,       G_BOX,                                NONE, OUTLINED, 0x00021100, 0x0000, 0x0000, 0x001b, 0x0008 },
        /* 211 [1] */ {   2,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         85, 0x0007, 0x0001, 0x000c, 0x0001 } /* "MIDIMAZE 1.0" */,
        /* 212 [2] */ {   3,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         86, 0x0004, 0x0002, 0x0012, 0x0001 } /* "Copyright (c) 1987" */,
        /* 213 [3] */ {   4,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         87, 0x0009, 0x0003, 0x0009, 0x0001 } /* "XANTH F/X" */,
        /* 214 [4] */ {   5,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         88, 0x0003, 0x0004, 0x0014, 0x0001 } /* "All rights reserved." */,
        /* 215 [5] */ {   0,   -1,   -1,    G_BUTTON,      SELECTABLE|DEFAULT|EXIT|LASTOB,   NORMAL,         89, 0x000b, 0x0006, 0x0005, 0x0001 } /* "Wow!" */,
        /* TREE 13 (216) */
        /* 216 [0] */ {  -1,    1,    2,       G_BOX,                                NONE, OUTLINED, 0x00021100, 0x0000, 0x0000, 0x000d, 0x0004 },
        /* 217 [1] */ {   2,   -1,   -1,    G_STRING,                                NONE,   NORMAL,         90, 0x0002, 0x0001, 0x0009, 0x0001 } /* "This is a" */,
        /* 218 [2] */ {   0,   -1,   -1,    G_STRING,                              LASTOB,   NORMAL,         91, 0x0003, 0x0002, 0x0007, 0x0001 } /* "MIDICAM" */,
        };
/* only one TEDINFO object (editing the player name) */
TEDINFO rs_tedinfo[1] = { 
        {(char*)81,(char*)82,(char*)83,3,6,2,4480,0,-1,11,17},
        };
/* start index of all used resource trees */
static long rs_trindex[14] = { 0,23,27,31,34,37,40,43,46,101,204,207,210,216 };
static int pxy[8];
static OBJECT *rsrc_tree_ptr; /* unused */
static MFDB src_MFDB;
static MFDB dest_MFDB;
OBJECT *rsrc_object_array[14]; /* adresses of all used resource trees found at rs_trindex[] */
short wind_handle;

/************************************************************
 *** Copy the window background from logbase as a redraw
 ************************************************************/
void redraw_window_background(int wi_ghandle) {
int wi_gw_h;
int wi_gw_w;
int wi_gw_y;
int wi_gw_x;
int wi_gfield;

    wind_update(BEG_UPDATE);
    wi_gfield = WF_FIRSTXYWH;
    do {
        wind_get(wi_ghandle, wi_gfield, &wi_gw_x, &wi_gw_y, &wi_gw_w, &wi_gw_h);
        if(wi_gw_w == 0 && wi_gw_h == 0) break;
        pxy[0] = pxy[4] = wi_gw_x;
        pxy[1] = pxy[5] = wi_gw_y;
        pxy[2] = pxy[6] = wi_gw_x+wi_gw_w-1;
        pxy[3] = pxy[7] = wi_gw_y+wi_gw_h-1;
        vro_cpyfm(wi_ghandle, S_ONLY, pxy, &src_MFDB, &dest_MFDB);
        wi_gfield = WF_NEXTXYWH;
    } while(1);
    wind_update(END_UPDATE);
}

/************************************************************
 *** int init_aes_window(void)
 ************************************************************/
int init_aes_window(void) {
int index;
int obj;

    rs_object[6].ob_x--; /* fix the position of the Atari menu title box */
    index = 0;
    rsrc_tree_ptr = rs_object; /* unused */
    /* fix the whole resource tree */
    for(obj = 0; obj < NUM_OBS; obj++) {
        rsrc_obfix(rs_object, obj);
        if(rs_object[obj].ob_type == G_TITLE || rs_object[obj].ob_type == G_STRING || rs_object[obj].ob_type == G_BUTTON) {
            rs_object[obj].ob_spec = (long)rs_strings[index++];
        } else if(rs_object[obj].ob_type == G_FBOXTEXT) {
            index += 3; /* that is setup further down */
        }
    }
    /* generate an address table for all used trees */
    for(obj = 0; obj < NUM_TREE; obj++)
        rsrc_object_array[obj] = rs_trindex[obj]+rs_object;

    /* The background of the play dialog is transparent  */
    rsrc_object_array[RSCTREE_PLAY_DIALOG]->ob_type = G_IBOX;
    /* setup the TEDINFO for the set name dialog */
    rsrc_object_array[RSCTREE_SET_NAME][1].ob_spec = (long)&rs_tedinfo;
    rs_tedinfo[0].te_ptext = rs_strings[(long)rs_tedinfo[0].te_ptext];
    rs_tedinfo[0].te_ptmplt = rs_strings[(long)rs_tedinfo[0].te_ptmplt];
    rs_tedinfo[0].te_pvalid = rs_strings[(long)rs_tedinfo[0].te_pvalid];

    /* show the menu bar */
    menu_bar(rsrc_object_array[RSCTREE_MENU], 1);

    /* copy from logbase to Physbase() to restore the background in a redraw */
    src_MFDB.fd_addr = screen_ptr[0];
    dest_MFDB.fd_addr = Physbase();
    src_MFDB.fd_w = dest_MFDB.fd_w = screen_rez ? SCREEN_BW_WIDTH : SCREEN_COL_WIDTH;
    src_MFDB.fd_h = dest_MFDB.fd_h = screen_rez ? SCREEN_BW_HEIGHT : SCREEN_COL_HEIGHT;
    src_MFDB.fd_wdwidth = dest_MFDB.fd_wdwidth = screen_rez ? (SCREEN_BW_WIDTH/16) : (SCREEN_COL_WIDTH/16);
    src_MFDB.fd_stand = dest_MFDB.fd_stand = 0;
    src_MFDB.fd_nplanes = dest_MFDB.fd_nplanes = screen_rez ? 1 : 4;
    pxy[0] = 0; pxy[1] = 0; pxy[2] = screen_rez ? (SCREEN_BW_WIDTH-1) : (SCREEN_COL_WIDTH-1); pxy[3] = screen_rez ? (SCREEN_BW_HEIGHT-1) : (SCREEN_COL_HEIGHT-1);
    pxy[4] = 0; pxy[5] = 0; pxy[6] = screen_rez ? (SCREEN_BW_WIDTH-1) : (SCREEN_COL_WIDTH-1); pxy[7] = screen_rez ? (SCREEN_BW_HEIGHT-1) : (SCREEN_COL_HEIGHT-1);

    /* Create a window. The window is larger than the screen to hide the borders */
    wind_handle = wind_create(0, -1, screen_rez ? 18 : 10, screen_rez ? (SCREEN_BW_WIDTH+2) : (SCREEN_COL_WIDTH+2), screen_rez ? (SCREEN_BW_HEIGHT-9) : (SCREEN_COL_HEIGHT-9));
    if(wind_handle < 0) {
        form_alert(1, "[3][ |Can't open| window!][OK]");
        return FAILURE;
    }
    wind_open(wind_handle, -1, screen_rez ? 18 : 10, screen_rez ? (SCREEN_BW_WIDTH+2) : (SCREEN_COL_WIDTH+2), screen_rez ? (SCREEN_BW_HEIGHT-9) : (SCREEN_COL_HEIGHT-9));

    return SUCCESS;
}

/************************************************************
 *** void exit_aes_window(void)
 ************************************************************/
void exit_aes_window(void) {
    wind_close(wind_handle);
    wind_delete(wind_handle);
    /* hide the menu bar */
    menu_bar(rsrc_object_array[RSCTREE_MENU], 0);
}
