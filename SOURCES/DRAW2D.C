/************************************************************
 *** Routines to draw the 2D map
 ************************************************************/
#include "GLOBALS.H"

#define BORDER_WIDTH    3

/* border around the map */
#define MAP_BORDER (screen_rez ? BORDER_WIDTH*2 : BORDER_WIDTH)
/* maximum width of the map */
#define MAP_WIDTH (screen_rez ? (320-(BORDER_WIDTH*2)*2-1) : (160-BORDER_WIDTH*2-1))
/* maximum height of the map */
#define MAP_HEIGHT (screen_rez ? (200-(BORDER_WIDTH*2)*2) : (100-BORDER_WIDTH*2))


/************************************************************
 *** Draw the 2D map
 ************************************************************/
void draw_2Dmap(void) {
int x;
int y;

    /* erase map window */
    if(screen_rez)
        blit_fill_box_bw(MAP_BORDER-6, MAP_BORDER-6, MAP_BORDER+MAP_WIDTH+6, MAP_BORDER+MAP_HEIGHT+6, COLOR_MAGNESIUM_INDEX);
    else
        blit_fill_box_color(MAP_BORDER-3, MAP_BORDER-3, MAP_BORDER+MAP_WIDTH+3, MAP_BORDER+MAP_HEIGHT+3, COLOR_STEEL_INDEX);

    for(y = 0; y <= maze_size; y += 2) {
        for(x = 1; x < maze_size; x += 2) {
            /* 1st: draw all horizontal maze walls */
            if(get_maze_data(y, x, 0) == MAZE_FIELD_WALL) {
                int yc = MAP_HEIGHT*y/maze_size+MAP_BORDER;
                int x1 = MAP_WIDTH*(x-1)/maze_size+MAP_BORDER;
                int x2 = MAP_WIDTH*(x+1)/maze_size+MAP_BORDER;
                if(screen_rez)
                    blit_draw_hline_bw(x1, x2, yc, COLOR_BLACK_INDEX);
                else
                    blit_draw_hline_color(x1, x2, yc, COLOR_MAGNESIUM_INDEX);
            }
            /* 2nd: draw all vertical maze walls */
            if(get_maze_data(y, x, 1) == MAZE_FIELD_WALL) {
                int xc = MAP_WIDTH*y/maze_size+MAP_BORDER;
                int y1 = MAP_HEIGHT*(x-1)/maze_size+MAP_BORDER;
                int y2 = MAP_HEIGHT*(x+1)/maze_size+MAP_BORDER;
                if(screen_rez)
                    blit_draw_vline_bw(y1, y2, xc, COLOR_BLACK_INDEX);
                else
                    blit_draw_vline_color(y1, y2, xc, COLOR_MAGNESIUM_INDEX);
            }
        }
    }
#if DEBUG_2D_MAZE /* This draws the current movement plan of a ninja into the map */
    for(int ply = 0; ply < playerAndDroneCount; ply++) {
        if(!player_data[ply].dr_dir[0]) /* no plan? */
            continue;
        y = player_data[ply].dr_field[0].y;
        x = player_data[ply].dr_field[0].x;
        for(int j=1; j<6; j++) {
            if(player_data[ply].dr_dir[j] < 0) /* normal exist */
                break;
            if(player_data[ply].dr_field[j].y == 0) /* kill shot possible */
                break;
            if(x == player_data[ply].dr_field[j].x) {
                int xc = MAP_WIDTH*x/maze_size+MAP_BORDER;
                int y1 = MAP_HEIGHT*y/maze_size+MAP_BORDER;
                int y2 = MAP_HEIGHT*player_data[ply].dr_field[j].y/maze_size+MAP_BORDER;
                if(screen_rez)
                    blit_draw_vline_bw(y1, y2, xc, color_cnv_back[ply]);
                else
                    blit_draw_vline_color(y1, y2, xc, color_cnv_back[ply]);
                y = player_data[ply].dr_field[j].y;
            } else {
                int yc = MAP_HEIGHT*y/maze_size+MAP_BORDER;
                int x1 = MAP_WIDTH*x/maze_size+MAP_BORDER;
                int x2 = MAP_WIDTH*player_data[ply].dr_field[j].x/maze_size+MAP_BORDER;
                if(screen_rez)
                    blit_draw_hline_bw(x1, x2, yc, color_cnv_back[ply]);
                else
                    blit_draw_hline_color(x1, x2, yc, color_cnv_back[ply]);
                x = player_data[ply].dr_field[j].x;
            }
        }
    }
#endif
}

/************************************************************
 *** Draw player shapes onto the map
 ************************************************************/
void set_ply_2Dmap(int player) {
int posY;
int posX;
int playerX;
int playerY;

    playerY = player_data[player].ply_y;
    playerX = player_data[player].ply_x;
    posX = muls_divs(MAP_WIDTH, playerX, maze_size << MAZE_FIELD_SHIFT)+MAP_BORDER-2;
    posY = muls_divs(MAP_HEIGHT, playerY, maze_size << MAZE_FIELD_SHIFT)+MAP_BORDER+2;
    if(screen_rez) {
        /* This is just a 10x10 circle, drawn with the color (pattern) of the player */
        blit_draw_shape_bw(posX-2, posY+2, mapsmily_ptr+2*5, 2, 5, bw_fillpattern_table[color_cnv_back[player]]);
        /* A face doesn't work well in b/w because of the texture it looks really bad */
    } else {
        /* This is just a 5x5 circle, drawn with the color of the player */
        blit_draw_shape_color(posX, posY, mapsmily_ptr+1*5, 1, 5, col_setcolor_jumptable[color_cnv_back[player]]);
        /* This is just the eyes and mouth of the player, drawn in the frame color */
        blit_draw_shape_color(posX, posY, mapsmilyface_ptr+1*5, 1, 5, col_setcolor_jumptable[color_cnv_frame[player]]);
    }
}
