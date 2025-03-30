// Defines

// color palette used for Midi Maze
#define COLOR_BLACK_INDEX       0
#define COLOR_SILVER_INDEX      1
#define COLOR_MAGNESIUM_INDEX   2
#define COLOR_BLUE_INDEX        3
#define COLOR_GREY_INDEX        4
#define COLOR_DKGREEN_INDEX     5
#define COLOR_ALUMINIUM_INDEX   6
#define COLOR_STEEL_INDEX       7
#define COLOR_YELLOW_INDEX      8
#define COLOR_ORANGE_INDEX      9
#define COLOR_PURPLE_INDEX      10
#define COLOR_MAGENTA_INDEX     11
#define COLOR_LTBLUE_INDEX      12
#define COLOR_GREEN_INDEX       13
#define COLOR_RED_INDEX         14
#define COLOR_WHITE_INDEX       15

#define DRONE_TARGET 'r' // #114 Very Dumb
#define DRONE_STANDARD 'l' // #108 Plain Dumb
#define DRONE_NINJA 'k' // #107 Not so Dumb

// MIDI bytes for special actions
#define MIDI_COUNT_PLAYERS      0x80 // Detect number of players in the MIDI ring
#define MIDI_RESET_SCORE        0x81 // Reset all scores
#define MIDI_TERMINATE_GAME     0x82 // Game terminated by the Master
#define MIDI_SEND_DATA          0x83 // after MIDI_START_GAME this package contains all shared data
#define MIDI_START_GAME         0x84 // Start the game (followed by MIDI_SEND_DATA)
#define MIDI_ABOUT              0x85 // Show about box
#define MIDI_NAME_DIALOG        0x86 // Allow all players to enter their name

#define MIDI_DEFAULT_TIMEOUT    20

// Player directions
#define PLAYER_DIR_NORTH        0x00
#define PLAYER_DIR_NORTHEAST    0x20
#define PLAYER_DIR_EAST         0x40
#define PLAYER_DIR_SOUTHEAST    0x60
#define PLAYER_DIR_SOUTH        0x80
#define PLAYER_DIR_SOUTHWEST    0xa0
#define PLAYER_DIR_WEST         0xc0
#define PLAYER_DIR_NORTHWEST    0xe0

#define NUM_STRINGS 92
#define NUM_FRSTR 1
#define NUM_IMAGES 0
#define NUM_BB 0
#define NUM_FRIMG 0
#define NUM_IB 0
#define NUM_TI 0
#define NUM_OBS 219
#define NUM_TREE 14

#define RSCTREE_MENU 0
#define RSCTREE_SLAVE 1
#define RSCTREE_MIDIRING_TIMEOUT 2
#define RSCTREE_GAME_TERMINATED 3
#define RSCTREE_MAZE_ERROR 4
#define RSCTREE_MOUSE_CTRL 5
#define RSCTREE_JOYSTICK_CTRL 6
#define RSCTREE_NUMBER_XX 7
#define RSCTREE_PLAY_DIALOG 8
#define RSCTREE_TEAM_DIALOG 9
#define RSCTREE_SUSPENDED 10
#define RSCTREE_SET_NAME 11
#define RSCTREE_ABOUT 12
#define RSCTREE_MIDICAM 13

// @FILE:DRAWPLAYERS.C

#define SHAPE_COUNT			24
#define FACE_COUNT			20

// @FILE:DRAW2D.C

#define MAP_BORDER			(screen_rez ? 6 : 3)
#define MAP_WIDTH			(screen_rez ? (320-6*2-1) : (160-3*2-1))
#define MAP_HEIGHT			(screen_rez ? (200-6*2) : (100-3*2))

// Do not write these out
// @FILE:

// Global Types
typedef struct
{
	long		cb_pcontrol; /* 0 */
	long		cb_pglobal; /* 4 */
	long		cb_pintin; /* 8 */
	long		cb_pintout; /* 12 */
	long		cb_padrin; /* 16 */
	long		cb_padrout; /* 20 */
} CBLK; /* 24 */


typedef struct
{
    void    *kb_midivec;     /* 0 */
    void    *kb_vkbderr;     /* 4 */
    void    *kb_vmiderr;     /* 8 */
    void    *kb_statvec;     /* 12 */
    void    *kb_mousevec;     /* 16 */
    void    *kb_clockvec;     /* 20 */
    void    *kb_joyvec;     /* 24 */
    void    *kb_midisys;     /* 28 */
    void    *kb_kbdsys;     /* 32 */
} KBDVBASE;     /* 36 */

typedef struct
{
    int mf_xhot; /* 0 */
    int mf_yhot; /* 2 */
    int mf_nplanes; /* 4 */
    int mf_fg; /* 6 */
    int mf_bg; /* 8 */
    int mf_mask[16]; /* 10 */
    int mf_data[16]; /* 42 */
} MFORM; /* 74 */

typedef struct
{
    void *fd_addr; /* 0 */
    int fd_w; /* 4 */
    int fd_h; /* 6 */
    int fd_wdwidth; /* 8 */
    int fd_stand; /* 10 */
    int fd_nplanes; /* 12 */
    int fd_rl; /* 14 */
    int fd_r2; /* 16 */
    int fd_r3; /* 18 */
} MFDB; /* 20 */

typedef struct
{
    int ob_next; /* 0 */
    int ob_head; /* 2 */
    int ob_tail; /* 4 */
    unsigned int ob_type; /* 6 */
    unsigned int ob_flags; /* 8 */
    unsigned int ob_state; /* 10 */
    long ob_spec; /* 12 */
    int ob_x; /* 16 */
    int ob_y; /* 18 */
    int ob_width; /* 20 */
    int ob_height; /* 22 */
} OBJECT; /* 24 */

typedef struct
{
    char    *te_ptext;        /* 0 */
    char    *te_ptmplt;       /* 4 */
    char    *te_pvalid;       /* 8 */
    int    te_font;           /* 12 */
    int    te_fontid;         /* 14 */
    int    te_just;           /* 16 */
    int    te_color;          /* 18 */
    int    te_fontsize;       /* 20 */
    int    te_thickness;      /* 22 */
    int    te_txtlen;         /* 24 */
    int    te_tmplen;         /* 26 */
} TEDINFO; /* 28 */

typedef struct
{
	int	dummy; /* 0 */
	int	*image; /* 2 */
} foobar; /* 6 */

// @FILE: GLOBALS.H

typedef struct
{
    int y; /* 0 */
    int x; /* 2 */
} PLAYER_COORD; /* 4 */

typedef struct
{
    int ply_y; /* 0 */
    int ply_x; /* 2 */
    int ply_dir; /* 4 */
    int ply_unused_6; /* 6 */
    int ply_unused_8; /* 8 */
    int ply_lives; /* 10 */
    int ply_refresh; /* 12 */
    int ply_hitflag; /* 14 */
    int ply_reload; /* 16 */
    int ply_score; /* 18 */
    int ply_gunman; /* 20 */
    int ply_looser; /* 22 */
    int ply_team; /* 24 */
    int dr_type; /* 26 */
    int dr_rotateTimerA; /* 28 */
    int dr_rotateTimerB; /* 30 */
    int dr_joystick; /* 32 */
    int dr_isInactive; /* 34 */
    int dr_dir[6]; /* 36 */
    PLAYER_COORD dr_field[6]; /* 48 */
    int dr_fieldIndex; /* 72 */
    int dr_fieldResetTimer; /* 74 */
    int dr_targetLocked; /* 76 */
    int dr_fireDirection; /* 78 */
    int dr_humanEnemies[18]; /* 80 */
    int dr_currentTarget; /* 116 */
    int dr_permanentTarget; /* 118 */
    int ply_plist; /* 120 */
    int ply_shooty; /* 122 */
    int ply_shootx; /* 124 */
    int ply_shootr; /* 126 */
    int ply_shoot; /* 128 */
    int ply_slist; /* 130 */
} PLAYER_DATA; /* 132 */

typedef struct
{
    int deltaY; /* 0 */
    int deltaX; /* 2 */
} XY_SPEED_TABLE; /* 4 */

// @FILE: MIDICAM.C

typedef struct
{
    int dY; /* 0 */
    int dX; /* 2 */
    int dir; /* 4 */
} POS_STRUCT; /* 6 */

// @FILE: SAVESCREEN.C

typedef struct
{
    int offset; /* 0 */
    int lines; /* 2 */
    int words; /* 4 */
} SAVEAREA; /* 6 */

// @FILE: MAZE.C

typedef struct
{
    int distance; /* 0 */
    int xOffset; /* 2 */
    int player; /* 4 */
} PLAYER_POS; /* 6 */

typedef struct
{
    int minY; /* 0 */
    int minX; /* 2 */
    int maxY; /* 4 */
    int maxX; /* 6 */
    int fieldOffsetY; /* 8 */
    int fieldOffsetX; /* 10 */
    int flipped; /* 12 */
} DIR_STRUCT; /* 14 */

typedef struct
{
    short y; /* 0 */
    short x; /* 2 */
    short index; /* 4 */
} OBJECT_TABLE_ENTRY; /* 6 */

// @FILE: DRAWWALLS.C

typedef struct
{
    short xleft; /* 0 */
    short xright; /* 2 */
} TABLE_LIST_XCOORD; /* 4 */

// @FILE: DRAW3D.C

typedef struct
{
    int type; /* 0 */
    int sprite_wallcolor; /* 2 */
    int x; /* 4 */
    int h; /* 6 */
    int x2_size; /* 8 */
    int h2_color; /* 10 */
} ELEM_LIST_STRUCT; /* 12 */

// Global Variables

long crystal(void *) /* L0000 */
{
}

// @FILE: LOWMEM.C
void sub_read_vbclock(); /* L0001 */
{
}

int read_vbclock(); /* L0002 */
{
}

// @FILE: READMIDI.C
int get_midi(int timeout); /* L0004 */
{
}

// @FILE: MAIN.C
void main(); /* L0009 */
{
int intin[11]; /* -22(A6) */
int intout[57]; /* -136(A6) */
int savedColors[16]; /* -168(A6) */
int i; /* -170(A6) */
}

// @FILE: RUNGAME.C
void run_game(); /* L000E */
{
int i; /* -2(A6) */
}

// @FILE: DRONE.C

void calc_drone_angle_table(); /* L0025 */
{
int i; /* -2(A6) */
int y; /* -4(A6) */
int x; /* -6(A6) */
}

// @FILE: RUNGAME.C

int load_datas(); /* L002D */
{
}

// @FILE: MAZE.C

void init_dirtable(); /* L0030 */
{
int i; /* -2(A6) */
}

void make_draw_list(int y,int x,int dir); /* L0033 */
{
int compassDir; /* -2(A6) */
}

// @FILE: SCREEN.C

int init_screen(); /* L0034 */
{
register unsigned short *srcScreenPtr; /* A4 */
register unsigned short *destScreenPtr; /* A5 */
register long i; /* D7 */

}

int load_color_titlescreen(); /* L003D */
{
}

int load_bw_titlescreen(); /* L003F */
{
}

void set_screen_offs(int offset); /* L0041 */
{
}

void switch_screens(); /* L0042 */
{
}

void switch_logbase(); /* L0043 */
{
}

void switch_org_scr(); /* L0044 */
{
}

// @FILE: SOUND.C

void init_sound(); /* L0045 */
{
int i; /* -2(A6) */
}

void exit_sound(); /* L0048 */
{
int i; /* -2(A6) */
}

// @FILE: DISPATCH.C

void dispatch(); /* L004B */
{
int action; /* -2(A6) */
}

// @FILE: ENDSHAPES.C

void init_end_shape(); /* L0059 */
{
}

// @FILE: MAINGAME.C

int game_loop(int isSolo,int isJoystick); /* L005B */
{
int currentScore; /* -2(A6) */
int i; /* -4(A6) */
int j; /* -6(A6) */
int k; /* -8(A6) */
int tempVar; /* -10(A6) */
int joystickDirection; /* -12(A6) */
int unused_14; /* -14(A6) */
int playerIndex; /* -16(A6) */
int unused_18; /* -18(A6) */
int midiCamRotateIndex; /* -20(A6) */
int savedDisplay_2d_map_flag; /* -22(A6) */
int joystickButton; /* -24(A6) */
int lastJoystickButton; /* -26(A6) */
int bwColorFlag; /* -28(A6) */
}

// @FILE: MIDICAM.C

void midicam_keyboard_shortcuts(); /* L0142 */
{
int key; /* -2(A6) */
int newDisplay2DMapFlag; /* -4(A6) */
}

int midicam_find_next_player(); /* L0160 */
{
int i; /* -2(A6) */
int anyPlayerAliveFlag; /* -4(A6) */
}

void position_midicam_users(); /* L016D */
{
int midicam; /* -2(A6) */
int fieldY; /* -4(A6) */
int fieldX; /* -6(A6) */
int wallCount; /* -8(A6) */
int notdone; /* -10(A6) */
int randDir; /* -12(A6) */
}

// @FILE: KILLBOARD.C

void add_one_smily(int score,int shotPlayer); /* L0176 */
{
int x; /* -2(A6) */
int y; /* -4(A6) */
int imageOffset; /* -6(A6) */
int imageWidth; /* -8(A6) */
int imageHeight; /* -10(A6) */
unsigned short *imageMask; /* -14(A6) */
}

// @FILE: MORESHAPES.C

void flip_crossedsmil_img(); /* L0184 */
{
int i; /* -2(A6) */
int bits; /* -4(A6) */
unsigned long orgBitMask; /* -8(A6) */
unsigned long newBitMask; /* -12(A6) */
unsigned long *imageMaskPtr; /* -16(A6) */
}

void init_some_shp(); /* L018B */
{
}

// @FILE: NOTEBOARD.C

void draw_one_note_score(int player,int score); /* L018D */
{
int x; /* -2(A6) */
int y; /* -4(A6) */
int i; /* -6(A6) */
}

void draw_all_notes_score(int playerCount); /* L01A2 */
{
int i; /* -2(A6) */
}

void update_notes_score(int playerCount); /* L01A8 */
{
int i; /* -2(A6) */
int notdone; /* -4(A6) */
}

// @FILE: SAVESCREEN.C

void copy_screen(); /* L01B1 */
{
register unsigned short *dest; /* A4 */
register unsigned short *src; /* A5 */
register int i; /* D7 */

}

void save_part_of_screen_for_dialog(int saveFlag); /* L01B5 */
{
register long bufferIndex; /* D4 */
register long wordIndex; /* D5 */
register long lineIndex; /* D6 */
register long areaIndex; /* D7 */
register unsigned short *scrPtr; /* A5 */

SAVEAREA saveAreas[2]; /* -12(A6) */
}

// @FILE: JOYSTICKMOUSE.C

int ask_mouse(); /* L01C8 */
{
int savedDX; /* -2(A6) */
int joystickMask; /* -4(A6) */
}

// @FILE: MISCOUTPUT.C

void bconout_dec_number(int num); /* L01D4 */
{
}

void bconout_string(const char *str); /* L01D7 */
{
}

void midicam_print_current_player(); /* L01DA */
{
int i; /* -2(A6) */
}

// @FILE: DRAW2D.C

void draw_2Dmap(); /* L01E3 */
{
int y; /* -2(A6) */
int x; /* -4(A6) */
int xy1; /* -6(A6) */
int xy2; /* -8(A6) */
int xyc; /* -10(A6) */
}

void set_ply_2Dmap(int player); /* L021E */
{
int playerY; /* -6(A6) */
int playerX; /* -8(A6) */
int posX; /* -10(A6) */
int posY; /* -12(A6) */
}

// @FILE: DRONE.C

void drone_setup(int humanPlayers); /* L0229 */
{
int humanSoloPlayerList[18]; /* -40(A6) */
int currentHumanSoloPlayer; /* -42(A6) */
int allPlayerCount; /* -44(A6) */
int team0Index; /* -46(A6) */
int playerIndex; /* -48(A6) */
int team0HasMembers; /* -50(A6) */
int team1HasMembers; /* -52(A6) */
int team2HasMembers; /* -54(A6) */
int team3HasMembers; /* -56(A6) */
int team0[18]; /* -92(A6) */
int team1[18]; /* -128(A6) */
int team2[18]; /* -164(A6) */
int team3[18]; /* -200(A6) */
int team1Index; /* -202(A6) */
int team2Index; /* -204(A6) */
int team3Index; /* -206(A6) */
int teamCount; /* -208(A6) */
int team0Attackable; /* -210(A6) */
int team1Attackable; /* -212(A6) */
int team2Attackable; /* -214(A6) */
int team3Attackable; /* -216(A6) */
}

void drone_action(int player); /* L029A */
{
int i; /* -2(A6) */
int target_player; /* -4(A6) */
}

int drone_move(int player); /* L02B3 */
{
int fireButtonMaskAlwaysZero; /* -2(A6) */
}

void drone_check_directions(int player,int *canNorthPtr,int *canSouthPtr,int *canEastPtr,int *canWestPtr,int useAltCoord,int altYField,int altXField); /* L02CA */
{
int y; /* -2(A6) */
int x; /* -4(A6) */
int yField; /* -6(A6) */
int xField; /* -8(A6) */
}

void drone_sub_findMoveToTarget(int player); /* L02D1 */
{
int targetDistanceY; /* -2(A6) */
int targetDistanceX; /* -4(A6) */
int playerY; /* -6(A6) */
int playerX; /* -8(A6) */
int targetY; /* -10(A6) */
int targetX; /* -12(A6) */
int target_player; /* -14(A6) */
}

void drone_sub_calc_ninja(int player); /* L02E9 */
{
int deltaY; /* -14(A6) */
int deltaX; /* -16(A6) */
int playerFieldY; /* -18(A6) */
int playerFieldX; /* -20(A6) */
int targetFieldY; /* -22(A6) */
int targetFieldX; /* -24(A6) */
int fieldIndex; /* -30(A6) */
int target_player; /* -32(A6) */
}

void drone_generate_joystickdata(int player); /* L0361 */
{
int dir; /* -2(A6) */
}

int drone_aim2target(int player); /* L0386 */
{
int targetFieldY; /* -2(A6) */
int targetFieldX; /* -4(A6) */
int playerFieldY; /* -6(A6) */
int playerFieldX; /* -8(A6) */
int unused; /* -10(A6) */
int deltaY; /* -12(A6) */
int deltaX; /* -14(A6) */
int target_player; /* -18(A6) */
}

int drone_delta_into_direction(int deltaY,int deltaX); /* L038F */
{
int yPositive; /* -2(A6) */
int xPositive; /* -4(A6) */
int angle; /* -6(A6) */
}

int drone_isTargetIsVisibleNorth(int player,int fieldY,int fieldX); /* L039F */
{
int targetFieldY; /* -2(A6) */
int targetFieldX; /* -4(A6) */
int playerFieldY; /* -6(A6) */
int playerFieldX; /* -8(A6) */
int target_player; /* -10(A6) */
}

int drone_isTargetIsVisibleSouth(int player,int fieldY,int fieldX); /* L03AE */
{
int targetFieldY; /* -2(A6) */
int targetFieldX; /* -4(A6) */
int playerFieldY; /* -6(A6) */
int playerFieldX; /* -8(A6) */
int target_player; /* -10(A6) */
}

int drone_isTargetIsVisibleEast(int player,int fieldY,int fieldX); /* L03BD */
{
int targetFieldY; /* -2(A6) */
int targetFieldX; /* -4(A6) */
int playerFieldY; /* -6(A6) */
int playerFieldX; /* -8(A6) */
int target_player; /* -10(A6) */
}

int drone_isTargetIsVisibleWest(int player,int fieldY,int fieldX); /* L03CC */
{
int targetFieldY; /* -2(A6) */
int targetFieldX; /* -4(A6) */
int playerFieldY; /* -6(A6) */
int playerFieldX; /* -8(A6) */
int target_player; /* -10(A6) */
}

void drone_set_position(int player,int viewCompassDirChar); /* L03DB */
{
int playerFieldY; /* -2(A6) */
int playerFieldX; /* -4(A6) */
}

void drone_sub_standard(int player); /* L03E3 */
{
int targetFieldY; /* -2(A6) */
int targetFieldX; /* -4(A6) */
int playerFieldY; /* -6(A6) */
int playerFieldX; /* -8(A6) */
int deltaY; /* -10(A6) */
int deltaX; /* -12(A6) */
int target_player; /* -14(A6) */
}

int drone_sub_ninja_a(int player,int viewCompassDirChar); /* L03FD */
{
int canNorth; /* -2(A6) */
int canSouth; /* -4(A6) */
int canEast; /* -6(A6) */
int canWest; /* -8(A6) */
int playerFieldY; /* -18(A6) */
int playerFieldX; /* -20(A6) */
int targetFieldY; /* -22(A6) */
int targetFieldX; /* -24(A6) */
int target_player; /* -26(A6) */
}

int drone_sub_ninja_b(int player,int viewCompassDirChar); /* L0528 */
{
int canNorth; /* -2(A6) */
int canSouth; /* -4(A6) */
int canEast; /* -6(A6) */
int canWest; /* -8(A6) */
int playerFieldY; /* -10(A6) */
int playerFieldX; /* -12(A6) */
int targetPlayerFieldY; /* -14(A6) */
int targetPlayerFieldX; /* -16(A6) */
int unused_18; /* -18(A6) */
int target_player; /* -20(A6) */
int unused_22; /* -22(A6) */
int unused_24; /* -24(A6) */
int unused_26; /* -26(A6) */
}

int drone_sub_ninja_c(int player,int viewCompassDirChar); /* L0586 */
{
int canNorth; /* -2(A6) */
int canSouth; /* -4(A6) */
int canEast; /* -6(A6) */
int canWest; /* -8(A6) */
int playerFieldY; /* -10(A6) */
int playerFieldX; /* -12(A6) */
int targetPlayerFieldY; /* -14(A6) */
int targetPlayerFieldX; /* -16(A6) */
int unused_18; /* -18(A6) */
int target_player; /* -20(A6) */
int unused_22; /* -22(A6) */
int unused_24; /* -24(A6) */
int unused_26; /* -26(A6) */
}

int drone_sub_ninja_d(int player,int viewCompassDirChar); /* L05D8 */
{
int canNorth; /* -2(A6) */
int canSouth; /* -4(A6) */
int canEast; /* -6(A6) */
int canWest; /* -8(A6) */
int playerFieldY; /* -10(A6) */
int playerFieldX; /* -12(A6) */
int targetPlayerFieldY; /* -14(A6) */
int targetPlayerFieldX; /* -16(A6) */
int unused_18; /* -18(A6) */
int target_player; /* -20(A6) */
int unused_22; /* -22(A6) */
int unused_24; /* -24(A6) */
int unused_26; /* -26(A6) */
}

int drone_sub_ninja_e(int player,int viewCompassDirChar); /* L0636 */
{
int canNorth; /* -2(A6) */
int canSouth; /* -4(A6) */
int canEast; /* -6(A6) */
int canWest; /* -8(A6) */
int playerFieldY; /* -10(A6) */
int playerFieldX; /* -12(A6) */
int targetPlayerFieldY; /* -14(A6) */
int targetPlayerFieldX; /* -16(A6) */
int unused_18; /* -18(A6) */
int target_player; /* -20(A6) */
int unused_22; /* -22(A6) */
int unused_24; /* -24(A6) */
int unused_26; /* -26(A6) */
}

void drone_move_down(int player); /* L0696_UNUSED_4cf5 */
{
}

void drone_move_upright(int player); /* L0699 */
{
}

void drone_move_upleft(int player); /* L069C */
{
}

void drone_move_up(int player); /* L069F */
{
}

void drone_turn_around(int player); /* L06A2 */
{
}

// @FILE: AESINIT.C

void redraw_window_background(int wi_ghandle); /* L06A5 */
{
int wi_gfield; /* -2(A6) */
int wi_gw_x; /* -4(A6) */
int wi_gw_y; /* -6(A6) */
int wi_gw_w; /* -8(A6) */
int wi_gw_h; /* -10(A6) */
}

int init_aes_window(); /* L06A9 */
{
int obj; /* -2(A6) */
int index; /* -4(A6) */
}

void exit_aes_window(); /* L06CF */
{
}

// @FILE: MASTER.C

int master_solo_loop(int isSolo); /* L06D0 */
{
int i; /* -2(A6) */
int j; /* -4(A6) */
int dontExitLoop; /* -6(A6) */
int menuTitleId; /* -8(A6) */
int menuItemId; /* -10(A6) */
int pbutton; /* -12(A6) */
int buffer[8]; /* -28(A6) */
int prefReturnCode; /* -30(A6) */
int midiByte; /* -32(A6) */
int joystickActive; /* -34(A6) */
int key_code; /* -36(A6) */
int keyboard_state; /* -38(A6) */
int which_events; /* -40(A6) */
int returnCode; /* -42(A6) */
char filepath[64]; /* -106(A6) */
char filename[20]; /* -126(A6) */
char mazePathAndName[100]; /* -226(A6) */
}

// @FILE: PREFDIALOG.C

void rsc_flags_revive(OBJECT *tree); /* L0727 */
{
int i; /* -2(A6) */
int j; /* -4(A6) */
}

int do_preference_form(OBJECT *tree,int possibleDroneCount,int *droneCountPtr); /* L072E */
{
int i; /* -2(A6) */
int j; /* -4(A6) */
int fo_cx; /* -8(A6) */
int fo_cy; /* -10(A6) */
int fo_cw; /* -12(A6) */
int fo_ch; /* -14(A6) */
int obj_index; /* -16(A6) */
}

void update_names_in_rsc(int *droneList); /* L075C */
{
int i; /* -2(A6) */
int mindex; /* -4(A6) */
int j; /* -6(A6) */
}

// @FILE: STRING.C

void strcpy_srcdst(const char *srcPtr,char *destPtr); /* L0763 */
{
}

// @FILE: SLAVE.C

int slave_midicam_loop(int isMidicamFlag); /* L0765 */
{
int i; /* -2(A6) */
int midiByte; /* -4(A6) */
int joystickFlag; /* -6(A6) */
int dontExitLoop; /* -8(A6) */
int key_code; /* -10(A6) */
int returnCode; /* -12(A6) */
}

void slave_show_playername_dialog(int isSolo); /* L0793 */
{
int i; /* -2(A6) */
int fo_cx; /* -6(A6) */
int fo_cy; /* -8(A6) */
int fo_cw; /* -10(A6) */
int fo_ch; /* -12(A6) */
}

// @FILE: MIDICOMM.C

void midi_send_playernames(); /* L079F */
{
int i; /* -2(A6) */
int j; /* -4(A6) */
char *name; /* -8(A6) */
}

void print_playername(int player); /* L07AB */
{
int i; /* -2(A6) */
const char *name; /* -6(A6) */
}

int send_datas(); /* L07B6 */
{
int i; /* -2(A6) */
int mazeSizeInBytes; /* -4(A6) */
int midiByte; /* -6(A6) */
}

int receive_datas(); /* L07D1 */
{
int i; /* -2(A6) */
int mazeSizeInBytes; /* -4(A6) */
int midiByte; /* -6(A6) */
}

// @FILE: RSCMISC.C

void rsc_draw_buttonless_dialog(int objectIndex); /* L07E5 */
{
int fo_cx; /* -2(A6) */
int fo_cy; /* -4(A6) */
int fo_cw; /* -6(A6) */
int fo_ch; /* -8(A6) */
}

void do_about_dialog(int objectIndex); /* L07E6 */
{
int fo_cx; /* -2(A6) */
int fo_cy; /* -4(A6) */
int fo_cw; /* -6(A6) */
int fo_ch; /* -8(A6) */
}

// @FILE: CONTERM.C

void _conterm_set_shift(); /* L07E7 */
{
}

void _conterm_clear_shift(); /* L07E8 */
{
}

void _conterm_update_shift_status(int setFlag); /* L07E9 */
{
}

// @FILE: MAZE.C

int LoadMaze(const char *pathname,const char *filename); /* L07EC */
{
int fieldY; /* -2(A6) */
int fieldX; /* -4(A6) */
int fhandle; /* -6(A6) */
int error; /* -8(A6) */
char lineBuf[66]; /* -74(A6) */
char tmp; /* -76(A6) */
char alertBuf[64]; /* -140(A6) */
}

int get_maze_data(int fieldY,int fieldX,int flipped); /* L0804 */
{
}

void set_maze_data(int fieldY,int fieldX,int val); /* L080C */
{
}

void set_all_player(); /* L080E */
{
int i; /* -2(A6) */
}

void set_object(int newObjectIndex,int y,int x); /* L0816 */
{
int fieldY; /* -2(A6) */
int fieldX; /* -4(A6) */
int fieldYReminder; /* -6(A6) */
int fieldXReminder; /* -8(A6) */
int mazaFieldData; /* -10(A6) */
int nextObject; /* -12(A6) */
}

void draw_maze_generate_renderlist(int y,int x,int fieldOffsetY,int fieldOffsetX,int flip,int leftRightFlag); /* L081D */
{
register int viewingWidth; /* D4 */
register int viewingDistance; /* D5 */
register int fieldFX; /* D6 */
register int fieldFY; /* D7 */

int _fieldY; /* -2(A6) */
int _fieldX; /* -4(A6) */
int fieldY; /* -6(A6) */
int fieldX; /* -8(A6) */
}

int _random(); /* L0831 */
{
}

int _rnd(int maxVal); /* L0833 */
{
int randVal; /* -2(A6) */
int maxUnscaledVal; /* -4(A6) */
}

void draw_3d_ply(int cellFY,int cellFX,int flip); /* L0836 */
{
int distance; /* -2(A6) */
int xOffset; /* -4(A6) */
int size; /* -6(A6) */
int y; /* -8(A6) */
int x; /* -10(A6) */
int spriteID; /* -12(A6) */
int player; /* -14(A6) */
int objCount; /* -16(A6) */
int nextObject; /* -18(A6) */
int i; /* -20(A6) */
int j; /* -22(A6) */
PLAYER_POS objects[10]; /* -82(A6) */
}

int init_all_player(int playerCount,int isDrone); /* L084F */
{
int i; /* -2(A6) */
int j; /* -4(A6) */
}

int hunt_ply_pos(int player); /* L085B */
{
int tries; /* -2(A6) */
int fieldY; /* -4(A6) */
int fieldX; /* -6(A6) */
int wallCount; /* -8(A6) */
int found; /* -10(A6) */
int i; /* -12(A6) */
int deltaY; /* -14(A6) */
int deltaX; /* -16(A6) */
int dir; /* -18(A6) */
int distance; /* -20(A6) */
}

// @FILE: JOYSTICKMOUSE.C

void init_joystick(); /* L0877 */
{
}

void exit_joystick(); /* L0878 */
{
}

int ask_joystick(int index,int *buttonPressed); /* L0879 */
{
}

void own_joyvec(char *buf); /* L087E */
{
int i; /* -2(A6) */
}

void init_mouse(); /* L0882 */
{
}

void exit_mouse(); /* L0883 */
{
}

void own_mousevec(char *buf); /* L0884 */
{
int i; /* -2(A6) */
}

// @FILE: DRAWWALLS.C

void draw_maze_calc_viewmatrix(int microY,int microX,int minYOffset,int minXOffset,int maxYOffset,int maxXOffset,int isFlipped,int dir); /* L0887 */
{
register int i; /* D4 */
register int j; /* D5 */
register int j2; /* D6 */
register int k; /* D7 */

int minYDelta; /* -2(A6) */
int minXDelta; /* -4(A6) */
int maxYDelta; /* -6(A6) */
int maxXDelta; /* -8(A6) */
}

int draw_mazes_set_wall(int y1p,int x1p,int y2p,int x2p,int color,int leftRightFlag); /* L0894 */
{
int y1; /* -2(A6) */
int x1; /* -4(A6) */
int y2; /* -6(A6) */
int x2; /* -8(A6) */
}

void objecttable_clear(); /* L0897 */
{
}

int objecttable_set_wall(int x1,int h1,int x2,int h2,int color,int leftRightFlag); /* L0898 */
{
int xleft; /* -4(A6) */
int xright; /* -6(A6) */
}

int objecttable_check_if_hidden(int xleft,int xright); /* L08A0 */
{
int i; /* -2(A6) */
int ret; /* -4(A6) */
int fullWidth; /* -6(A6) */
}

int objecttable_check_view_fully_covered(); /* L08AA */
{
}

void objecttable_add(int xleft,int xright); /* L08AF */
{
int xleftIndex; /* -2(A6) */
int xrightIndex; /* -4(A6) */
int foundxleftFlag; /* -6(A6) */
int foundxrightFlag; /* -8(A6) */
int newxleft; /* -10(A6) */
int newxright; /* -12(A6) */
}

int objecttable_search(int x1,int startIndex,int *pFoundFlag); /* L08B4 */
{
int i; /* -2(A6) */
}

void objecttable_shift_table(int lowerIndex,int upperIndex); /* L08BB */
{
int width; /* -2(A6) */
}

int draw_mazes_shorten_wall(int y1,int x1,int y2,int x2,int slope,int *param_18,int *param_22); /* L08C2 */
{
register int divisor; /* D4 */
register int diff; /* D5 */
register int deltaY; /* D6 */
register int deltaX; /* D7 */

}

int draw_mazes_coord_order(int x1,int y1,int x2,int y2,int x3,int y3); /* L08C9 */
{
int flagXordered; /* -2(A6) */
int flagYordered; /* -4(A6) */
}

int draw_mazes_clip_wall(int *py1,int *px1,int *py2,int *px2); /* L08DC */
{
int xy1InViewFlag; /* -2(A6) */
int xy2InViewFlag; /* -4(A6) */
int y; /* -6(A6) */
int x; /* -8(A6) */
int tmp; /* -10(A6) */
}

// @FILE: FASTMATH.C

int fast_sin(int factor,int angle); /* L08F5 */
{
}

int fast_cos(int factor,int angle); /* L08F9 */
{
}

void rotate2d(int *px,int *py,int angle); /* L08FB */
{
int retX; /* -2(A6) */
}

// @FILE: LIVESTATUS.C

void init_live_shape(); /* L08FC */
{
}

void update_smily(); /* L08FE */
{
int i; /* -2(A6) */
int shapeOffset; /* -4(A6) */
int lives; /* -6(A6) */
}

// @FILE: DRAW3D.C

void clear_draw_list(); /* L0906 */
{
}

void to_draw_list(int type,int sprite_wallcolor,int x,int h,int x2_size,int h2_color); /* L0907 */
{
}

void draw_list(); /* L0908 */
{
int x1; /* -2(A6) */
int h1; /* -4(A6) */
int x2; /* -6(A6) */
int h2; /* -8(A6) */
}

void calc_yx_to_xh(int *pinY_outX,int *pinX_outH); /* L0910 */
{
register int newH; /* D6 */
register int newX; /* D7 */

}

// @FILE: DRAW3D.C

void draw_vline(int x,int h); /* L0911 */
{
}

void draw_wall(int x1,int h1,int x2,int h2,int color); /* L0915 */
{
register int xe; /* D5 */
register int slope; /* D6 */
register int endX; /* D7 */

}

// @FILE: DRAWPLAYERS.C

void init_faces_shapes(); /* L0922 */
{
int shapeIndex; /* -4(A6) */
int sheight; /* -6(A6) */
int shapeLine; /* -8(A6) */
int sword; /* -10(A6) */
int lastBodyOffset; /* -12(A6) */
int lastFaceOffset; /* -14(A6) */
int lastShadowOffset; /* -16(A6) */
int shapeWidthInWords; /* -18(A6) */
int shapeScale; /* -20(A6) */
int shadowHeightScale; /* -22(A6) */
int shadowHalfHeight; /* -24(A6) */
}

void init_faces_shapes_bw(); /* L092F */
{
int i; /* -2(A6) */
}

void image_double_width(unsigned short *imgPtr,long wordCount); /* L0932 */
{
register unsigned short *imgPtrReg; /* A5 */
register long wordsReg; /* D5 */
register int bitImage; /* D6 */
register int bitPos; /* D7 */

}

void draw_shape(int x,int size,int sprite,int shadowOffset,int playerIndex); /* L093B */
{
int shadowImageOffset; /* -2(A6) */
int shapeIndex; /* -4(A6) */
int y; /* -6(A6) */
int shapeWidthInWords; /* -8(A6) */
int height; /* -10(A6) */
int maxHeight; /* -12(A6) */
unsigned short *imageMask; /* -16(A6) */
}

// @FILE: GAMELOGIC.C

void calc_sin_table(); /* L094D */
{
int angle; /* -2(A6) */
int y; /* -4(A6) */
int x; /* -6(A6) */
}

int move_player(int player,int joystickData,int dronesActiveFlag); /* L0950 */
{
register int newFieldX; /* D3 */
register int newFieldY; /* D4 */
register int playerReg; /* D5 */
register int playerX; /* D6 */
register int playerY; /* D7 */

int cellYfract; /* -2(A6) */
int cellXfract; /* -4(A6) */
int direction; /* -6(A6) */
int tooCloseTop; /* -8(A6) */
int tooCloseBottom; /* -10(A6) */
int tooCloseLeft; /* -12(A6) */
int tooCloseRight; /* -14(A6) */
int yOffset; /* -16(A6) */
int xOffset; /* -18(A6) */
int bumpOfWalls; /* -20(A6) */
int fieldY; /* -22(A6) */
int fieldX; /* -24(A6) */
int distanceY; /* -26(A6) */
int distanceX; /* -28(A6) */
int objectID; /* -30(A6) */
int speedY; /* -32(A6) */
int speedX; /* -34(A6) */
int oldDiffY; /* -36(A6) */
int oldDiffX; /* -38(A6) */
int surroundingWalls; /* -40(A6) */
}

void move_shoot(int player); /* L09A1 */
{
register int shotXField; /* D3 */
register int shotYField; /* D4 */
register int shotX; /* D5 */
register int shotY; /* D6 */
register int playerReg; /* D7 */

int xOffset; /* -2(A6) */
int yOffset; /* -4(A6) */
int hasShot; /* -6(A6) */
int shotDirection; /* -8(A6) */
int zCoord; /* -10(A6) */
int objectID; /* -12(A6) */
int distanceY; /* -14(A6) */
int distanceX; /* -16(A6) */
int saveShotYField; /* -18(A6) */
int saveShotXField; /* -20(A6) */
}

// @FILE: BLIT_SHAPE_BW.C

void blit_draw_shape_bw(int x,int y,const unsigned short *imageMask,int widthInWords,int height,const unsigned short *ditherTable); /* L09BA */
{
}

// @FILE: MULS_DIVS.C

int muls_divs(int a,int b,int c); /* L09C2 */
{
}

// @FILE: CALCTABS.C

void calc_160_tab(); /* L09C3 */
{
}

void calc_80_tab(); /* L09C5 */
{
}

// @FILE: BLIT_LINE_BW.C

void blit_draw_hline_bw(int x1,int x2,int y,int col); /* L09C7 */
{
}

void blit_draw_hline_bw_double(int x1,int x2,int y,int col); /* L09C8 */
{
}

void blit_draw_vline_bw(int y1,int y2,int x,int col); /* L09C9 */
{
}

void blit_fill_box_bw(int x1,int y1,int x2,int y2,int col); /* L09CA */
{
}

void blit_fill_box_bw_double(int x1,int y1,int x2,int y2,int col); /* L09CC */
{
}

void blit_clear_window_bw(); /* L09CE */
{
}

// @FILE: ATARI.C

int xbios(...); /* L09E5 */
{
}

int bios(...); /* L09E6 */
{
}

int gemdos(...); /* L09E7 */
{
}

int appl_init(); /* L09EB */
{
}

int appl_exit(); /* L09EC */
{
}

int evnt_multi(int flags,int bclk,int bmsk,int bst,int m1flags,int m1x,int m1y,int m1w,int m1h,int m2flags,int m2x,int m2y,int m2w,int m2h,int *mepbuff,int tlc,int thc,int *pmx,int *pmy,int *pmb,int *pks,int *pkr,int *pbr); /* L09ED */
{
}

int form_do(OBJECT *form,int start); /* L09EE */
{
}

int form_dial(int flag,int littlx,int littly,int littlw,int littlh,int bigx,int bigy,int bigw,int bigh); /* L09EF */
{
}

int form_alert(int defbut,const char *astring); /* L09F0 */
{
}

int form_center(OBJECT *tree,int *pcx,int *pcy,int *pcw,int *pch); /* L09F1 */
{
}

int fsel_input(char *pipath,char *pisel,int *pbutton); /* L09F2 */
{
}

int graf_handle(int *pwchar,int *phbox,int *phchar,int *pwbox); /* L09F3 */
{
}

int graf_mouse(int m_number,MFORM *m_addr); /* L09F4 */
{
}

int menu_bar(OBJECT *tree,int showit); /* L09F5 */
{
}

int menu_tnormal(OBJECT *tree,int titlenumm,int normalit); /* L09F6 */
{
}

int objc_draw(OBJECT *tree,int drawob,int depth,int xc,int yc,int wc,int hc); /* L09F7 */
{
}

int rsrc_obfix(OBJECT *tree,int obj); /* L09F8 */
{
}

int wind_create(int kind,int wx,int wy,int ww,int wh); /* L09F9 */
{
}

int wind_open(int handle,int wx,int wy,int ww,int wh); /* L09FA */
{
}

int wind_close(int handle); /* L09FB */
{
}

int wind_delete(int handle); /* L09FC */
{
}

int wind_get(int w_handle,int w_field,int *pw1,int *pw2,int *pw3,int *pw4); /* L09FD */
{
}

int wind_update(int beg_update); /* L09FE */
{
}

void v_clrwk(int handle); /* L09FF */
{
}

void v_clsvwk(int handle); /* L0A00 */
{
}

void v_opnvwk(int *work_in,int *handle,int *work_out); /* L0A01 */
{
}

void vro_cpyfm(int handle,int wr_mode,int *pxyarray,MFDB *psrcMFDB,MFDB *pdesMFDB); /* L0A02 */
{
}

long lmul(long a,long b); /* L0A06 */
{
}

// @FILE: PROTECTION.C

int check_copy_protection(); /* L0A0A */
{
}

// @FILE: BLIT_SHAPE_COL.C

void blit_draw_shape_color(int x,int y,const unsigned short *imageMask,int widthInWords,int height,const void *colorFuncPtr); /* L0A16 */
{
}

void		blit_draw_shape_color_0(unsigned short *scrPtr, int xoffs, int orMask, int andMask) /* L0A1E */
{
}
void		blit_draw_shape_color_1(unsigned short *scrPtr, int xoffs, int orMask, int andMask) /* L0A1F */
{
}
void		blit_draw_shape_color_2(unsigned short *scrPtr, int xoffs, int orMask, int andMask) /* L0A20 */
{
}
void		blit_draw_shape_color_3(unsigned short *scrPtr, int xoffs, int orMask, int andMask) /* L0A21 */
{
}
void		blit_draw_shape_color_4(unsigned short *scrPtr, int xoffs, int orMask, int andMask) /* L0A22 */
{
}
void		blit_draw_shape_color_5(unsigned short *scrPtr, int xoffs, int orMask, int andMask) /* L0A23 */
{
}
void		blit_draw_shape_color_6(unsigned short *scrPtr, int xoffs, int orMask, int andMask) /* L0A24 */
{
}
void		blit_draw_shape_color_7(unsigned short *scrPtr, int xoffs, int orMask, int andMask) /* L0A25 */
{
}
void		blit_draw_shape_color_8(unsigned short *scrPtr, int xoffs, int orMask, int andMask) /* L0A26 */
{
}
void		blit_draw_shape_color_9(unsigned short *scrPtr, int xoffs, int orMask, int andMask) /* L0A27 */
{
}
void		blit_draw_shape_color_10(unsigned short *scrPtr, int xoffs, int orMask, int andMask) /* L0A28 */
{
}
void		blit_draw_shape_color_11(unsigned short *scrPtr, int xoffs, int orMask, int andMask) /* L0A29 */
{
}
void		blit_draw_shape_color_12(unsigned short *scrPtr, int xoffs, int orMask, int andMask) /* L0A2A */
{
}
void		blit_draw_shape_color_13(unsigned short *scrPtr, int xoffs, int orMask, int andMask) /* L0A2B */
{
}
void		blit_draw_shape_color_14(unsigned short *scrPtr, int xoffs, int orMask, int andMask) /* L0A2C */
{
}
void		blit_draw_shape_color_15(unsigned short *scrPtr, int xoffs, int orMask, int andMask) /* L0A2D */
{
}

// @FILE: BLIT_LINE_COL.C

void blit_draw_hline_color(int x1,int x2,int y,int col); /* L0A2E */
{
}

void blit_draw_vline_color(int y1,int y2,int x,int col); /* L0A2F */
{
}

void blit_fill_box_color(int x1,int y1,int x2,int y2,int col); /* L0A30 */
{
}

void blit_clear_window_color(); /* L0A32 */
{
}

// @FILE: DECOMPRESSTITLE.C

void decompress_image_to_screen(unsigned short *bytesW,unsigned short *screenPtrW); /* L0A48 */
{
}



// DATA: L0A56
static const unsigned char ctrl_cnts[348]; /* L0A56 @d */
static const unsigned short colortable[16]; /* L0A57 */
static const unsigned short color_ply_back[16]; /* L0A58 @d */
static const unsigned short color_ply_frame[16]; /* L0A59 @d */
static short team_notes_color_table[4]; /* L0A5A @d */
static DIR_STRUCT dir_table[8]; /* L0A5B */
static const char sound_shot[196]; /* L0A5C */
static const char sound_hit[18]; /* L0A5D */
static const void *switch_table1[6]; /* L0A5E */
static unsigned short loosershape_img[36]; /* L0A5F */
static unsigned short blinzshape_img[12]; /* L0A60 */
static const short lose_anim[29]; /* L0A61 @d */
static const short winner_anim[29]; /* L0A62 @d */
static const void *switch_table2[38]; /* L0A63 */
static const POS_STRUCT midicam_start_setup[4]; /* L0A64 */
static unsigned short crossedsmil_img[60]; /* L0A65 */
static unsigned short mapsmily_img[10]; /* L0A66 */
static unsigned short mapsmily2_img[10]; /* L0A67 */
static short mouse_dy2; /* L0A68 @d */
static const char *_run_in_low_or_high_res_alert_str; /* L0A69 */
static const char *_MIDIMAZE_D8A_read_error_alert_str; /* L0A6A */
static const char *_MIDIMAZE_D8A_str; /* L0A6B */
static const char *_MIDIMAZE_D8A_opening_error_alert_str; /* L0A6C */
static const char *_MIDICAM1_str; /* L0A6D */
static const char *_MIDICAM2_str; /* L0A6E */
static const char *_says_str; /* L0A6F */
static const char *_have_a_nice_day1_str; /* L0A70 */
static const char *_game_suspended_alert_str; /* L0A71 */
static const char *_your_team_wins_str; /* L0A72 */
static const char *_your_team_loses_str; /* L0A73 */
static const char *_you_win_str; /* L0A74 */
static const char *_wins_str; /* L0A75 */
static const char *_have_a_nice_day2_str; /* L0A76 */
static const char *_15_spaces_str; /* L0A77 */
static char *rs_strings[92]; /* L0A78 */
static long rs_frstr[1]; /* LRSC1 */
static short rs_bitblk[7]; /* LRSC2 */
static long rs_frimg[1]; /* LRSC3 */
static short rs_iconblk[17]; /* LRSC4 */
static TEDINFO rs_tedinfo[1]; /* L0A79;L0A7A+4;L0A7B+8 */
static foobar rs_imdope[1]; /* L0A7a */
static const char *pname; /* L0A7b */
static OBJECT rs_object[219]; /* L0A7C;L0A7D+160 */
static long rs_trindex[14]; /* L0A7E @d */
static short maze_loaded_flag; /* L0A7F @d */
static const void *switch_table3[7]; /* L0A80 */
static const short revive_lives_rscindices[3]; /* L0A81 @d */
static int rsc_drones[3]; /* L0A82;L0A83+2;L0A84+4 @d */
static const short drone_count_up_rscindices[3]; /* L0A85 @d */
static const short drone_count_down_rscindices[3]; /* L0A86 @d */
static const short drone_count_number_rscindices[3]; /* L0A87 @d */
static const short drone_count_box_rscindices[3]; /* L0A88 @d */
static const short team_group_rscindices[16][4]; /* L0A89 @d */
static const short rsc_playername_rscindices[16]; /* L0A8A @d */
static const char *str_drone_type_br_array[3]; /* L0A8B */
static const char **emptyName; /* L0A8C */
static const char **defaultPlayerName; /* L0A8D */
static const char *droneNames[3]; /* L0A8E */
static char *rsrc_str_0; /* L0A8F */
static char *rsrc_str_1; /* L0A90 */
static char *rsrc_str_2; /* L0A91 */
static char *rsrc_str_3; /* L0A92 */
static char *rsrc_str_4; /* L0A93 */
static char *rsrc_str_5; /* L0A94 */
static char *rsrc_str_6; /* L0A95 */
static char *rsrc_str_7; /* L0A96 */
static char *rsrc_str_8; /* L0A97 */
static char *rsrc_str_9; /* L0A98 */
static char *rsrc_str_10; /* L0A99 */
static char *rsrc_str_11; /* L0A9A */
static char *rsrc_str_12; /* L0A9B */
static char *rsrc_str_13; /* L0A9C */
static char *rsrc_str_14; /* L0A9D */
static char *rsrc_str_15; /* L0A9E */
static char *rsrc_str_16; /* L0A9F */
static char *rsrc_str_17; /* L0AA0 */
static char *rsrc_str_18; /* L0AA1 */
static char *rsrc_str_19; /* L0AA2 */
static char *rsrc_str_20; /* L0AA3 */
static char *rsrc_str_21; /* L0AA4 */
static char *rsrc_str_22; /* L0AA5 */
static char *rsrc_str_23; /* L0AA6 */
static char *rsrc_str_24; /* L0AA7 */
static char *rsrc_str_25; /* L0AA8 */
static char *rsrc_str_26; /* L0AA9 */
static char *rsrc_str_27; /* L0AAA */
static char *rsrc_str_28; /* L0AAB */
static char *rsrc_str_29; /* L0AAC */
static char *rsrc_str_30; /* L0AAD */
static char *rsrc_str_31; /* L0AAE */
static char *rsrc_str_32; /* L0AAF */
static char *rsrc_str_33; /* L0AB0 */
static char *rsrc_str_34; /* L0AB1 */
static char *rsrc_str_35; /* L0AB2 */
static char *rsrc_str_36; /* L0AB3 */
static char *rsrc_str_37; /* L0AB4 */
static char *rsrc_str_38; /* L0AB5 */
static char *rsrc_str_39; /* L0AB6 */
static char *rsrc_str_40; /* L0AB7 */
static char *rsrc_str_41; /* L0AB8 */
static char *rsrc_str_42; /* L0AB9 */
static char *rsrc_str_43; /* L0ABA */
static char *rsrc_str_44; /* L0ABB */
static char *rsrc_str_45; /* L0ABC */
static char *rsrc_str_46; /* L0ABD */
static char *rsrc_str_47; /* L0ABE */
static char *rsrc_str_48; /* L0ABF */
static char *rsrc_str_49; /* L0AC0 */
static char *rsrc_str_50; /* L0AC1 */
static char *rsrc_str_51; /* L0AC2 */
static char *rsrc_str_52; /* L0AC3 */
static char *rsrc_str_53; /* L0AC4 */
static char *rsrc_str_54; /* L0AC5 */
static char *rsrc_str_55; /* L0AC6 */
static char *rsrc_str_56; /* L0AC7 */
static char *rsrc_str_57; /* L0AC8 */
static char *rsrc_str_58; /* L0AC9 */
static char *rsrc_str_59; /* L0ACA */
static char *rsrc_str_60; /* L0ACB */
static char *rsrc_str_61; /* L0ACC */
static char *rsrc_str_62; /* L0ACD */
static char *rsrc_str_63; /* L0ACE */
static char *rsrc_str_64; /* L0ACF */
static char *rsrc_str_65; /* L0AD0 */
static char *rsrc_str_66; /* L0AD1 */
static char *rsrc_str_67; /* L0AD2 */
static char *rsrc_str_68; /* L0AD3 */
static char *rsrc_str_69; /* L0AD4 */
static char *rsrc_str_70; /* L0AD5 */
static char *rsrc_str_71; /* L0AD6 */
static char *rsrc_str_72; /* L0AD7 */
static char *rsrc_str_73; /* L0AD8 */
static char *rsrc_str_74; /* L0AD9 */
static char *rsrc_str_75; /* L0ADA */
static char *rsrc_str_76; /* L0ADB */
static char *rsrc_str_77; /* L0ADC */
static char *rsrc_str_78; /* L0ADD */
static char *rsrc_str_79; /* L0ADE */
static char *rsrc_str_80; /* L0ADF */
static char *rsrc_str_81; /* L0AE0 */
static char *rsrc_str_82; /* L0AE1 */
static char *rsrc_str_83; /* L0AE2 */
static char *rsrc_str_84; /* L0AE3 */
static char *rsrc_str_85; /* L0AE4 */
static char *rsrc_str_86; /* L0AE5 */
static char *rsrc_str_87; /* L0AE6 */
static char *rsrc_str_88; /* L0AE7 */
static char *rsrc_str_89; /* L0AE8 */
static char *rsrc_str_90; /* L0AE9 */
static char *rsrc_str_91; /* L0AEA */
static const char *_cant_open_window_alert_str; /* L0AEB */
static const char *_this_is_a_solo_machine_alert_str; /* L0AEC */
static const char *_this_is_a_master_machine_alert_str; /* L0AED */
static const char *_default_mazefilename1_str; /* L0AEE */
static const char *_default_mazefilename2_str; /* L0AEF */
static const char *_mouse_control_selected_alert_str; /* L0AF0 */
static const char *_joystick_control_selected_alert_str; /* L0AF1 */
static const char *_what_about_it_alert_str; /* L0AF2 */
static const char *_midiring_boo_boo_alert_str1; /* L0AF3 */
static const char *_too_many_machines_online_alert_str1; /* L0AF4 */
static const char *_please_load_a_maze_first_alert_str; /* L0AF5 */
static const char *_midiring_boo_boo_alert_str2; /* L0AF6 */
static const char *_too_many_machines_online_alert_str2; /* L0AF7 */
static const char *_midiring_timeout_alert_str; /* L0AF8 */
static const char *_game_terminated_alert_str; /* L0AF9 */
static const char *_maze_too_small_alert_str; /* L0AFA */
static const char *_do_you_really_wanna_quit_alert_str; /* L0AFB */
static const char *_drone_type_target_str; /* L0AFC */
static const char *_drone_type_standard_str; /* L0AFD */
static const char *_drone_type_ninja_str; /* L0AFE */
static const char *_7_dashes_str; /* L0AFF */
static const char *_player_xx_str; /* L0B00 */
static const char *_target_x_str; /* L0B01 */
static const char *_std_drone_str; /* L0B02 */
static const char *_ninja_x_str; /* L0B03 */
static const char *_remote_str; /* L0B04 */
static short objekt_anz; /* L0B05 @d */
static const char **_error_read_maze_file_alert_str_ptr; /* L0B06 */
static const char **ptr_str_alert_mazefile_format_booboo; /* L0B07 */
static const char **ptr_str_alert__mazefile_header_booboo; /* L0B08 */
static const char *cantOpenMaze1String; /* L0B09 */
static const char *cantOpenMaze2String; /* L0B0A */
static const short face_shape_tab[32]; /* L0B0B @d */
static char mouse_defaults[4]; /* L0B0C */
static const char *_cant_open_mazefile_alert_str; /* L0B0D */
static const char *_error_read_maze_file_alert_str; /* L0B0E */
static const char *_mazefile_format_booboo_alert_str; /* L0B0F */
static const char *_mazefile_header_booboo_alert_str; /* L0B10 */
static unsigned short live_shape_img[400]; /* L0B11 */
static short wand_farb_tab[2]; /* L0B12;L0B13+2 @d */
static const unsigned short bmask_d[8]; /* L0B14 */
static const unsigned short bmask_allDown[16]; /* L0B15 */
static const unsigned short bmask_allUp[16]; /* L0B16 */
static const unsigned short bmask_singlebit[16]; /* L0B17 */
static const unsigned short fm_bw_muster0[5]; /* L0B18 */
static const unsigned short fm_bw_muster1[5]; /* L0B19 */
static const unsigned short fm_bw_muster2[5]; /* L0B1A */
static const unsigned short fm_bw_muster3[5]; /* L0B1B */
static const unsigned short fm_bw_muster4[5]; /* L0B1C */
static const unsigned short fm_bw_muster5[5]; /* L0B1D */
static const unsigned short fm_bw_muster6[5]; /* L0B1E */
static const unsigned short fm_bw_muster7[5]; /* L0B1F */
static const unsigned short fm_bw_muster8[5]; /* L0B20 */
static const unsigned short fm_bw_muster9[5]; /* L0B21 */
static const unsigned short fm_bw_muster10[5]; /* L0B22 */
static const unsigned short fm_bw_muster11[5]; /* L0B23 */
static const unsigned short fm_bw_muster12[5]; /* L0B24 */
static const unsigned short fm_bw_muster13[5]; /* L0B25 */
static const unsigned short fm_bw_muster14[5]; /* L0B26 */
static const unsigned short fm_bw_muster15[5]; /* L0B27 */
static const unsigned short fm_bw_muster16[5]; /* L0B28 */
static const unsigned short fm_bw_muster17[5]; /* L0B29 */
static const unsigned short fm_bw_muster18[5]; /* L0B2A */
static const unsigned short *bw_fillpattern_table[19]; /* L0B2B;L0B2C+8 */
static const void *col_setcolor_jumptable[16]; /* L0B2D;L0B2E+20;L0B2F+56 */

// BSS: L0B30

long ustk[257]; /* L0B30_BASE;L0B30+1024 */
short read_vbclock_ret; /* L0B31 */
short midi_timeout_value; /* L0B32 */
char *str_drone_name; /* L0B33 */
char *playerNameStrPtr; /* L0B34 */
short joystick_flag; /* L0B35 */
short init_joy_flag; /* L0B36 */
XY_SPEED_TABLE draw_maze_viewmatrix[9][17]; /* L0B37;L0B38+2;L0B39+64;L0B3A+66;L0B3B+544;L0B3C+546;L0B3D+608;L0B3E+610 */
short table_size; /* L0B3F */
TABLE_LIST_XCOORD table_list[10]; /* L0B40;L0B41+2 */
short draw_elem_count; /* L0B42 */
ELEM_LIST_STRUCT draw_elem_list[100] /* L0B43 */
short mult160tab[256]; /* L0B44 */
short mult80tab[400]; /* L0B45 */
unsigned short *screen_offs_adr; /* L0B46 */
long retsav; /* L0B47 */
short *pblock; /* L0B48 */
short *iioff; /* L0B49 */
short *pioff; /* L0B4A */
short *iooff; /* L0B4B */
short *pooff; /* L0B4C */
short intin[128]; /* L0B4D */
short window_floor_height; /* L0B4E */
short score_table[16]; /* L0B4F */
short player_joy_table[16]; /* L0B50 */
int pxy[8]; /* L0B51;L0B52+2;L0B53+4;L0B54+6;L0B55+8;L0B56+10;L0B57+12;L0B58+14 */
short intout[128]; /* L0B59 */
short team_flag; /* L0B5A */
int drone_needs2GoSouth; /* L0B5B */
short maze_size; /* L0B5C */
CBLK c; /* L0B5D;L0B5E+4;L0B5F+8;L0B60+12;L0B61+16;L0B62+20 */
short color_cnv_back[16]; /* L0B63 */
int drone_needs2GoWest; /* L0B64 */
short midicam_autoselect_player_flag; /* L0B65 */
unsigned short draw_shadow_img[752]; /* L0B66 */
int dummy; /* L0B67 */
short reload_time; /* L0B68 */
short viewposition_direction; /* L0B69 */
char screenBuffer[32255]; /* L0B6A_BASE;L0B6A+255 */
short draw_shape_bodyImageOffset[24]; /* L0B6B;L0B6C+24 */
short shape_image_shadowHeight[24]; /* L0B6D */
short ptsin[128]; /* L0B6E */
short *sine_table; /* L0B6F */
short regen_time; /* L0B70 */
PLAYER_DATA player_data[16]; /* L0B71 */
short areDronesActiveFlag; /* L0B72 */
short color_cnv_frame[16]; /* L0B73 */
OBJECT_TABLE_ENTRY objekt_table[32]; /* L0B74 */
long draw_shape_faceImageOffset[24]; /* L0B75 */
short ptsout[128]; /* L0B76 */
short currently_displayed_notes_score[16]; /* L0B77 */
char load_buffer[133354]; /* L0B78;L0B79+130;L0B7A+3302 */
unsigned short global[15]; /* L0B7B */
int vdi_handle; /* L0B7C */
short we_dont_have_a_winner; /* L0B7D */
short midimaze_d8a_filehandle; /* L0B7E */
short screen_offset; /* L0B7F */
unsigned short *crossedsmil_ptr; /* L0B80 */
unsigned short *live_shape_ptr; /* L0B81 */
short screen_rez; /* L0B82 */
unsigned short *blinzshape_ptr; /* L0B83 */
short viewposition_y; /* L0B84 */
short viewposition_x; /* L0B85 */
unsigned short *mapsmily2_ptr; /* L0B86 */
OBJECT *rsrc_tree_ptr; /* L0B87 */
short draw_shape_shadowImageOffset[24]; /* L0B88 */
short display_2d_map_flag; /* L0B89 */
char saved_sound_regs[14]; /* L0B8A */
short mouse_record[3]; /* L0B8B;L0B8C+2;L0B8D+4 */
short _random_seed; /* L0B8E */
unsigned short control[4]; /* L0B8F */
unsigned short int_in[16]; /* L0B90;L0B91+2;L0B92+4;L0B93+6;L0B94+8;L0B95+10;L0B96+12;L0B97+14;L0B98+16;L0B99+18;L0B9A+20;L0B9B+22;L0B9C+24;L0B9D+26;L0B9E+28;L0B9F+30 */
unsigned short int_in[7]; /* L0BA0;L0BA1+2;L0BA2+4;L0BA3+6;L0BA4+8;L0BA5+10;L0BA6+12 */
short drone_angle_table[33]; /* L0BA7 */
short screen_flag; /* L0BA8 */
short playerAndDroneCount; /* L0BA9 */
short distance_scale_factor; /* L0BAA */
short revive_time; /* L0BAB */
unsigned short *mapsmily_ptr; /* L0BAC */
int drone_can_east; /* L0BAD */
KBDVBASE *KBDVECS_ptr; /* L0BAE */
MFDB src_MFDB; /* L0BAF;L0BB0+4;L0BB1+6;L0BB2+8;L0BB3+10;L0BB4+12 */
long ad_c; /* L0BB5 */
short own_number; /* L0BB6 */
short midicam_player_number; /* L0BB7 */
const char *sound_shot_ptr; /* L0BB8 */
const char *sound_hit_ptr; /* L0BB9 */
short machines_online; /* L0BBA */
unsigned short *shape_ptr; /* L0BBB */
short main_wind_offset; /* L0BBC */
short friendly_fire; /* L0BBD */
MFDB dest_MFDB; /* L0BBE;L0BBF+4;L0BC0+6;L0BC1+8;L0BC2+10;L0BC3+12 */
char joystick[4]; /* L0BC4;L0BC5+1 */
short kills_wind_offset; /* L0BC6 */
short score_notes_wind_offset; /* L0BC7 */
int drone_can_north; /* L0BC8 */
char maze_datas[4096]; /* L0BC9 */
void *kbdVectors_midivec; /* L0BCA */
void *kbdVectors_vkbderr; /* L0BCB */
void *kbdVectors_vmiderr; /* L0BCC */
void *kbdVectors_statvec; /* L0BCD */
void *kbdVectors_mousevec; /* L0BCE */
void *kbdVectors_clockvec; /* L0BCF */
void *kbdVectors_joyvec; /* L0BD0 */
void *kbdVectors_ikbdsys; /* L0BD1 */
short live_status_wind_offset; /* L0BD2 */
short team_scores[4]; /* L0BD3 */
short gl_apid; /* L0BD4 */
short contrl[12]; /* L0BD5;L0BD6+2;L0BD7+6;L0BD8+12;L0BD9+14;L0BDA+16 */
short revive_lives; /* L0BDB */
unsigned short *loosershape_ptr; /* L0BDC */
long addr_in[2]; /* L0BDD;L0BDE+4 */
short user_is_midicam; /* L0BDF */
unsigned short save_part_of_screen_for_dialog_buffer[5256]; /* L0BE0 */
int drone_can_south; /* L0BE1 */
XY_SPEED_TABLE xy_speed_table[256]; /* L0BE2 */
unsigned short *shape_face_ptr; /* L0BE3 */
int drone_needs2GoEast; /* L0BE4 */
int drone_can_west; /* L0BE5 */
OBJECT *rsrc_object_array[14]; /* L0BE6;L0BE7+32;L0BE8+36;L0BE9+44;L0BEA+48 */
short wind_handle; /* L0BEB */
short protectionIsValid; /* L0BEC */
short window_hcenter; /* L0BED */
short window_sky_height; /* L0BEE */
short midicam_player_count; /* L0BEF */
short midicam_timer_counter; /* L0BF0 */
int active_drones_by_type[3]; /* L0BF1;L0BF2+2;L0BF3+4 */
short draw_shape_bodyHeight[24]; /* L0BF4 */
long addr_out[1]; /* L0BF5 */
short window_hwidth; /* L0BF6 */
unsigned short *screen_ptr[2]; /* L0BF7;L0BF8+4 */
int drone_needs2GoNorth; /* L0BF9 */

// BSS-END: L0BF9
