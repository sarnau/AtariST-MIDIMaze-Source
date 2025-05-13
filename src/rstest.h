/*
 * resource set indices for rstest
 *
 * created by ORCS 2.18
 */

/*
 * Number of Strings:        95
 * Number of Bitblks:        0
 * Number of Iconblks:       0
 * Number of Color Iconblks: 0
 * Number of Color Icons:    0
 * Number of Tedinfos:       1
 * Number of Free Strings:   0
 * Number of Free Images:    0
 * Number of Objects:        223
 * Number of Trees:          15
 * Number of Userblks:       0
 * Number of Images:         0
 * Total file size:          6482
 */

#ifdef RSC_NAME
#undef RSC_NAME
#endif
#ifndef __ALCYON__
#define RSC_NAME "rstest"
#endif
#ifdef RSC_ID
#undef RSC_ID
#endif
#ifdef rstest
#define RSC_ID rstest
#else
#define RSC_ID 0
#endif

#ifndef RSC_STATIC_FILE
# define RSC_STATIC_FILE 0
#endif
#if !RSC_STATIC_FILE
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
#endif



#define RSCTREE_MENU                       0 /* menu */
#define TITLE_ABOUT                        3 /* TITLE in tree RSCTREE_MENU */
#define TITLE_MAZE                         4 /* TITLE in tree RSCTREE_MENU */
#define ABOUT                              7 /* STRING in tree RSCTREE_MENU */
#define MAZE_LOAD                         16 /* STRING in tree RSCTREE_MENU */
#define MAZE_RESET_SCORE                  17 /* STRING in tree RSCTREE_MENU */
#define MAZE_PLAY                         19 /* STRING in tree RSCTREE_MENU */
#define MAZE_SET_NAMES                    20 /* STRING in tree RSCTREE_MENU */
#define MAZE_QUIT                         22 /* STRING in tree RSCTREE_MENU */

#define RSCTREE_SLAVE                      1 /* free form */

#define RSCTREE_MIDIRING_TIMEOUT           2 /* free form */

#define RSCTREE_GAME_TERMINATED            3 /* free form */

#define RSCTREE_MAZE_ERROR                 4 /* free form */

#define RSCTREE_MOUSE_CTRL                 5 /* free form */

#define RSCTREE_JOYSTICK_CTRL              6 /* free form */

#define RSCTREE_NUMBER_XX                  7 /* free form */

#define RSCTREE_PLAY_DIALOG                8 /* form/dialog */
#define DUMB_BOX                          10 /* IBOX in tree RSCTREE_PLAY_DIALOG */
#define DUMB_UP                           11 /* BOXCHAR in tree RSCTREE_PLAY_DIALOG */
#define DUMB_DOWN                         12 /* BOXCHAR in tree RSCTREE_PLAY_DIALOG */
#define DUMB_VAL                          14 /* STRING in tree RSCTREE_PLAY_DIALOG */
#define PLAIN_BOX                         15 /* IBOX in tree RSCTREE_PLAY_DIALOG */
#define PLAIN_UP                          16 /* BOXCHAR in tree RSCTREE_PLAY_DIALOG */
#define PLAIN_DOWN                        17 /* BOXCHAR in tree RSCTREE_PLAY_DIALOG */
#define PLAIN_VAL                         19 /* STRING in tree RSCTREE_PLAY_DIALOG */
#define NINJA_BOX                         20 /* IBOX in tree RSCTREE_PLAY_DIALOG */
#define NINJA_UP                          21 /* BOXCHAR in tree RSCTREE_PLAY_DIALOG */
#define NINJA_DOWN                        22 /* BOXCHAR in tree RSCTREE_PLAY_DIALOG */
#define NINJA_VAL                         24 /* STRING in tree RSCTREE_PLAY_DIALOG */
#define RELOAD_FAST                       29 /* BUTTON in tree RSCTREE_PLAY_DIALOG */
#define RELOAD_SLOW                       30 /* BUTTON in tree RSCTREE_PLAY_DIALOG */
#define REGEN_SLOW                        33 /* BUTTON in tree RSCTREE_PLAY_DIALOG */
#define REGEN_FAST                        34 /* BUTTON in tree RSCTREE_PLAY_DIALOG */
#define REVIVE_SLOW                       38 /* BUTTON in tree RSCTREE_PLAY_DIALOG */
#define REVIVE_FAST                       39 /* BUTTON in tree RSCTREE_PLAY_DIALOG */
#define PREF_1LIFE                        42 /* BUTTON in tree RSCTREE_PLAY_DIALOG */
#define PREF_2LIVES                       43 /* BUTTON in tree RSCTREE_PLAY_DIALOG */
#define PREF_3LIVES                       44 /* BUTTON in tree RSCTREE_PLAY_DIALOG */
#define PREF_TITLE                        45 /* BUTTON in tree RSCTREE_PLAY_DIALOG */
#define PREF_NAH                          48 /* BUTTON in tree RSCTREE_PLAY_DIALOG */
#define PREF_OK                           49 /* BUTTON in tree RSCTREE_PLAY_DIALOG */
#define PREF_SINGLES                      53 /* BUTTON in tree RSCTREE_PLAY_DIALOG */
#define PREF_TEAMS                        54 /* BUTTON in tree RSCTREE_PLAY_DIALOG */

#define RSCTREE_TEAM_DIALOG                9 /* form/dialog */
#define PLAYER_1                           3 /* STRING in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_1_TEAM_1                    4 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_1_TEAM_2                    5 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_1_TEAM_3                    6 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_1_TEAM_4                    7 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_2                           9 /* STRING in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_2_TEAM_1                   10 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_2_TEAM_2                   11 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_2_TEAM_3                   12 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_2_TEAM_4                   13 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_3                          15 /* STRING in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_3_TEAM_1                   16 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_3_TEAM_2                   17 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_3_TEAM_3                   18 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_3_TEAM_4                   19 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_4                          21 /* STRING in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_4_TEAM_1                   22 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_4_TEAM_2                   23 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_4_TEAM_3                   24 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_4_TEAM_4                   25 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_5                          28 /* STRING in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_5_TEAM_1                   29 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_5_TEAM_2                   30 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_5_TEAM_3                   31 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_5_TEAM_4                   32 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_6                          34 /* STRING in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_6_TEAM_1                   35 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_6_TEAM_2                   36 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_6_TEAM_3                   37 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_6_TEAM_4                   38 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_7                          40 /* STRING in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_7_TEAM_1                   41 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_7_TEAM_2                   42 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_7_TEAM_3                   43 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_7_TEAM_4                   44 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_8                          46 /* STRING in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_8_TEAM_1                   47 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_8_TEAM_2                   48 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_8_TEAM_3                   49 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_8_TEAM_4                   50 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_9                          53 /* STRING in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_9_TEAM_1                   54 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_9_TEAM_2                   55 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_9_TEAM_3                   56 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_9_TEAM_4                   57 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_10                         59 /* STRING in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_10_TEAM_1                  60 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_10_TEAM_2                  61 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_10_TEAM_3                  62 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_10_TEAM_4                  63 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_11                         65 /* STRING in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_11_TEAM_1                  66 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_11_TEAM_2                  67 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_11_TEAM_3                  68 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_11_TEAM_4                  69 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_12                         71 /* STRING in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_12_TEAM_1                  72 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_12_TEAM_2                  73 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_12_TEAM_3                  74 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_12_TEAM_4                  75 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_13                         78 /* STRING in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_13_TEAM_1                  79 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_13_TEAM_2                  80 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_13_TEAM_3                  81 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_13_TEAM_4                  82 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_14                         84 /* STRING in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_14_TEAM_1                  85 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_14_TEAM_2                  86 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_14_TEAM_3                  87 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_14_TEAM_4                  88 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_15                         90 /* STRING in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_15_TEAM_1                  91 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_15_TEAM_2                  92 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_15_TEAM_3                  93 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_15_TEAM_4                  94 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_16                         96 /* STRING in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_16_TEAM_1                  97 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_16_TEAM_2                  98 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_16_TEAM_3                  99 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define PLAYER_16_TEAM_4                 100 /* BOX in tree RSCTREE_TEAM_DIALOG */
#define TEAM_OK                          101 /* BUTTON in tree RSCTREE_TEAM_DIALOG */
#define FRIENDLY_FIRE                    102 /* BUTTON in tree RSCTREE_TEAM_DIALOG */

#define RSCTREE_SUSPENDED                 10 /* free form */

#define RSCTREE_SET_NAME                  11 /* form/dialog */
#define SET_NAME_PLAYER                    1 /* FBOXTEXT in tree RSCTREE_SET_NAME */
#define SET_NAME_OK                        2 /* BUTTON in tree RSCTREE_SET_NAME */

#define RSCTREE_ABOUT                     12 /* form/dialog */
#define ABOUT_WOW                          5 /* BUTTON in tree RSCTREE_ABOUT */
#define ABOUT_COMPILER                     6 /* STRING in tree RSCTREE_ABOUT */

#define RSCTREE_MIDICAM                   13 /* free form */

#define RSCTREE_KEYBOARD_CTRL             14 /* free form */




#ifdef __STDC__
#ifndef _WORD
#  ifdef WORD
#    define _WORD WORD
#  else
#    define _WORD short
#  endif
#endif
extern _WORD rstest_rsc_load(_WORD wchar, _WORD hchar);
extern _WORD rstest_rsc_gaddr(_WORD type, _WORD idx, void *gaddr);
extern _WORD rstest_rsc_free(void);
#endif
