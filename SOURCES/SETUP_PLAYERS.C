/************************************************************
 *** SETUP_PLAYERS.C
 ************************************************************/
#include "GLOBALS.H"

/***********************************************************
 *** At game start initialize all player variables and position them in the maze
 ************************************************************/
int init_all_player(int playerCount,int isDrone) {
int j;
int i;

    playerAndDroneCount = we_dont_have_a_winner = playerCount;

    /* remove all objects from the maze */
    for(i = 1; i <= MAZE_MAX_SIZE-1; i += 2) {
        for(j = 1; j <= MAZE_MAX_SIZE-1; j += 2) {
            set_maze_data(i, j, MAZE_FIELD_EMPTY);
        }
    }
    objekt_anz = 0; /* reset number of objects in a given cell */

    /* remove all lives */
    for(i = 0; i < playerAndDroneCount; i++)
        player_data[i].ply_lives = 0;

    /* find position for all players and drones */
    for(i = 0; i < playerAndDroneCount; i++) {
        if(!hunt_ply_pos(i))
            return NO; /* maze too small for all players */

        /* defaults for the start of the game */
        player_data[i].ply_lives = PLAYER_MAX_LIVES; /* we always start with PLAYER_MAX_LIVES lives */
        player_data[i].ply_refresh = 0; /* no refresh necessary */
        player_data[i].ply_shoot = 0; /* no shot active */
        player_data[i].ply_reload = 0; /* not reloading */
        player_data[i].ply_score = 0; /* no score */
        player_data[i].ply_hitflag = FALSE; /* not currently shot */
        if(isDrone) {
            player_data[i].dr_rotateCounter = 0;
            player_data[i].dr_upRotationCounter = 0;
            player_data[i].dr_joystick = 0;
            player_data[i].dr_dir[0] = 0;
            player_data[i].dr_targetLocked = FALSE;
            player_data[i].dr_isInactive = FALSE;
            player_data[i].dr_fieldResetTimer = 0;
        }
    }
    return YES;
}

/************************************************************
 *** Find a new position in a maze for a given player. This is called at init or after a player died.
 *** It is also called if the collision function triggers it's bug and a player ends up in a closed square.
 *** If the maze is too small for all players, it will return NO (and the game ends)
 ************************************************************/
int hunt_ply_pos(int player) {
int distance;
int dir;
int deltaX;
int deltaY;
int i;
int noValidPositionFound;
int wallCount;
int fieldX;
int fieldY;
int tries;

    noValidPositionFound = TRUE; /* so far: nothing found */
    for(tries = 0; tries < 666 && noValidPositionFound; tries++) {
        /* pick a random field */
        fieldY = _rnd(maze_size)|1;
        fieldX = _rnd(maze_size)|1;
        if(get_maze_data(fieldY, fieldX, 0) != MAZE_FIELD_EMPTY)  /* is the field available? */
            continue;   /* => no */

        /* count the walls around this field */
        wallCount = 0;
        if(get_maze_data(fieldY-1, fieldX, 0) == MAZE_FIELD_WALL)
            wallCount++;
        if(get_maze_data(fieldY, fieldX-1, 0) == MAZE_FIELD_WALL)
            wallCount++;
        if(get_maze_data(fieldY+1, fieldX, 0) == MAZE_FIELD_WALL)
            wallCount++;
        if(get_maze_data(fieldY, fieldX+1, 0) == MAZE_FIELD_WALL)
            wallCount++;
        if(wallCount == 4) /* player is surrounded by walls => try a new position */
            continue;

        noValidPositionFound = FALSE; /* found one! */
        player_data[player].ply_y = fieldY << MAZE_FIELD_SHIFT;
        player_data[player].ply_x = fieldX << MAZE_FIELD_SHIFT;

        /* minimum distance to the next player/drone */
        /* This starts at 5 full fields (out of immediate attack range for drones) */
        /* at try 0 and goes down to 0 after 20 tries. */
        distance = (5-tries/20) * MAZE_CELL_SIZE;

        for(i = 0; i < playerAndDroneCount; i++) {
            if(player == i) /* ignore ourselves */
                continue;
            if(player_data[i].ply_lives <= 0) /* also dead players can be ignored */
                continue;

            deltaY = player_data[i].ply_y-player_data[player].ply_y;
            deltaY = abs(deltaY);
            deltaX = player_data[i].ply_x-player_data[player].ply_x;
            deltaX = abs(deltaX);
            if(deltaY < distance || deltaX < distance) {
                noValidPositionFound = TRUE; /* too close, we don't take the positon */
                break;
            }
        }
    }
    /* nothing found => this is a fatal fail for MIDImaze */
    if(noValidPositionFound)
        return NO;

    /* position the player into the maze */
    set_object(player, player_data[player].ply_y, player_data[player].ply_x);
    player_data[player].ply_plist = -1;

    /* find a random position */
    /* This could handled much simpler, by just writing dir = _rnd(256) & 0xc0 */
    dir = _rnd(256) & 0xf8;
    if(dir < PLAYER_DIR_EAST) {
        dir = PLAYER_DIR_NORTH;
    } else {
        dir = (dir < PLAYER_DIR_SOUTH) ? PLAYER_DIR_EAST : dir < PLAYER_DIR_WEST ? PLAYER_DIR_SOUTH : PLAYER_DIR_WEST;
    }
    player_data[player].ply_dir = dir;

    /* setup drone variables */
    if(player_data[player].dr_type == DRONE_NINJA || player_data[player].dr_type == DRONE_STANDARD || player_data[player].dr_type == DRONE_TARGET) {
        switch(player_data[player].dr_type) {
        case DRONE_NINJA:
            player_data[player].dr_dir[0] = 0;
            player_data[player].dr_fieldIndex = 0;
            player_data[player].dr_fieldResetTimer = 0;
        case DRONE_STANDARD:
            player_data[player].dr_targetLocked = FALSE;
            player_data[player].dr_isInactive = FALSE;
        case DRONE_TARGET:
            player_data[player].dr_rotateCounter = 0;
            player_data[player].dr_upRotationCounter = 0;
            player_data[player].dr_joystick = 0;
            break;
        default:
            break;
        }
    }

    /* success */
    return YES;
}
