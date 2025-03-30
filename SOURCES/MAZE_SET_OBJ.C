/************************************************************
 *** MAZE_SET_OBJ.C
 ************************************************************/
#include "GLOBALS.H"

/************************************************************
 *** Render player and shots in a given field. There is no clipping
 *** optimization: all objects in a cell are drawn (back to front)
 ************************************************************/
void draw_mazes_set_object(int cellFY,int cellFX,int flip) {
/* up to 10 objects in one field.
  Considering that a player has a radius of 96, it is practically impossible to have more
  than 5 in one field. Plus 5 shots for these players, at it's extreme.
  However, theoretically it might be: a field with no walls around it, could have 4 players
  at the corners, just inside the field. That leaves space for more 4 more players between
  them and one in the middle. Making it 9. If now all shoot at the same time towards the
  center, we might end up with more than 10 and crash. I don't remember MIDImaze ever crashing
  this way, but because stack space is not expensive, you might want to bump it up a bit. 
*/
struct {
    int distance;
    int xOffset;
    int player;
} objects[10];
int j;
int i;
int nextObject;
int objCount;
int player;
int spriteID;
int x;
int y;
int size;
int xOffset;
int distance;

    /* builds a list of all objects in that cell, already rotated */
    objCount = 0;
    for(player = get_maze_data(cellFY, cellFX, flip); player != MAZE_FIELD_EMPTY; player = nextObject) {
        if(player < PLAYER_MAX_COUNT) { /* player */
            y = player_data[player].ply_y;
            x = player_data[player].ply_x;
            nextObject = player_data[player].ply_plist;
        } else { /* shot */
            nextObject = player-PLAYER_MAX_COUNT;
            y = player_data[nextObject].ply_shooty;
            x = player_data[nextObject].ply_shootx;
            nextObject = player_data[nextObject].ply_slist;
        }
        distance = y-viewposition_y;
        xOffset = x-viewposition_x;
        rotate2d(&distance, &xOffset, viewposition_direction);
        objects[objCount].distance = distance;
        objects[objCount].xOffset = xOffset;
        objects[objCount].player = player;
        if(++objCount >= sizeof(objects)/sizeof(objects[0])) break;
    }

    /* sort the array by distance (in the most inefficient way, but the array is only max. 10 elements long) */
    for(i = objCount-1; i > 0; i--) {
        for(j = 0; j < i; j++) {
            if(objects[j].distance <= objects[j+1].distance)
                continue;
            /* swap objects */
            nextObject = objects[j].distance; objects[j].distance = objects[j+1].distance; objects[j+1].distance = nextObject;
            nextObject = objects[j].xOffset; objects[j].xOffset = objects[j+1].xOffset; objects[j+1].xOffset = nextObject;
            nextObject = objects[j].player; objects[j].player = objects[j+1].player; objects[j+1].player = nextObject;
        }
    }

    /* add all objects in the list to our draw list */
    while(objCount > 0) {
        objCount--;
        player = objects[objCount].player;
        distance = objects[objCount].distance;
        xOffset = objects[objCount].xOffset;
        if(player < PLAYER_MAX_COUNT) { /* player */
            if(player != own_number && distance < 0) { /* don't draw our own shape or anything behind up */
                if((size = -4000/distance) < 1) { /* scale by the distance */
                    size = 1;
                }
                if(size > 32) size = 32; /* maximum size of the shape */
                if(screen_rez) size <<= 1;
                /* x offset: */
                y = viewscreen_hcenter-muls_divs(xOffset, viewscreen_halfwidth, distance);
                if(!objecttable_check_if_hidden(y-size, y+size-1)) {
                    /* 32 faces for every 11.25 degrees (256/32 = 8) */
                    static const short face_shape_tab[32] = { 0,19,18,17,16,15,14,13,12,11,10,10,10,10,10,10,10,10,10,10,10,10,9,8,7,6,5,4,3,2,1,0 };
                    /* The first part is the viewing angle between player and our viewposition, */
                    /* if both face each other directly. If the player has an xOffset, this would not be correct. */
                    /* It needs to be compensated, which is the (xOffset * 32)/distance. */
                    spriteID = (player_data[player].ply_dir-128-viewposition_direction)+(xOffset*32)/distance;
                    spriteID = face_shape_tab[(spriteID>>3)&0x1f];
                    /* The shadow offset is below the sky horizon, it also needs to be scaled as well. */
                    /* It is (20 * MAZE_CELL_SIZE) / distance in color (double that for b/w). */
                    x = viewscreen_sky_height+1 - (viewscreen_cell_pixels * MAZE_CELL_SIZE)/distance;
                    to_draw_list(DRAW_TYPE_PLAYER, spriteID, y-size, x, size, player_data[player].ply_hitflag ? player_data[player].ply_gunman : player);
                }
            }
        } else { /* a shot */
            player -= PLAYER_MAX_COUNT;

            /* Not sure, why the shot is rotated again. It's already in the list (see player above) */
            /* These 3 lines can just be removed. */
            distance = player_data[player].ply_shooty-viewposition_y;
            xOffset = player_data[player].ply_shootx-viewposition_x;
            rotate2d(&distance, &xOffset, viewposition_direction);

            /* ignore a shot that is behind the player */
            if(distance < 0 && ((xOffset >= 0 && -distance >= xOffset) || (xOffset < 0 && distance <= xOffset))) {
                if((size = -1000/distance) == 0) { /* a shot is 1/4 of the size of a player */
                    size = 1;
                }
                if(size > 32) size = 32; /* maximum size of the shape */
                if(screen_rez) size <<= 1;
                /* x offset, which needs to be scaled to the width of the window relative to the distance. */
                y = viewscreen_hcenter-muls_divs(xOffset, viewscreen_halfwidth, distance);
                    /* The shadow offset is below the sky horizon, it also needs to be scaled as well. */
                    /* It is (20 * MAZE_CELL_SIZE) / distance in color (double that for b/w). */
                x = viewscreen_sky_height+1-(viewscreen_cell_pixels * MAZE_CELL_SIZE)/distance;
                if(!objecttable_check_if_hidden(y-size, y+size-1)) {
                    to_draw_list(DRAW_TYPE_SHOT, 0, y-size, x, size, player);
                }
            }
        }
    }
}
