/************************************************************
 *** MAZE_OBJECT.C
 ************************************************************/
#include "globals.h"

char maze_datas[MAZE_MAX_SIZE*MAZE_MAX_SIZE];
short objekt_anz = 0;
static struct {
    short y;
    short x;
    short index;
} object_table[PLAYER_MAX_COUNT*2]; /* players + one shot per player at once */

/************************************************************
 *** int get_maze_data(int fieldY,int fieldX,int flipped)
 ************************************************************/
int get_maze_data(int fieldY,int fieldX,int flipped) {
    /* the area outside of the maze returns 1x1 boxes */
    /* these boxes allow us to detect easily illegal fields, in case */
    /* the player accidentially ends up on one. */
    if(fieldY < 0 || fieldY > MAZE_MAX_SIZE-1 || fieldX < 0 || fieldX > MAZE_MAX_SIZE-1)
        return (fieldY & fieldX & 1) ? MAZE_FIELD_EMPTY : MAZE_FIELD_WALL;
    return !flipped ? maze_datas[fieldY*MAZE_MAX_SIZE+fieldX] : maze_datas[fieldX*MAZE_MAX_SIZE+fieldY];
}

/************************************************************
 *** void set_maze_data(int fieldY,int fieldX,int val)
 ************************************************************/
void set_maze_data(int fieldY,int fieldX,int val) {
    if(fieldY < 0 || fieldY > MAZE_MAX_SIZE-1 || fieldX < 0 || fieldX > MAZE_MAX_SIZE-1)
        return;
    maze_datas[fieldY*MAZE_MAX_SIZE+fieldX] = val;
}

/************************************************************
 *** void set_all_player(void)
 ************************************************************/
void set_all_player(void) {
int i;

    /* remove all objects from the maze */
    while(objekt_anz > 0) {
        objekt_anz--;
        set_maze_data(object_table[objekt_anz].y, object_table[objekt_anz].x, MAZE_FIELD_EMPTY);
    }
    /* set all players and their shots */
    for(i = 0; i < playerAndDroneCount; i++) {
        /* only position players which are either alive or still in the "got shot" state */
        if(player_data[i].ply_lives > 0 || player_data[i].ply_hitflag) {
            set_object(i, player_data[i].ply_y, player_data[i].ply_x);
            player_data[i].ply_plist = MAZE_FIELD_EMPTY;
        }
        /* if the player has a shot in the air, set that one as well */
        if(player_data[i].ply_shoot > 0) {
            set_object(i+PLAYER_MAX_COUNT, player_data[i].ply_shooty, player_data[i].ply_shootx);
            player_data[i].ply_slist = MAZE_FIELD_EMPTY;
        }
    }
}

/************************************************************
 *** void set_object(int newObjectIndex,int y,int x)
 ************************************************************/
void set_object(int newObjectIndex,int y,int x) {
int nextObject;
int mazaFieldData;
int fieldX;
int fieldY;

    fieldY = (y >> MAZE_FIELD_SHIFT)|1;
    fieldX = (x >> MAZE_FIELD_SHIFT)|1;
    mazaFieldData = get_maze_data(fieldY, fieldX, 0);
    if(mazaFieldData == MAZE_FIELD_EMPTY) { /* field is empty? */
        /* we can just put it into that field */
        set_maze_data(fieldY, fieldX, newObjectIndex);
        object_table[objekt_anz].y = fieldY;
        object_table[objekt_anz].x = fieldX;
        object_table[objekt_anz++].index = newObjectIndex;

    } else {
        /* otherwise find the end of the object list */
        do {
            nextObject = (mazaFieldData < PLAYER_MAX_COUNT) ? player_data[mazaFieldData].ply_plist : player_data[mazaFieldData-PLAYER_MAX_COUNT].ply_slist;
            if(nextObject == MAZE_FIELD_EMPTY) break; /* empty? */
            mazaFieldData = nextObject;
        } while(1);
        /* append to the end of the list */
        if(mazaFieldData < PLAYER_MAX_COUNT) {
            player_data[mazaFieldData].ply_plist = newObjectIndex;
        } else {
            player_data[mazaFieldData-PLAYER_MAX_COUNT].ply_slist = newObjectIndex;
        }
    }
}
