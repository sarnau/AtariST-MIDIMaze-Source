/************************************************************
 *** Specific code for the MIDIcam
 ************************************************************/
#include "globals.h"

/* mask away a potential negative sign, we only want positive numbers */
#define RANDOM_POSITIVE() (Random() & 0x7fff)

static int midicam_find_next_player(void);

short user_is_midicam;
short midicam_player_number;
short midicam_player_count;
short midicam_timer_counter;
short midicam_autoselect_player_flag;

/* default positions for the MIDIcam */
static const struct {
    int dY;
    int dX;
    int dir;
} midicam_start_setup[4] = { 
        {-1, 0, PLAYER_DIR_SOUTHEAST},
        { 0,-1, PLAYER_DIR_NORTHEAST},
        { 1, 0, PLAYER_DIR_NORTHWEST},
        { 0, 1, PLAYER_DIR_SOUTHWEST},
        };

/************************************************************
 *** The MIDIcam support several key presses to jump around
 ************************************************************/
void midicam_keyboard_shortcuts(void) {
int newDisplay2DMapFlag;

    if(Bconstat(CON)) {
        midicam_autoselect_player_flag = NO; /* do not automatically switch between players */
        newDisplay2DMapFlag = FALSE; /* map is off, if there is a key pressed */
        switch(((int)Bconin(CON))&0xff) {
        case '0': own_number = 9; break;
        case '1': own_number = 0; break;
        case '2': own_number = 1; break;
        case '3': own_number = 2; break;
        case '4': own_number = 3; break;
        case '5': own_number = 4; break;
        case '6': own_number = 5; break;
        case '7': own_number = 6; break;
        case '8': own_number = 7; break;
        case '9': own_number = 8; break;
        case ')': own_number = 9; break; /* Shift-9 */
        case '!': own_number = 10; break; /* Shift-1 */
        case '@': own_number = 11; break; /* Shift-2 */
        case '#': own_number = 12; break; /* Shift-3 */
        case '$': own_number = 13; break; /* Shift-4 */
        case '%': own_number = 14; break; /* Shift-5 */
        case '^': own_number = 15; break; /* Shift-6 */
        case ' ': newDisplay2DMapFlag = !display_2d_map_flag; break;
        default: /* any other key */
            midicam_autoselect_player_flag = YES; /* automatically switch between players  */
            midicam_timer_counter = 0; /* reset the player switch timer  */
            newDisplay2DMapFlag = display_2d_map_flag; /* do not touch the 2D map flag */
            break;
        }
        /* remove all pending key presses */
        while(Bconstat(CON))
            Bconin(CON);

        display_2d_map_flag = newDisplay2DMapFlag; /* update the 2D map display */
    }
    /* automatically find a player to follow? */
    if(midicam_autoselect_player_flag)
        own_number = midicam_find_next_player(); /* pick one */

    /* if the player changes, show the name and update the smily status */
    if(own_number != midicam_player_number) {
        midicam_print_current_player();
        update_happiness_quotient_indicator();
    }
}

/************************************************************
 *** Automatically pick a new player to follow with the MIDIcam after a certain time
 ************************************************************/
int midicam_find_next_player(void) {
int anyPlayerAliveFlag;
int i;

    /* If the player died and the 2D map is not displayed, set the switch timer all the way down  */
    if(player_data[own_number].ply_lives <= 0 && !display_2d_map_flag && midicam_timer_counter > 5)
        midicam_timer_counter = 5;

    /* decrement the switch timer, just return if it is not expired  */
    if(--midicam_timer_counter > 0)
        return own_number;

    /* reload it */
    midicam_timer_counter = 50;

    /* show the map, if not already displayed */
    if(!display_2d_map_flag) {
        display_2d_map_flag = YES;
        return own_number;
    }
    display_2d_map_flag = NO;

    /* check if there is a single player/drone that is alive */
    anyPlayerAliveFlag = NO;
    for(i = 0; i < playerAndDroneCount; i++) {
        anyPlayerAliveFlag |= player_data[i].ply_lives > 0;
    }
    if(anyPlayerAliveFlag && (Random() & 1)) { /* alive and 50:50 chance hit? */
        /* then pick the first non-dead random player */
        while(1) {
            i = RANDOM_POSITIVE() % playerAndDroneCount;
            if(i >= 0 && i < playerAndDroneCount && player_data[i].ply_lives > 0)
                return i;
        }
    }
    /* nobody alive or 50:50 chance missed => pick any random player/drone (dead or alive) */
    if(playerAndDroneCount == PLAYER_MAX_COUNT)
        return RANDOM_POSITIVE() % playerAndDroneCount;
    else
        return RANDOM_POSITIVE() % (PLAYER_MAX_COUNT-playerAndDroneCount) + playerAndDroneCount;
}

/************************************************************
 *** Find a good start position for all the MIDIcam positions
 ************************************************************/
void position_midicam_users(void) {
int randDir;
int notdone;
int wallCount;
int fieldX;
int fieldY;
int midicam;

    /* The MIDIcam positions are players behind the last player or drone. They a invisble in the game. */
    for(midicam = playerAndDroneCount; midicam < PLAYER_MAX_COUNT; midicam++) {
        do {
            /* get a random field (do not use _rnd(), because the MIDIcam can not be synchronized in the network, this would screw up the random generator for all players). This field is always at an odd position. */
            fieldY = (RANDOM_POSITIVE() % maze_size) | 1;
            fieldX = (RANDOM_POSITIVE() % maze_size) | 1;

            /* check for walls around the proposed starting position */
            wallCount = 0;
            if(get_maze_data(fieldY-1, fieldX, 0) == MAZE_FIELD_WALL)
                wallCount++;
            if(get_maze_data(fieldY, fieldX-1, 0) == MAZE_FIELD_WALL)
                wallCount++;
            if(get_maze_data(fieldY+1, fieldX, 0) == MAZE_FIELD_WALL)
                wallCount++;
            if(get_maze_data(fieldY, fieldX+1, 0) == MAZE_FIELD_WALL)
                wallCount++;
        } while(wallCount == 0 || wallCount == 4); /* do not position the MIDIcam into an open or a closed space  */

        /* a loop to find a nice starting viewing direction for the MIDIcam */
        notdone = TRUE;
        while(notdone) {
            randDir = RANDOM_POSITIVE() & 3; /* pick a random direction for the camera */
            /* check the field into the random direction. If it is a wall, try again. This avoids that the MIDIcam */
            /* initially faces a wall, which is not nice */
            /* The endless loop requieres that all 4 possible directions will be generated at some point */
            /* With the random generator of an Atari ST this is very quickly the case. Because it is just */
            /* the starting direction, it might have been better to just generate one random direction */
            /* and then increment in case of a wall to test other directions. That way it would be guranteed */
            /* to be done in 4 steps.  */
            if(get_maze_data(midicam_start_setup[randDir].dY+fieldY, midicam_start_setup[randDir].dX+fieldX, 0) == MAZE_FIELD_WALL) {
                /* move the camera closer to the wall for a better view. This depends on the viewing direction */
                player_data[midicam].ply_y = (fieldY << MAZE_FIELD_SHIFT) + midicam_start_setup[randDir].dY*120;
                player_data[midicam].ply_x = (fieldX << MAZE_FIELD_SHIFT) + midicam_start_setup[randDir].dX*120;
                player_data[midicam].ply_dir = midicam_start_setup[randDir].dir;
                player_data[midicam].ply_lives = PLAYER_MAX_LIVES; /* to avoid having a dead MIDIcam (it can't be killed anyway) */
                notdone = FALSE;
            }
        }
    }
}

/************************************************************
 *** Print the currently monitored player into the pop char area
 *** for the MIDIcam
 ************************************************************/
void midicam_print_current_player(void) {
int i;

    for(i = 0; i < 2; i++) {
        BCON_SETCURSOR(10, screen_rez ? 48 : 24);
        BCON_SETFCOLOR(screen_rez ? COLOR_SILVER_INDEX : COLOR_WHITE_INDEX);
        BCON_SETBCOLOR(COLOR_BLACK_INDEX);
        /* erase the area for the player name with 15 spaces (2 for the number, 1 for ":', plus 10 for the name plus 2 spare ones) */
        bconout_string("               ");
        BCON_SETCURSOR(10, screen_rez ? 48 : 24);
        bconout_dec_number(own_number+1); Bconout(CON, ':'); print_playername(own_number);
        switch_logbase();
    }
}
