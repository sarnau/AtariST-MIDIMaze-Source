/************************************************************
 *** MAKE_RENDERLIST.C
 ************************************************************/
#include "GLOBALS.H"

/************************************************************
 *** void draw_maze_generate_renderlist(int y,int x,int fieldOffsetY,int fieldOffsetX,int flip,int leftRightFlag)
 ************************************************************/
void draw_maze_generate_renderlist(int y,int x,int fieldOffsetY /* +1/-1 */,int fieldOffsetX /* +1/-1 */,int flip /* 0/1 */,int leftRightFlag /* 0/1 */) {
register int viewingWidth;
register int viewingDistance;
register int fieldFX;
register int fieldFY;
int fieldX;
int fieldY;
int _fieldX;
int _fieldY;

    objecttable_clear();
    clear_draw_list();

    _fieldY = (y >> MAZE_FIELD_SHIFT)|1;
    _fieldX = (x >> MAZE_FIELD_SHIFT)|1;
    if(!flip) {
        fieldY = _fieldY;
        fieldX = _fieldX;
    } else {
        fieldY = _fieldX;
        fieldX = _fieldY;
    }

    /*
       Rendering example always uses North for simplicity.
       In other cases the variables simply will have different values,
       because the maze is rotated. It doesn't change the algorithm. 
       fieldOffsetY = -1, fieldOffsetX = 1, flip = 0

       We start in the fields in front of the player and move up to 8 fields away.
       If at any point the whole view is covered, then the algorithm stops.
    */

    /* start from the very front and move to the back, up to 8 fields (which are 4 maze blocks) */
    fieldFY = fieldY;
    for(viewingDistance = 7; viewingDistance >= 0; viewingDistance--) {

        /* render walls running towards the horizon on and left of the center line */
        viewingWidth = 7; /* maximum of 8 fields to the left */
        fieldFX = fieldX; /* start: current field of the player */
        do {
            /* 1. render the actual field (which can only contain a player or shot) */
            draw_mazes_set_object(fieldFY, fieldFX, flip);
            fieldFX -= fieldOffsetX;
            /* 2. render walls, which are one field left */
            if(get_maze_data(fieldFY, fieldFX, flip) == MAZE_FIELD_WALL) {
                if(draw_mazes_set_wall(viewingDistance, viewingWidth, viewingDistance+1, viewingWidth, flip, leftRightFlag^1))
                    break; /* stop loop, if wall is invisible via viewport clipping anyway */
            }
            if(objecttable_check_view_fully_covered()) break;
            /* 3. go to the next field, which will be a player/shot again */
            fieldFX -= fieldOffsetX;
        } while(--viewingWidth >= 0);

        /* render walls running towards the horizon right of the center line */
        viewingWidth = 8; /* maximum of 8 fields to the right */
        fieldFX = fieldX; fieldFX += fieldOffsetX; /* start: field to the right of the player */
        do {
            /* 1. render walls, which are one field left */
            if(get_maze_data(fieldFY, fieldFX, flip) == MAZE_FIELD_WALL) {
                if(draw_mazes_set_wall(viewingDistance, viewingWidth, viewingDistance+1, viewingWidth, flip, leftRightFlag))
                    break; /* stop loop, if wall is invisible via viewport clipping anyway */
            }
            if(objecttable_check_view_fully_covered()) break;
            if(viewingWidth == 16) break;
            /* 2. go to the next field, which will be a player/shot and render player/shot */
            fieldFX += fieldOffsetX;
            draw_mazes_set_object(fieldFY, fieldFX, flip);
            /* 3. go to the next field, which will be a wall again */
            fieldFX += fieldOffsetX;
            viewingWidth++;
        } while(1);

        /* render walls running parallel the horizon left of the center line */
        fieldFY += fieldOffsetY;
        fieldFX = fieldX; /* start: current field of the player */
        for(viewingWidth = 7; viewingWidth >= 0; viewingWidth--) {
            if(get_maze_data(fieldFY, fieldFX, flip) == MAZE_FIELD_WALL) {
                if(draw_mazes_set_wall(viewingDistance, viewingWidth, viewingDistance, viewingWidth+1, flip^1, leftRightFlag^1))
                    break; /* stop loop, if wall is invisible via viewport clipping anyway */
            }
            if(objecttable_check_view_fully_covered()) break;
            fieldFX -= fieldOffsetX+fieldOffsetX;
        }

        /* render walls running parallel the horizon right of the center line */
        fieldFX = fieldX; fieldFX += fieldOffsetX; fieldFX += fieldOffsetX; /* start: field right of the player, Y is unchanged */
        for(viewingWidth = 8; viewingWidth < 16; viewingWidth++) {
            if(get_maze_data(fieldFY, fieldFX, flip) == MAZE_FIELD_WALL) {
                if(draw_mazes_set_wall(viewingDistance, viewingWidth, viewingDistance, viewingWidth+1, flip^1, leftRightFlag))
                    break; /* stop loop, if wall is invisible via viewport clipping anyway */
            }
            if(objecttable_check_view_fully_covered()) break;
            fieldFX += fieldOffsetX+fieldOffsetX;
        }
        fieldFY += fieldOffsetY;

        if(objecttable_check_view_fully_covered()) break;
    }
}
