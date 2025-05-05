/************************************************************
 *** MAZE_DRAWLIST.C
 ************************************************************/
#include "globals.h"

/* This table allows us to use one code path for building the draw list, depending on the viewing direction of the player */
static struct {
    int minY;
    int minX;
    int maxY;
    int maxX;
    int fieldOffsetY;
    int fieldOffsetX;
    int flipped;
} dir_table[8] = { 
        {-7,-7, 1, 9,  -1, 1,  0}, /* N */
        { 8, 8,-8, 0,   1,-1,  1}, /* NE */
        {-7, 8, 9, 0,   1, 1,  1}, /* E */
        { 8,-7, 0, 9,   1, 1,  0}, /* SE */
        { 8, 8, 0,-8,   1,-1,  0}, /* S */
        {-7,-7, 9, 1,  -1, 1,  1}, /* SW */
        { 8,-7,-8, 1,  -1,-1,  1}, /* W */
        {-7, 8, 1,-8,  -1,-1,  0}, /* NW */
        };
short viewposition_direction;
short viewposition_y;
short viewposition_x;

/************************************************************
 *** void init_dirtable(void)
 ************************************************************/
void init_dirtable(void) {
int i;

    /* probably for easy typing the table has plain integers (fieldX/fieldY offsets) */
    /* instead of the actual units (MAZE_CELL_SIZE * field) */
    for(i = 0; i < sizeof(dir_table)/sizeof(dir_table[0]); i++) {
        dir_table[i].minY *= MAZE_CELL_SIZE;
        dir_table[i].minX *= MAZE_CELL_SIZE;
        dir_table[i].maxY *= MAZE_CELL_SIZE;
        dir_table[i].maxX *= MAZE_CELL_SIZE;
    }
}

/************************************************************
 *** void make_draw_list(int y,int x,int dir)
 ************************************************************/
void make_draw_list(int y,int x,int dir) {
int compassDir;

    /* keep the position/direction of the current player/MIDIcam/etc */
    viewposition_y = y;
    viewposition_x = x;
    viewposition_direction = dir;
    compassDir = (dir >> 5) & 7; /* fold the direction into 8 general directions, matching dir_table[] */

    /* calculate viewmatrix_delta[][] based on the direction */
    draw_maze_calc_viewmatrix(y & (MAZE_CELL_SIZE-1), x & (MAZE_CELL_SIZE-1),
                              dir_table[compassDir].minY, dir_table[compassDir].minX,
                              dir_table[compassDir].maxY, dir_table[compassDir].maxX,
                              dir_table[compassDir].flipped, dir);

    set_all_player(); /* position all players/drones into the maze */

    /* now generate the whole render list with all walls, players and shots */
    /* This takes visibility and clipping into account */
    draw_maze_generate_renderlist(y, x,
                                  dir_table[compassDir].fieldOffsetY, dir_table[compassDir].fieldOffsetX,
                                  dir_table[compassDir].flipped, compassDir & 1);
}
