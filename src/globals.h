/************************************************************
 *** Header for global variables and functions
 ************************************************************/
#ifndef __MIDIMAZE_GLOBAL_H__
#define __MIDIMAZE_GLOBAL_H__ 1

#include <stdio.h>
#include <stdlib.h>
#include "portab.h"
#include "gemdefs.h"
#include "osif.h"

#ifdef __atarist__
#define NON_ATARI_HACK 0
#else
#define NON_ATARI_HACK 1 // Because we don't support MIDI, we disable the master/slave recognition at launch
#endif

/* Several defines to enable individual bugfixes for the original MIDImaze */
#define BUGFIX_MIDI_TIMEOUT     1 /* fix weird MIDI loop timeouts during play */
#define BUGFIX_DIVISION_BY_ZERO 1 /* Avoid a division by zero when drawing a horizontal wall. Not needed on an original Atari ST, because the Atari ignores a division by zero. */
#define BUGFIX_UMLAUTS_IN_NAMES 1 /* Anybody using an 8-bit ASCII character in a player name, will stop the MIDI transmission from working */

#define DEBUG_2D_MAZE 0 // shows all oppnents and the 'plan' of Ninjas

#ifdef __m68k__
#define INTELSWAP16(a) a
#define INTELSWAP32(a) a
#else
#define INTELSWAP16(a) ((unsigned short)(((unsigned short)(a) >> 8) | ((unsigned short)(a) << 8)))
#define INTELSWAP32(a) ((uint32_t)((((uint32_t)(a) << 24) & 0xFF000000L) | (((uint32_t)(a) << 8) & 0x00FF0000L) | (((uint32_t)(a) >> 8) & 0x0000FF00L) | (((uint32_t)(a) >> 24) & 0x000000FFL)))
#endif

/* symbolic names for the 16 different colors */
#define COLOR_BLACK_INDEX 0
#define COLOR_SILVER_INDEX 1
#define COLOR_MAGNESIUM_INDEX 2
#define COLOR_BLUE_INDEX 3
#define COLOR_GREY_INDEX 4
#define COLOR_DKGREEN_INDEX 5
#define COLOR_ALUMINIUM_INDEX 6
#define COLOR_STEEL_INDEX 7
#define COLOR_YELLOW_INDEX 8
#define COLOR_ORANGE_INDEX 9
#define COLOR_PURPLE_INDEX 10
#define COLOR_MAGENTA_INDEX 11
#define COLOR_LTBLUE_INDEX 12
#define COLOR_GREEN_INDEX 13
#define COLOR_RED_INDEX 14
#define COLOR_WHITE_INDEX 15

/* The following constants are added to document dependencies to the screen width */
/* The can not be change, a lot depends on them - often subtle, like in decompress_image_to_screen() */
/* 640x400x1 */
#define SCREEN_BW_WIDTH 640
#define SCREEN_BW_HEIGHT 400
#define SCREEN_BW_PLANES 1
#define SCREEN_BW_LINEOFFSET (SCREEN_BW_WIDTH*SCREEN_BW_PLANES/8)
/* 320x200x4 */
#define SCREEN_COL_WIDTH 320
#define SCREEN_COL_HEIGHT 200
#define SCREEN_COL_PLANES 4
#define SCREEN_COL_LINEOFFSET (SCREEN_COL_WIDTH*SCREEN_COL_PLANES/8)
/* screen size is same for both resolutions */
#define SCREEN_SIZE 32000

/* different modes for the dispatch() function */
#define DISPATCH_QUIT -1
#define DISPATCH_MASTER 0
#define DISPATCH_SLAVE 1
#define DISPATCH_SOLO 2
#define DISPATCH_AUTOMATIC 3
#define DISPATCH_MIDICAM 4

/* Joystick bitmask for the 4 directions and the button */
#define JOYSTICK_UP 0x01
#define JOYSTICK_DOWN 0x02
#define JOYSTICK_LEFT 0x04
#define JOYSTICK_RIGHT 0x08
#define JOYSTICK_BUTTON 0x10

/* internal codes for the different drone types */
#define DRONE_TARGET 'r' /* #114 Very Dumb (Target, they never fire a shot) */
#define DRONE_STANDARD 'l' /* #108 Plain Dumb (Standard) */
#define DRONE_NINJA 'k' /* #107 Not so Dumb (Ninja) */
#define DRONE_TYPES 3 /* 3 drone types. The drone code is hard-coded to them. This is just to easily find references which are dynamic */

/* MIDI messages to trigger certain actions */
#define MIDI_COUNT_PLAYERS 0x80 /* Detect number of players in the MIDI ring */
#define MIDI_RESET_SCORE 0x81 /* Reset all scores */
#define MIDI_TERMINATE_GAME 0x82 /* Game terminated by the Master */
#define MIDI_SEND_DATA 0x83 /* after MIDI_START_GAME this package contains all shared data */
#define MIDI_START_GAME 0x84 /* Start the game (followed by MIDI_SEND_DATA) */
#define MIDI_ABOUT 0x85 /* Show about box */
#define MIDI_NAME_DIALOG 0x86 /* Allow all players to enter their name */

/* 0.29..0.4s timeout (depending on the screen refresh rate) */
#define MIDI_DEFAULT_TIMEOUT 20

/* These are a hard constants and only used for reference. Search for the use-cases and check the code! */
#define PLAYER_MAX_COUNT 16 /* number of possible players. (The team dialog box is hard-coded for 16, also the MIDIcam player-switcher) */
#define PLAYER_MAX_TEAMS 4 /* number of possible teams. (The Team dialog box is hard-coded for 4, also the drone setup code) */
#define PLAYER_MAX_LIVES 3 /* number of max. lives */

/* Timer constants for the 3 main preferences */
#define TIME_RELOAD_FAST 10
#define TIME_RELOAD_SLOW 30
#define TIME_REGEN_FAST 100
#define TIME_REGEN_SLOW 200
#define TIME_REVIVE_FAST 50
#define TIME_REVIVE_SLOW 100

#define GAME_WIN_SCORE 10 /* number of kills to win the game */
#define MAX_GAME_SCORE 10 /* number of game wins before the score table overflows (it resets to 1) */

/* A direction is always a 0..255 number, except in the drone code, */
/* where 256 is sometimes used as 0 (because 0 has a special meaning) */
#define PLAYER_DIR_NORTH     0x00
#define PLAYER_DIR_NORTHEAST 0x20
#define PLAYER_DIR_EAST      0x40
#define PLAYER_DIR_SOUTHEAST 0x60
#define PLAYER_DIR_SOUTH     0x80
#define PLAYER_DIR_SOUTHWEST 0xa0
#define PLAYER_DIR_WEST      0xc0
#define PLAYER_DIR_NORTHWEST 0xe0

#define MAZE_MAX_SIZE 64 /* 64x64 is the maximum size for a maze (if it gets larger, I would be careful - there might be short overflows - 64*64 is only 4096 bytes long) */
#define MAZE_CELL_SIZE 256 /* size of a cell in the maze, power of 2 - DO NOT CHANGE! A lot depends on this (e.g. MAZE_CELL_SIZE * MAZE_MAX_SIZE has to fit into a 16-bit short, the width of a player/shot, distance to the walls, etc), this is just for reference. */
#define MAZE_FIELD_SHIFT 7 /* 1<<MAZE_FIELD_SHIFT == (MAZE_CELL_SIZE/2), used to check for walls and players. */

#define MAZE_FIELD_EMPTY -1 /* used to mark an empty field or the end of ply_plist/ply_slist in the player_data */
#define MAZE_FIELD_WALL 1 /* used to mark a wall field */

/* Player rotates by 8 units per round (~11.25 degrees, or 32 steps for a full 360 degree turn) */
#define PLAYER_MOTION_ROTATE 8
/* Player maximum speed is 32 units per round (8 rounds for moving one field) */
/* Because a cell is MAZE_CELL_SIZE units, it takes 8 steps to pass through one. */
/* For drones it is essential that this matches the rotation speed, otherwise */
/* the drones could get stuck on corners or at least moves quite "frustrated" */
#define PLAYER_MOTION_SPEED (MAZE_CELL_SIZE/8)

/* Different objects in the draw_elem_list[] */
#define DRAW_TYPE_WALL      1
#define DRAW_TYPE_PLAYER    2
#define DRAW_TYPE_SHOT      3

/* number of shapes for the body */
#define BODY_SHAPE_COUNT 24 /* There are 24 different sizes of smily shapes in the image data */
#define BODY_SHAPE_MAX_SIZE 32 /* The actual height in pixel is: 2 (size = 0) and 53 (size = 32) */
#define BODY_SHAPE_NO_SHADOW 50 /* a shadow offset of 50 (out of 100) is behind the smily, which makes it invisible */

#define BODY_SHAPE_BITMAPSIZE 1586L /* data size in the MIDIMAZE.D8A file for a shape in all different sizes */
#define BODY_SHAPE_FACE_COUNT 20 /* There are 20 different faces (used for rotations: 0=front, 1-9 to the right, 11-19 to the left, 10 = no face). Shape 10 is used 12 times for the back (= 32 different rotation values), because of the round player sprite the 144 degrees away from the player can't be seen and always look the same)  */
#define BODY_SHAPE_FRONT_VIEW 0 /* index 0 is a smily from the front */
#define BODY_SHAPE_BACK_VIEW 10 /* index 10 is a smily from the back (no face, just the frame - also used for the shot) */


/* 3 VT52 macros, which probably existed in the original code as well */
#define BCON_SETCURSOR(r,c) Bconout(CON, 27); Bconout(CON, 'Y'); Bconout(CON, (r)+32); Bconout(CON, (c)+32)
#define BCON_SETFCOLOR(c) Bconout(CON, 27); Bconout(CON, 'b'); Bconout(CON, (c))
#define BCON_SETBCOLOR(c) Bconout(CON, 27); Bconout(CON, 'c'); Bconout(CON, (c))

#define BCON_DEFAULT_TEXT_COLOR() BCON_SETBCOLOR(screen_rez ? COLOR_SILVER_INDEX : COLOR_WHITE_INDEX); \
                                  BCON_SETFCOLOR(COLOR_BLACK_INDEX);

typedef struct {
    int ply_y; /* Y coordinate of the player */
    int ply_x; /* Y coordinate of the player */
    int ply_dir; /* viewing/moving direction of players (0..255, which matches 0..359 degrees) */
    int ply_unused_6; /* never read/written to */
    int ply_unused_8; /* never read/written to */
    int ply_lives; /* number of lives; negative, if the player is dead */
    int ply_refresh; /* if >0 used to revive/refresh and gain another live. */
    int ply_hitflag; /* true, if the player got hit by a shot. Used for flashing the screen or drones to pick the shooter as a target */
    int ply_reload; /* timer to reload after shooting */
    int ply_score;  /* score if this player (not used for teams). 10 wins the game. */
    int ply_gunman; /* if this player was shot/killed, the index of the shooter */
    int ply_looser; /* player that got shot by the current player (used for the kill board) */
    int ply_team; /* team number, if teams are active (0..3) */
    int dr_type; /* type of the drone (DRONE_TARGET,DRONE_STANDARD,DRONE_NINJA) */
    int dr_rotateCounter; /* rotation timer for idle movement and 180-degree turn */
    int dr_upRotationCounter; /* rotation timer for up/left and up/right motion to get around corners */
    int dr_joystick; /* generated joystick position for this drone */
    int dr_isInactive; /* 0, if the drone did not move and doesn't have a pending move. Used to trigger a new movement "plan" */
    int dr_dir[6]; /* allows the drone to plan more complex moves, only used by the ninja */
    struct {
        int y;
        int x;
    } dr_field[6]; /* allows the drone to plan more complex moves, only used by the ninja */
    int dr_fieldIndex; /* allows the drone to plan more complex moves, only used by the ninja */
    int dr_fieldResetTimer; /* allows the drone to plan more complex moves, only used by the ninja */
    int dr_targetLocked; /* a target is in-sight and to be fired at */
    int dr_fireDirection; /* no fire direction (only set, never read) */
    int dr_humanEnemies[PLAYER_MAX_COUNT+2]; /* non-team mode: -1 terminated list of all human players */
    int dr_currentTarget; /* <0: no target, >=0: current player, which is a target to be tracked down and shot */
    int dr_permanentTarget; /* permanent target player, used if there is no other current target. Defined before the game starts */
    int ply_plist; /* object was a player: index of the next player/shot in the list of object in a maze field (0..15: player, 16..31: shot) */
    int ply_shooty; /* Y coordinate of the current shot */
    int ply_shootx; /* X coordinate of the current shot */
    int ply_shootr; /* direction of the current shot (0..255, which matches 0..359 degrees) */
    int ply_shoot; /* != 0, if a shot is active (probably was a timer at some point, because it is initialized with 10, instead of 1) */
    int ply_slist; /* object was a shot: index of the next player/shot in the list of object in a maze field (0..15: player, 16..31: shot) */
} PLAYER_DATA;

typedef struct {
    int deltaY;
    int deltaX;
} XY_SPEED_TABLE;

typedef void (*SETCOLOR)(unsigned short *scrPtr, int xoffs, int orMask, int andMask);

#if BUGFIX_MIDI_TIMEOUT
typedef long miditimeout_t;
#else
typedef short miditimeout_t;
#endif


extern const unsigned short colortable[16];
extern short team_notes_color_table[PLAYER_MAX_TEAMS];
extern TEDINFO rs_tedinfo[1];
extern short maze_loaded_flag;
extern const short team_group_rscindices[PLAYER_MAX_COUNT][PLAYER_MAX_TEAMS];
extern const short rsc_playername_rscindices[PLAYER_MAX_COUNT];
extern const unsigned short *bw_fillpattern_table[16+3]; /* +3 for the dithering function */
extern SETCOLOR col_setcolor_jumptable[16];
extern unsigned short *screen_offs_adr;
extern short viewscreen_floor_height;
extern short score_table[PLAYER_MAX_COUNT];
extern short player_joy_table[PLAYER_MAX_COUNT];
extern short team_flag;
extern short maze_size;
extern short color_cnv_back[PLAYER_MAX_COUNT];
extern short midicam_autoselect_player_flag;
extern short dummy;
extern short reload_time;
extern short draw_shape_bodyImageOffset[BODY_SHAPE_COUNT];
extern short *sine_table;
extern short regen_time;
extern PLAYER_DATA player_data[PLAYER_MAX_COUNT];
extern short areDronesActiveFlag;
extern short color_cnv_frame[PLAYER_MAX_COUNT];
extern long draw_shape_faceImageOffset[BODY_SHAPE_COUNT];
extern short currently_displayed_notes_score[PLAYER_MAX_COUNT];
extern char load_buffer[];
extern short we_dont_have_a_winner;
extern short midimaze_d8a_filehandle;
extern unsigned short *smileybuster_ptr;
extern short screen_rez;
extern unsigned short *blinzshape_ptr;
extern unsigned short *mapsmilyface_ptr;
extern short display_2d_map_flag;
extern short _random_seed;
extern short screen_flag;
extern short playerAndDroneCount;
extern short viewscreen_cell_pixels;
extern short revive_time;
extern unsigned short *mapsmily_ptr;
extern short own_number;
extern short midicam_player_number;
extern const char *sound_shot_ptr;
extern const char *sound_hit_ptr;
extern short machines_online;
extern unsigned short *shape_face_ptr;
extern short viewscreen_wind_offset;
extern short friendly_fire;
extern short popchart_wind_offset;
extern short score_notes_wind_offset;
extern char maze_datas[MAZE_MAX_SIZE*MAZE_MAX_SIZE];
extern short happiness_quotient_wind_offset;
extern short team_scores[PLAYER_MAX_TEAMS];
extern short revive_lives;
extern unsigned short *loosershape_ptr;
extern short user_is_midicam;
extern unsigned short *shape_ball_ptr;
extern OBJECT *rsrc_object_array[];
extern short wind_handle;
extern short protectionIsValid;
extern short viewscreen_hcenter;
extern short viewscreen_sky_height;
extern short midicam_player_count;
extern short midicam_timer_counter;
extern int active_drones_by_type[DRONE_TYPES];
extern short draw_shape_bodyHeight[BODY_SHAPE_COUNT];
extern short viewscreen_halfwidth;
extern unsigned short *screen_ptr[2];
extern short viewposition_direction;
extern short viewposition_y;
extern short viewposition_x;
extern short objekt_anz;
#define INPUT_JOYSTICK 0
#define INPUT_MOUSE    1
#define INPUT_KEYBOARD 2
extern short input_device;

#if BUGFIX_MIDI_TIMEOUT
extern long read_vbclock(void);
#else
extern int read_vbclock(void);
#endif
extern int get_midi(int timeout);
extern void setup_game(void);
extern void calc_drone_angle_table(void);
extern void init_dirtable(void);
extern void make_draw_list(int y,int x,int dir);
extern int init_screen(void);
extern void set_screen_offs(int offset);
extern void switch_screens(void);
extern void switch_logbase(void);
extern void switch_org_scr(void);
extern void init_sound(void);
extern void exit_sound(void);
extern void dispatch(void);
extern void init_end_shape(void);
extern int game_loop(int isSolo);
extern void midicam_keyboard_shortcuts(void);
extern void position_midicam_users(void);
extern void add_one_smily(int score,int shotPlayer);
extern void init_some_shp(void);
extern void draw_one_note_score(int player,int score);
extern void draw_all_notes_score(int playerCount);
extern void update_notes_score(int playerCount);
extern void copy_screen(void);
extern void save_part_of_screen_for_dialog(int saveFlag);
extern void bconout_dec_number(int num);
extern void bconout_string(const char *str);
extern void midicam_print_current_player(void);
extern void draw_2Dmap(void);
extern void set_ply_2Dmap(int player);
extern void drone_setup(int humanPlayers);
extern void drone_action(int player);
extern void redraw_window_background(int wi_ghandle);
extern int init_aes_window(void);
extern void exit_aes_window(void);
extern int master_solo_loop(int isSolo);
extern int do_preference_form(OBJECT *tree,int possibleDroneCount,int *droneCountPtr);
extern void strcpy_srcdst(const char *srcPtr,char *destPtr);
extern int slave_midicam_loop(int isMidicamFlag);
extern void playername_edit_dialog(int isSolo);
extern void midi_send_playernames(void);
extern void print_playername(int player);
extern int send_datas(void);
extern int receive_datas(void);
extern void rsc_draw_buttonless_dialog(int objectIndex);
extern void do_about_dialog(int objectIndex);
extern void _conterm_update_shift_status(int setFlag);
extern int LoadMaze(const char *pathname,const char *filename);
extern int get_maze_data(int fieldY,int fieldX,int flipped);
extern void set_maze_data(int fieldY,int fieldX,int val);
extern void set_all_player(void);
extern void set_object(int newObjectIndex,int y,int x);
extern void draw_mazes_set_object(int cellFY,int cellFX,int flip);
extern int _rnd(int maxVal);
extern int init_all_player(int playerCount,int isDrone);
extern int hunt_ply_pos(int player);
extern void init_joystick(void);
extern void exit_joystick(void);
extern int ask_joystick(void);
extern void init_mouse(void);
extern void exit_mouse(void);
extern int ask_mouse(void);
void init_keyboard(void);
void exit_keyboard(void);
int ask_keyboard(void);
void init_input(void);
void exit_input(void);
int ask_input(void);
extern void draw_maze_calc_viewmatrix(int microY,int microX,int minYOffset,int minXOffset,int maxYOffset,int maxXOffset,int isFlipped,int dir);
extern int draw_mazes_set_wall(int y1p,int x1p,int y2p,int x2p,int color,int leftRightFlag);
extern void objecttable_clear(void);
extern int objecttable_check_if_hidden(int xleft,int xright);
extern int objecttable_check_view_fully_covered(void);
extern void rotate2d(int *px,int *py,int angle);
extern void init_happiness_quotient_shape(void);
extern void update_happiness_quotient_indicator(void);
extern void clear_draw_list(void);
extern void to_draw_list(int type,int sprite_wallcolor,int x,int h,int x2_size,int h2_color);
extern void draw_list(void);
extern void calc_yx_to_xh(int *pinY_outX,int *pinX_outH);
extern void init_faces_shapes(void);
extern void image_double_width(unsigned short *imgPtr,long wordCount);
extern void draw_shape(int x,int size,int sprite,int yHeight,int playerIndex);
extern void calc_sin_table(void);
extern void draw_maze_generate_renderlist(int y,int x,int fieldOffsetY /* +1/-1 */,int fieldOffsetX /* +1/-1 */,int flip /* 0/1 */,int leftRightFlag /* 0/1 */);
extern int move_player(int player,int joystickData,int dronesActiveFlag);

/* From here on: the 68000 assembly code routines: */

extern void blit_draw_shape_bw(int x,int y,const unsigned short *imageMask,int widthInWords,int height,const unsigned short *ditherTable);

extern short muls_divs(short a,short b,short c);

extern void calc_color_mult_tab(void);
extern void calc_bw_mult_tab(void);

extern void blit_draw_hline_bw(int x1,int x2,int y,int col);
extern void blit_draw_hline_bw_double(int x1,int x2,int y,int col);
extern void blit_draw_vline_bw(int y1,int y2,int x,int col);
extern void blit_fill_box_bw(int x1,int y1,int x2,int y2,int col);
extern void blit_fill_box_bw_double(int x1,int y1,int x2,int y2,int col);
extern void blit_clear_window_bw(void);

extern int check_copy_protection(void);

extern void blit_draw_shape_color(int x,int y,const unsigned short *imageMask,int widthInWords,int height, SETCOLOR colFuncPtr);
extern void blit_draw_hline_color(int x1,int x2,int y,int col);
extern void blit_draw_vline_color(int y1,int y2,int x,int col);
extern void blit_fill_box_color(int x1,int y1,int x2,int y2,int col);
extern void blit_clear_window_color(void);

extern void decompress_image_to_screen(unsigned short *bytesW,unsigned short *screenPtrW);

/*
 * debugging helpers
 */
void dumpMapInfos(void);
void dumpPlayerInfo(void);

#endif
