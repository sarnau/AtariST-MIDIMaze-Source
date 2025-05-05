/************************************************************
 *** Movement of players and shots, collisions, etc.
 *** This is were all the magic happens!
 ************************************************************/
#include "globals.h"

/* These sizes are just for collision detection. They don't define how the shape is drawn. */
/* e.g. a shot has no radius at all, it hits if it enters the radius of the player. */

#define PLAYER_RADIUS 48 /* radius of a player shape */
#define PLAYER_WALL_DISTANCE 65 /* minimum distance of a player to a wall (> than it's radius)  */
/* A maze field is MAZE_CELL_SIZE units wide, so a player with a radius of 48 is small enough */
/* that typicall another player can squezze by (65+48) * 2 < MAZE_CELL_SIZE */

static void move_shoot(int player);

short reload_time;
short regen_time;
short revive_time;
short revive_lives;
short friendly_fire;
short team_flag;
PLAYER_DATA player_data[PLAYER_MAX_COUNT];

static XY_SPEED_TABLE xy_speed_table[256]; /* convert an angle into a two speed components: y and x */

/************************************************************
 *** void calc_sin_table(void)
 ************************************************************/
void calc_sin_table(void) {
int x;
int y;
int angle;

    for(angle = 0; angle < 256; angle++) {
        y = -PLAYER_MOTION_SPEED; /* This is the maximum speed for a player per calculation loop */
        x = 0;
        rotate2d(&y, &x, -angle);
        xy_speed_table[angle].deltaY = y;
        xy_speed_table[angle].deltaX = x;
    }
}

/************************************************************
 *** int move_player(int player,int joystickData,int dronesActiveFlag)
 ************************************************************/
int move_player(register int player,int joystickData,int dronesActiveFlag) {
register int newFieldX;
register int newFieldY;
register int playerX;
register int playerY;
int surroundingWalls;
int oldDiffX;
int oldDiffY;
int speedX;
int speedY;
int objectID;
int distanceX;
int distanceY;
int fieldX;
int fieldY;
int bumpOfWalls;
int xOffset;
int yOffset;
int tooCloseRight;
int tooCloseLeft;
int tooCloseBottom;
int tooCloseTop;
int direction;
int cellXfract;
int cellYfract;

    /* Is a refresh pending (which will add another live to the player) */
    /* decrement the refresh counter, if active */
    if(player_data[player].ply_refresh > 0 && --player_data[player].ply_refresh == 0) {
        /* add another live, but limit to a maximum of PLAYER_MAX_LIVES */
        if(++player_data[player].ply_lives < PLAYER_MAX_LIVES) {
            /* only if we have less than PLAYER_MAX_LIVES lives, we'll start the refresh timer again */
            player_data[player].ply_refresh = regen_time;
        }
        /* lives == 1 means, that the player was at 0 lives before (= dead) */
        if(player_data[player].ply_lives == 1) {
            /* set the lives to the number set in the preference */
            if((player_data[player].ply_lives = revive_lives) == PLAYER_MAX_LIVES) {
                /* do not refresh (=add more lives) if already at PLAYER_MAX_LIVES */
                player_data[player].ply_refresh = 0;
            }
            /* find a new position for the player in the maze */
            if(!hunt_ply_pos(player))
                /* the maze is too small for all players, this is fatal! */
                return FAILURE;

            /* now we hide the map and start the game! Because the player is alive again, */
            /* it became visible to other players and can also be shot. */
            if(player == own_number)
                display_2d_map_flag = NO;
        }
        /* if we are the current player, make a little sound that we received an addiontal live */
        if(player == own_number) {
            Bconout(CON, 7); /* BELL sound */
            update_happiness_quotient_indicator(); /* update the live status to reflect this change */
        }
    }
    /* is the player dead? */
    if(player_data[player].ply_lives == 0) {
        /* if the shot is still active, continue tracking it */
        if(player_data[player].ply_shoot)
            move_shoot(player);
        /* also still maintain the reload timer */
        if(player_data[player].ply_reload)
            player_data[player].ply_reload--;
        return SUCCESS;
    }

    /* deal with the rotation */
    playerY = player_data[player].ply_y;
    playerX = player_data[player].ply_x;
    direction = player_data[player].ply_dir;
    fieldY = (playerY >> MAZE_FIELD_SHIFT)|1;
    fieldX = (playerX >> MAZE_FIELD_SHIFT)|1;
    if(joystickData & JOYSTICK_LEFT)
        direction -= PLAYER_MOTION_ROTATE;
    else if(joystickData & JOYSTICK_RIGHT)
        direction += PLAYER_MOTION_ROTATE;
    direction &= 255; /* mask to keep the direction in the 0..255 range */

    /* fire a shot */
    if(((joystickData & JOYSTICK_BUTTON) == JOYSTICK_BUTTON) && player_data[player].ply_reload == 0) { /* button pressed and not waiting for a reload? */
        if(player == own_number) /* only play the sound, if the player on this computer is affected */
            Dosound((void*)sound_shot_ptr);

        player_data[player].ply_shootr = direction; /* the direction of the shot matches the viewing direction of the player */
        player_data[player].ply_shoot = 10; /* a shot is active (this was probably a timer to let the shot expire after a certain distance, now it is just a fleg) */
        player_data[player].ply_reload = reload_time; /* delay the next shot */
        player_data[player].ply_shooty = playerY; /* the position of the shot is equal to the player position */
        player_data[player].ply_shootx = playerX;
    }
    if(player_data[player].ply_shoot) /* is a shot active? */
        move_shoot(player); /* then move and track it */

    if(player_data[player].ply_reload) /* is the reload timer active? */
        player_data[player].ply_reload--; /* then decrement it */

    /* movement of the player */
    if(joystickData & JOYSTICK_UP) {
        speedY = xy_speed_table[direction].deltaY;
        speedX = xy_speed_table[direction].deltaX;
    } else if(joystickData & JOYSTICK_DOWN) {
        speedY = -xy_speed_table[direction].deltaY;
        speedX = -xy_speed_table[direction].deltaX;
    } else {
        speedY = 0; /* otherwise: no speed */
        speedX = 0;
    }

    playerY = player_data[player].ply_y; playerY += speedY;
    playerX = player_data[player].ply_x; playerX += speedX;
    /* current field of the player, this is used to find walls around the player */
    newFieldY = playerY; newFieldY >>= MAZE_FIELD_SHIFT; newFieldY |= 1;
    newFieldX = playerX; newFieldX >>= MAZE_FIELD_SHIFT; newFieldX |= 1;
    bumpOfWalls = TRUE;
    /* check all objects around us */
    for(yOffset = -2; yOffset <= 2; yOffset += 2) {
        for(xOffset = -2; xOffset <= 2; xOffset += 2) {
            /* get the object on that field */
            objectID = get_maze_data(yOffset+newFieldY, xOffset+newFieldX, 0);
            while(objectID != MAZE_FIELD_EMPTY) { /* iterate the whole list */
                if(objectID < PLAYER_MAX_COUNT) { /* a player (>= PLAYER_MAX_COUNT are shots) */
                    if(player != objectID) { /* ignore the player itself */
                        distanceY = player_data[objectID].ply_y-playerY;
                        distanceY = abs(distanceY);
                        distanceX = player_data[objectID].ply_x-playerX;
                        distanceX = abs(distanceX);
                        /* Are we too close to another player? */
                        if(distanceY < PLAYER_RADIUS*2 && distanceX < PLAYER_RADIUS*2) {
                            /* calculate the old delta, before the movement was added */
                            oldDiffY = player_data[objectID].ply_y-player_data[player].ply_y;
                            oldDiffY = abs(oldDiffY);
                            oldDiffX = player_data[objectID].ply_x-player_data[player].ply_x;
                            oldDiffX = abs(oldDiffX);
                            /* allow to touch, but not to intersect */
                            /* BUG: This is not a stable test, because the player be pushed into a different field, */
                            /* which could be a closed box and then the player is teleported somewhere else. */
                            /* Also: probably not nice: a player that didn't move at all, can be pushed away, */
                            /* if a different player is moving. */
                            if(oldDiffY < PLAYER_RADIUS*2) {
                                speedX -= (speedX < 0) ? -(PLAYER_RADIUS*2-distanceX) : PLAYER_RADIUS*2-distanceX;
                            } else if(oldDiffX < PLAYER_RADIUS*2) {
                                speedY -= (speedY < 0) ? -(PLAYER_RADIUS*2-distanceY) : PLAYER_RADIUS*2-distanceY;
                            } else {
                                if(distanceY > distanceX) {
                                    speedY -= (speedY < 0) ? -(PLAYER_RADIUS*2-distanceY) : PLAYER_RADIUS*2-distanceY;
                                } else {
                                    speedX -= (speedX < 0) ? -(PLAYER_RADIUS*2-distanceX) : PLAYER_RADIUS*2-distanceX;
                                }
                            }
                            /* calculate the new position after the fix */
                            playerY = player_data[player].ply_y;
                            playerY += speedY;
                            playerX = player_data[player].ply_x;
                            playerX += speedX;
                        }
                    }
                    /* pick the next object */
                    objectID = player_data[objectID].ply_plist;
                } else {
                    /* pick the next object */
                    objectID = player_data[objectID-PLAYER_MAX_COUNT].ply_slist;
                }
            }
        }
    }

    /* new field of the player, this is used to find walls around the player */
    newFieldY = playerY; newFieldY >>= MAZE_FIELD_SHIFT; newFieldY |= 1;
    newFieldX = playerX; newFieldX >>= MAZE_FIELD_SHIFT; newFieldX |= 1;
    cellYfract = playerY & (MAZE_CELL_SIZE-1);
    cellXfract = playerX & (MAZE_CELL_SIZE-1);
    /* check if the player is potentially too close to a wall */
    tooCloseTop = cellYfract < PLAYER_WALL_DISTANCE;
    tooCloseBottom = cellYfract > (MAZE_CELL_SIZE-PLAYER_WALL_DISTANCE);
    tooCloseLeft = cellXfract < PLAYER_WALL_DISTANCE;
    tooCloseRight = cellXfract > (MAZE_CELL_SIZE-PLAYER_WALL_DISTANCE);
    yOffset = tooCloseTop ? -1 : tooCloseBottom ? 1 : 0;
    xOffset = tooCloseLeft ? -1 : tooCloseRight ? 1 : 0;
    if(bumpOfWalls) { /* always true at this point */
        /* are we too close on the Y axis? */
        if((tooCloseTop || tooCloseBottom) && get_maze_data(yOffset+newFieldY, newFieldX, 0) == MAZE_FIELD_WALL) {
            playerY &= ~(MAZE_CELL_SIZE-1); /* stay in the field, but remove the position within the field */
            playerY += tooCloseBottom ? (MAZE_CELL_SIZE-PLAYER_WALL_DISTANCE) : PLAYER_WALL_DISTANCE; /* adjust to stay as close to the wall as possible */
            bumpOfWalls = FALSE;
        }
        /* are we too close on the X axis? */
        if((tooCloseLeft || tooCloseRight) && get_maze_data(newFieldY, xOffset+newFieldX, 0) == MAZE_FIELD_WALL) {
            playerX &= ~(MAZE_CELL_SIZE-1); /* stay in the field, but remove the position within the field */
            playerX += tooCloseRight ? (MAZE_CELL_SIZE-PLAYER_WALL_DISTANCE) : PLAYER_WALL_DISTANCE; /* adjust to stay as close to the wall as possible */
            bumpOfWalls = FALSE;
        }
    }
    /* we didn't bounce off a wall, but we are too close to twos walls which point towards us (outside corner) */
    /* .X.. */
    /* XX.. e.g. here: with the player p moving towards the 'X' corner  */
    /* ..p. */
    if(bumpOfWalls && yOffset && xOffset && get_maze_data(yOffset+newFieldY, xOffset+newFieldX, 0) == MAZE_FIELD_WALL) {
        /* this is the distance between the player and the wall in X or Y direction */
        distanceY = tooCloseTop ? cellYfract : MAZE_CELL_SIZE-cellYfract;
        distanceX = tooCloseLeft ? cellXfract : MAZE_CELL_SIZE-cellXfract;
        if(distanceY >= distanceX) { /* slide along the axis, which is is closer; if equal slide along X  */
            playerY &= ~(MAZE_CELL_SIZE-1); /* stay in the field, but remove the position within the field */
            playerY += tooCloseBottom ? (MAZE_CELL_SIZE-PLAYER_WALL_DISTANCE) : PLAYER_WALL_DISTANCE; /* adjust to stay as close to the wall as possible */
        } else {
            playerX &= ~(MAZE_CELL_SIZE-1); /* stay in the field, but remove the position within the field */
            playerX += tooCloseRight ? (MAZE_CELL_SIZE-PLAYER_WALL_DISTANCE) : PLAYER_WALL_DISTANCE; /* adjust to stay as close to the wall as possible */
        }
    }

    /* Are drones in the game and the player (= drone) didn't move or turn and there is not motion pending and no target? */
    if(dronesActiveFlag
       && playerY == player_data[player].ply_y && playerX == player_data[player].ply_x && player_data[player].ply_dir == direction
       && player_data[player].dr_rotateCounter == 0
       && !player_data[player].dr_targetLocked) {
        player_data[player].dr_isInactive = TRUE; /* then we mark the drone inactive. This will trigger a new motion pattern. */
    }

    /* set the new position/direction of the player */
    player_data[player].ply_y = playerY;
    player_data[player].ply_x = playerX;
    player_data[player].ply_dir = direction;

    /* Emergency check: did the player acidentially end up in a closed box? If so, jump him to a new spot in the maze */
    /* This is actually a bug in the code, if this happens. It can however happen in the player collision detection above. */
    playerY >>= MAZE_FIELD_SHIFT; playerY |= 1;
    playerX >>= MAZE_FIELD_SHIFT; playerX |= 1;
    surroundingWalls = 0;
    if(get_maze_data(playerY-1, playerX, 0) == MAZE_FIELD_WALL)
        surroundingWalls++;
    if(get_maze_data(playerY, playerX-1, 0) == MAZE_FIELD_WALL)
        surroundingWalls++;
    if(get_maze_data(playerY+1, playerX, 0) == MAZE_FIELD_WALL)
        surroundingWalls++;
    if(get_maze_data(playerY, playerX+1, 0) == MAZE_FIELD_WALL)
        surroundingWalls++;
    if(surroundingWalls == 4) {
        if(!hunt_ply_pos(player))
           return FAILURE;
    }
    return SUCCESS;
}

/************************************************************
 *** move the shot for a specfic player, including hit-detection (walls and other players)
 ************************************************************/
void move_shoot(register int player) {
register int shotXField;
register int shotYField;
register int shotX;
register int shotY;
int saveShotXField;
int saveShotYField;
int distanceX;
int distanceY;
int objectID;
int zCoord;
int shotDirection;
int hasShot;
int yOffset;
int xOffset;

    hasShot = TRUE;
    /* position and direction of the shot */
    shotY = player_data[player].ply_shooty;
    shotX = player_data[player].ply_shootx;
    shotDirection = player_data[player].ply_shootr;
    /* current field of the shot, this is used to find walls around the shot */
    shotYField = shotY; shotYField >>= MAZE_FIELD_SHIFT; shotYField |= 1;
    shotXField = shotX; shotXField >>= MAZE_FIELD_SHIFT; shotXField |= 1;
    /* we calculate the movement 3x, which means a shot it 3x faster than the player can move */
    for(zCoord = 0; zCoord < 3; zCoord++) {
        /* move the shot in X/Y direction */
        shotY += xy_speed_table[shotDirection].deltaY;
        shotX += xy_speed_table[shotDirection].deltaX;
        saveShotYField = shotYField;
        saveShotXField = shotXField;
        /* calculate a (potentially) new field */
        shotYField = shotY; shotYField >>= MAZE_FIELD_SHIFT; shotYField |= 1;
        shotXField = shotX; shotXField >>= MAZE_FIELD_SHIFT; shotXField |= 1;
        /* is the field changed and the new field between them is a wall? */
        if((shotYField != saveShotYField || shotXField != saveShotXField) && get_maze_data((saveShotYField+shotYField)>>1, (saveShotXField+shotXField)>>1, 0) == MAZE_FIELD_WALL) {
            /* then the shot hit a wall, which ends it */
            player_data[player].ply_shoot = 0;
            break; /* we can also stop processing the shot */
        }

        /* check the area around the shot for targets */
        for(xOffset = -2; xOffset <= 2 && hasShot; xOffset += 2) {
            for(yOffset = -2; yOffset <= 2 && hasShot; yOffset += 2) {
                /* iterate over all objects in these field */
                objectID = get_maze_data(xOffset+shotYField, yOffset+shotXField, 0);
                while(objectID != MAZE_FIELD_EMPTY) { /* iterate the whole list */
                    if(objectID < PLAYER_MAX_COUNT) { /* a player? */
                        /* is the object is not the current player itself and alive?  */
                        if(player != objectID && player_data[objectID].ply_lives > 0) {
                            /* calculate the distance to the player */
                            distanceY = player_data[objectID].ply_y-shotY;
                            distanceY = abs(distanceY);
                            distanceX = player_data[objectID].ply_x-shotX;
                            distanceX = abs(distanceX);
                            /* within the radius of the player? This means the shot hit a target! */
                            if(distanceY <= PLAYER_RADIUS && distanceX <= PLAYER_RADIUS) {
                                if(player == own_number) /* only play the sound, if the player on this computer has been hit */
                                    Dosound((void*)sound_hit_ptr);

                                /* player got hit */
                                player_data[objectID].ply_hitflag = TRUE;
                                /* ...by this player */
                                player_data[objectID].ply_gunman = player;

                                /* Remove a live for that player, but in team mode, we will check if they are in the same team. */
                                /* In that case, if friendly fire is off, we only remove a live, if that wouldn't kill the player */
                                if(!team_flag || player_data[player].ply_team != player_data[objectID].ply_team || (!friendly_fire && player_data[objectID].ply_lives != 1)) {
                                    /* reset the refresh timer */
                                    player_data[objectID].ply_refresh = regen_time;
                                    /* remove a live */
                                    if(--player_data[objectID].ply_lives == 0) {
                                        /* player ended up being dead, so restart the refresh time with the revive time */
                                        player_data[objectID].ply_refresh = revive_time;
                                        /* the shooter will get a point (because team members can't kill each other, */
                                        /* this will not add points) */
                                        player_data[player].ply_score++;
                                        /* did the player/team win the game by reaching GAME_WIN_SCORE? */
                                        if(team_flag) {
                                            if(++team_scores[player_data[player].ply_team] == GAME_WIN_SCORE)
                                                we_dont_have_a_winner = NO; /* game winner found! */
                                        } else {
                                            if(player_data[player].ply_score == GAME_WIN_SCORE)
                                                we_dont_have_a_winner = NO; /* game winner found! */
                                        }
                                    }
                                    player_data[player].ply_looser = objectID;
                                }
                                /* remove the shot, after it hit a target */
                                player_data[player].ply_shoot = 0;
                                /* we are being hit, so the live status needs updating */
                                if(objectID == own_number)
                                    update_happiness_quotient_indicator();
                                /* no shot left. This terminates all loops (a return would also do the trick) */
                                hasShot = FALSE;
                            }
                        }
                        /* pick the next object */
                        objectID = player_data[objectID].ply_plist;
                    } else {
                        /* pick the next object */
                        objectID = player_data[objectID-PLAYER_MAX_COUNT].ply_slist;
                    }
                }
            }
        }
    }
    player_data[player].ply_shooty = shotY;
    player_data[player].ply_shootx = shotX;
}
