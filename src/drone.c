/************************************************************
 *** All code to manage the drones
 *** The vast majority of it is for the ninja.
 ************************************************************/
#include "globals.h"

static int drone_move(int player);
static void drone_check_directions(int player,int *canNorthPtr,int *canSouthPtr,int *canEastPtr,int *canWestPtr,int useAltCoord,int altYField,int altXField);
static void drone_sub_findMoveToTarget(int player);
static void drone_sub_ninja(int player);
static void drone_generate_joystickdata(int player);
static int drone_aim2target(int player);
static int drone_delta_into_direction(int deltaY,int deltaX);
static int drone_isTargetIsVisibleNorth(int player,int fieldY,int fieldX);
static int drone_isTargetIsVisibleSouth(int player,int fieldY,int fieldX);
static int drone_isTargetIsVisibleEast(int player,int fieldY,int fieldX);
static int drone_isTargetIsVisibleWest(int player,int fieldY,int fieldX);
static void drone_set_position(int player,int viewCompassDirChar);
static void drone_sub_standard(int player);

static int drone_sub_ninja_plan(int player,int viewCompassDirChar);
static int drone_sub_ninja_north(int player,int viewCompassDirChar);
static int drone_sub_ninja_south(int player,int viewCompassDirChar);
static int drone_sub_ninja_east(int player,int viewCompassDirChar);
static int drone_sub_ninja_west(int player,int viewCompassDirChar);

static void drone_move_down(int player);
static void drone_move_upright(int player);
static void drone_move_upleft(int player);
static void drone_move_up(int player);
static void drone_turn_around(int player);


static int drone_needs2GoSouth;
static int drone_needs2GoWest;
static short drone_angle_table[33];
static int drone_can_east;
static int drone_can_north;
static int drone_can_south;
static int drone_needs2GoEast;
static int drone_can_west;
static int drone_needs2GoNorth;

/************************************************************
 *** calculate a table that allows to transform an X/Y coordinate into an angle
 ************************************************************/
void calc_drone_angle_table(void) {
int x;
int y;
int i;

    /* The table is a slight sine curve of 33 values going from 0..32 */
    /* 0, 2, 3, 4, 6, 7, 8, 9, 10, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 27, 28, 29, 30, 30, 31, 31, 32 */

    /* empty the table */
    for(i = 0; i <= 32; i++)
        drone_angle_table[i] = -1;

    /* create the reverse lookup table */
    for(i = 32; i >= 0; i--) {
        y = 1000;
        x = 0;
        rotate2d(&y, &x, i);
        drone_angle_table[muls_divs(32, x, y)] = i;
    }
    /* fill in the gapes with previous values */
    for(i = 0; i <= 32; i++) {
        if(drone_angle_table[i] == -1) {
            drone_angle_table[i] = drone_angle_table[i-1];
        }
    }
}

/************************************************************
 *** Drone setup before a game. In this round a current and a permanent target
 *** is assigned. This is always one specific target player!
 ************************************************************/
void drone_setup(int humanPlayers) {
int team3Attackable;
int team2Attackable;
int team1Attackable;
int team0Attackable;
int teamCount;
int team3Index;
int team2Index;
int team1Index;
int team3[18];
int team2[18];
int team1[18];
int team0[18];
int team3HasMembers;
int team2HasMembers;
int team1HasMembers;
int team0HasMembers;
int playerIndex;
int team0Index;
int allPlayerCount;
int currentHumanSoloPlayer;
int humanSoloPlayerList[18];

    /* reset all iterator variables */
    currentHumanSoloPlayer = 0;
    allPlayerCount = 0;
    team0Index = 0; /* set twice to 0... */
    team0HasMembers = team1HasMembers = team2HasMembers = team3HasMembers = FALSE;
    team0Index = team1Index = team2Index = team3Index = 0;
    teamCount = 0;
    team0Attackable = team1Attackable = team2Attackable = team3Attackable = FALSE;

    /* all drones and all human players */
    allPlayerCount = active_drones_by_type[0]+active_drones_by_type[1]+active_drones_by_type[2]+humanPlayers;

    if(team_flag) {
        /* Team mode */
        /* build lists for all teams members including drones */
        for(playerIndex = 0; playerIndex < allPlayerCount; playerIndex++) {
            switch(player_data[playerIndex].ply_team) {
            case 0: team0[team0Index++] = playerIndex; break;
            case 1: team1[team1Index++] = playerIndex; break;
            case 2: team2[team2Index++] = playerIndex; break;
            case 3: team3[team3Index++] = playerIndex; break;
            }
        }
        /* terminate the lists */
        team0[team0Index] = -1;
        team1[team1Index] = -1;
        team2[team2Index] = -1;
        team3[team3Index] = -1;

        /* Check all teams for members, if there are none, then the team is ignored */
        /* As a start all teams with members are attackable */
        if(team0[0] != -1) {
            teamCount++;
            team0HasMembers = team0Attackable = TRUE;
        }
        if(team1[0] != -1) {
            teamCount++;
            team1HasMembers = team1Attackable = TRUE;
        }
        if(team2[0] != -1) {
            teamCount++;
            team2HasMembers = team2Attackable = TRUE;
        }
        if(team3[0] != -1) {
            teamCount++;
            team3HasMembers = team3Attackable = TRUE;
        }

        /* is there only one team anyway? */
        if(teamCount == 1) {
            /* There are no targets for anyone; this is not really a game but just moving around  */
            for(playerIndex = 0; playerIndex < allPlayerCount; playerIndex++)
                player_data[playerIndex].dr_currentTarget = -1;
        } else {
            /* Iterate over all player teams and pick an individual player from a different team as a target. */
            /* This might not be nice, because the distribution is not fair or even random. It is surprising */
            /* that not the whole team is a target, but always a specific player. */
            team0Index = team1Index = team2Index = team3Index = 0;
            for(playerIndex = 0; playerIndex < allPlayerCount; playerIndex++) {
                switch(player_data[playerIndex].ply_team) {
                case 0: /* Team #0 (Team #1-#3 are quasi-identical) */
                    /* only standard or ninja drones have targets anyway */
                    if(player_data[playerIndex].dr_type == DRONE_NINJA || player_data[playerIndex].dr_type == DRONE_STANDARD) {
                        if(team1HasMembers && team1Attackable) {
                            if(team1[team1Index] == -1) team1Index = 0; /* iterate through the list, at the end: restart at the beginning */
                            /* pick a player as a target */
                            player_data[playerIndex].dr_currentTarget = team1[team1Index++];
                            /* if a different team has members, on the next round switch to one of these as a target, by making this one non-attackable */
                            if(team2HasMembers || team3HasMembers) team1Attackable = FALSE;
                            if(team2HasMembers) team2Attackable = TRUE;
                            if(team3HasMembers) team3Attackable = TRUE;
                        } else if(team2HasMembers && team2Attackable) {
                            if(team2[team2Index] == -1) team2Index = 0;
                            player_data[playerIndex].dr_currentTarget = team2[team2Index++];
                            if(team1HasMembers || team3HasMembers) team2Attackable = FALSE;
                            if(team3HasMembers) team3Attackable = TRUE;
                            else if(team1HasMembers) team1Attackable = TRUE;
                        } else if(team3HasMembers && team3Attackable) {
                            if(team3[team3Index] == -1) team3Index = 0;
                            player_data[playerIndex].dr_currentTarget = team3[team3Index++];
                            if(team1HasMembers || team2HasMembers) team3Attackable = FALSE;
                            if(team1HasMembers) team1Attackable = TRUE;
                            if(team2HasMembers) team2Attackable = TRUE;
                        }
                        player_data[playerIndex].dr_permanentTarget = player_data[playerIndex].dr_currentTarget;
                    }
                    break;
                case 1: /* Team #1 */
                    if(player_data[playerIndex].dr_type == DRONE_NINJA || player_data[playerIndex].dr_type == DRONE_STANDARD) {
                        if(team0HasMembers && team0Attackable) {
                            if(team0[team0Index] == -1) team0Index = 0;
                            player_data[playerIndex].dr_currentTarget = team0[team0Index++];
                            if(team3HasMembers || team2HasMembers) team0Attackable = FALSE;
                            if(team3HasMembers) team3Attackable = TRUE;
                            if(team2HasMembers) team2Attackable = TRUE;
                        } else if(team2HasMembers && team2Attackable) {
                            if(team2[team2Index] == -1) team2Index = 0;
                            player_data[playerIndex].dr_currentTarget = team2[team2Index++];
                            if(team0HasMembers || team3HasMembers) team2Attackable = FALSE;
                            if(team3HasMembers) team3Attackable = TRUE;
                            else if(team0HasMembers) team0Attackable = TRUE;
                        } else if(team3HasMembers && team3Attackable) {
                            if(team3[team3Index] == -1) team3Index = 0;
                            player_data[playerIndex].dr_currentTarget = team3[team3Index++];
                            if(team0HasMembers || team2HasMembers) team3Attackable = FALSE;
                            if(team0HasMembers) team0Attackable = TRUE;
                            if(team2HasMembers) team2Attackable = TRUE;
                        }
                        player_data[playerIndex].dr_permanentTarget = player_data[playerIndex].dr_currentTarget;
                    }
                    break;
                case 2: /* Team #2 */
                    if(player_data[playerIndex].dr_type == DRONE_NINJA || player_data[playerIndex].dr_type == DRONE_STANDARD) {
                        if(team1HasMembers && team1Attackable) {
                            if(team1[team1Index] == -1) team1Index = 0;
                            player_data[playerIndex].dr_currentTarget = team1[team1Index++];
                            if(team0HasMembers || team3HasMembers) team1Attackable = FALSE;
                            if(team0HasMembers) team0Attackable = TRUE;
                            if(team3HasMembers) team3Attackable = TRUE;
                        } else if(team0HasMembers && team0Attackable) {
                            if(team0[team0Index] == -1) team0Index = 0;
                            player_data[playerIndex].dr_currentTarget = team0[team0Index++];
                            if(team1HasMembers || team3HasMembers) team0Attackable = FALSE;
                            if(team3HasMembers) team3Attackable = TRUE;
                            else if(team1HasMembers) team1Attackable = TRUE;
                        } else if(team3HasMembers && team3Attackable) {
                            if(team3[team3Index] == -1) team3Index = 0;
                            player_data[playerIndex].dr_currentTarget = team3[team3Index++];
                            if(team0HasMembers || team1HasMembers) team3Attackable = FALSE;
                            if(team0HasMembers) team0Attackable = TRUE;
                            if(team1HasMembers) team1Attackable = TRUE;
                        }
                        player_data[playerIndex].dr_permanentTarget = player_data[playerIndex].dr_currentTarget;
                    }
                    break;
                case 3: /* Team #3 */
                    if(player_data[playerIndex].dr_type == DRONE_NINJA || player_data[playerIndex].dr_type == DRONE_STANDARD) {
                        if(team1HasMembers && team1Attackable) {
                            if(team1[team1Index] == -1) team1Index = 0;
                            player_data[playerIndex].dr_currentTarget = team1[team1Index++];
                            if(team2HasMembers || team0HasMembers) team1Attackable = FALSE;
                            if(team2HasMembers) team2Attackable = TRUE;
                            if(team0HasMembers) team0Attackable = TRUE;
                        } else if(team2HasMembers && team2Attackable) {
                            if(team2[team2Index] == -1) team2Index = 0;
                            player_data[playerIndex].dr_currentTarget = team2[team2Index++];
                            if(team0HasMembers || team1HasMembers) team2Attackable = FALSE;
                            if(team0HasMembers) team0Attackable = TRUE;
                            else if(team1HasMembers) team1Attackable = TRUE;
                        } else if(team0HasMembers && team0Attackable) {
                            if(team0[team0Index] == -1) team0Index = 0;
                            player_data[playerIndex].dr_currentTarget = team0[team0Index++];
                            if(team1HasMembers || team2HasMembers) team0Attackable = FALSE;
                            if(team1HasMembers) team1Attackable = TRUE;
                            if(team2HasMembers) team2Attackable = TRUE;
                        }
                        player_data[playerIndex].dr_permanentTarget = player_data[playerIndex].dr_currentTarget;
                    }
                    break;
                }
            }
        }

    } else {
        /* individual mode with no teams. In this mode drones _only_ attack humans, never other drones! */
        
        /* create a -1 terminated lists with all human players in it */
        for(playerIndex = 0; playerIndex < humanPlayers; playerIndex++)
            humanSoloPlayerList[playerIndex] = playerIndex;
        humanSoloPlayerList[playerIndex] = -1;

        /* go through all drone players */
        for(playerIndex = humanPlayers; playerIndex < allPlayerCount; playerIndex++) {
            /* only standard and ninja drones actually have targets (drones do not shoot) */
            if(player_data[playerIndex].dr_type == DRONE_NINJA || player_data[playerIndex].dr_type == DRONE_STANDARD) {
                /* add all players as potential human enemies to the drones */
                for(team0Index = 0; humanSoloPlayerList[team0Index] != -1; team0Index++)
                    player_data[playerIndex].dr_humanEnemies[team0Index] = humanSoloPlayerList[team0Index];
            }
            /* set the end of the enemy list */
            player_data[playerIndex].dr_humanEnemies[team0Index] = -1;

            /* Share the players as targets to all drones. This is done by picking one and then moving to the */
            /* next player as the next target, etc. This might not be entirely fair, e.g. if we have one standard drone */
            /* and one ninja with 2 players. In this case player #0 will be giving to the standard one, while player #1 */
            /* will be the target of the ninja. */
            if(humanSoloPlayerList[currentHumanSoloPlayer] == -1) {
                /* end of the list reached, start again at the beginning of the list */
                player_data[playerIndex].dr_currentTarget = player_data[playerIndex].dr_humanEnemies[currentHumanSoloPlayer = 0];
                player_data[playerIndex].dr_permanentTarget = player_data[playerIndex].dr_currentTarget;
            } else {
                /* pick a human enemy as a current and also a permanent target. Move to the next human player */
                player_data[playerIndex].dr_currentTarget = player_data[playerIndex].dr_humanEnemies[currentHumanSoloPlayer++];
                player_data[playerIndex].dr_permanentTarget = player_data[playerIndex].dr_currentTarget;
            }
        }
    }
}

/************************************************************
 *** void drone_action(int player)
 ************************************************************/
void drone_action(int player) {
int target_player;
int i;

    switch(player_data[player].dr_type) {
    case DRONE_NINJA:
        /* the player is dead, reset all temporary variables */
        if(player_data[player].ply_lives <= 0) {
            player_data[player].dr_dir[0] = 0;
            player_data[player].dr_targetLocked = FALSE; /* no active target to fire at */
            player_data[player].dr_fieldIndex = 0;
            player_data[player].dr_field[0].y = 0;
            player_data[player].dr_upRotationCounter = 0;
            player_data[player].dr_rotateCounter = 0;
            break;
        }

        /* Did the drone get hit by a human? It will not attack a different drone, even in different teams! */
        if(player_data[player].ply_hitflag && player_data[player].ply_gunman != player_data[player].dr_currentTarget && player_data[player_data[player].ply_gunman].dr_type != DRONE_NINJA && player_data[player_data[player].ply_gunman].dr_type != DRONE_STANDARD) {
            if(player_data[player_data[player].ply_gunman].ply_team != player_data[player].ply_team && team_flag) {
                /* Player in a different team => attack */
                player_data[player].dr_currentTarget = player_data[player].ply_gunman;
                player_data[player].dr_dir[0] = 0;
                player_data[player].dr_targetLocked = FALSE; /* no active target to fire at */
                player_data[player].dr_fieldIndex = 0;
                player_data[player].dr_field[0].y = 0;
                player_data[player].dr_upRotationCounter = 0;
                player_data[player].dr_rotateCounter = 0;
            } else if(!team_flag) {
                /* no teams active => attack */
                player_data[player].dr_currentTarget = player_data[player].ply_gunman;
                player_data[player].dr_dir[0] = 0;
                player_data[player].dr_targetLocked = FALSE; /* no active target to fire at */
                player_data[player].dr_fieldIndex = 0;
                player_data[player].dr_field[0].y = 0;
                player_data[player].dr_upRotationCounter = 0;
                player_data[player].dr_rotateCounter = 0;
            }
            /* only other case: same team as player => ignore the hit */
        }

        /* Drone doesn't have a target in team mode? Then just move around */
        target_player = player_data[player].dr_currentTarget;
        if(target_player < 0 && team_flag) {
            drone_check_directions(player, &drone_can_north, &drone_can_south, &drone_can_east, &drone_can_west, 0, 0, 0);
            drone_generate_joystickdata(player);
            break;
        }

        /* target exists and is dead in single-player mode */
        if(player_data[target_player].ply_lives <= 0 && !team_flag) {
            /* reset all variables */
            player_data[player].dr_dir[0] = 0;
            player_data[player].dr_fieldIndex = 0;
            player_data[player].dr_field[0].y = 0;
            player_data[player].dr_upRotationCounter = 0;
            player_data[player].dr_rotateCounter = 0;

            /* do we have a specific target? */
            if(player_data[player].dr_targetLocked) {
                /* try moving towards it */
                drone_sub_findMoveToTarget(player);
            }
            /* do we have a permanent target, that is alive? */
            if(player_data[player_data[player].dr_permanentTarget].ply_lives > 0) {
                /* found a new current target! */
                player_data[player].dr_currentTarget = player_data[player].dr_permanentTarget;
                target_player = player_data[player].dr_currentTarget;
            } else { /* no, look  */
                /* find a new current target */
                for(i = 0; player_data[player].dr_humanEnemies[i] != -1; i++) {
                    /* the target has to be alive and either human or a dummy (but the dr_humanEnemies only contains humans) */
                    if(player_data[i].ply_lives > 0 && player_data[i].dr_type != DRONE_NINJA && player_data[i].dr_type != DRONE_STANDARD) {
                        /* found a new current target! */
                        player_data[player].dr_currentTarget = player_data[player].dr_humanEnemies[i];
                        target_player = player_data[player].dr_currentTarget;
                        break;
                    }
                }
            }
        } else if(player_data[target_player].ply_lives <= 0 && team_flag) { /* target exists and is dead in team mode */
            /* reset all variables */
            player_data[player].dr_dir[0] = 0;
            player_data[player].dr_fieldIndex = 0;
            player_data[player].dr_field[0].y = 0;
            player_data[player].dr_targetLocked = FALSE; /* no active target to fire at */
            player_data[player].dr_fireDirection = -1; /* no fire direction (only set, never read) */
            player_data[player].dr_upRotationCounter = 0;
            player_data[player].dr_rotateCounter = 0;
            /* just move around */
            drone_check_directions(player, &drone_can_north, &drone_can_south, &drone_can_east, &drone_can_west, 0, 0, 0);
            drone_generate_joystickdata(player);
            break;
        }
        
        if(player_data[target_player].ply_lives <= 0) { /*  target is dead? */
            /* reset all variables */
            player_data[player].dr_dir[0] = 0;
            player_data[player].dr_fieldIndex = 0;
            player_data[player].dr_field[0].y = 0;
            player_data[player].dr_targetLocked = FALSE; /* no active target to fire at (only set, never read) */
            player_data[player].dr_upRotationCounter = 0;
            player_data[player].dr_rotateCounter = 0;
            /* just move around */
            drone_check_directions(player, &drone_can_north, &drone_can_south, &drone_can_east, &drone_can_west, 0, 0, 0);
            drone_generate_joystickdata(player);
            break;
        }

        if(drone_move(player)) return;
        drone_sub_ninja(player);
        drone_generate_joystickdata(player);
        break;

    case DRONE_TARGET:
        /* the player is dead, reset all temporary variables */
        if(player_data[player].ply_lives <= 0) {
            /* reset all variables */
            player_data[player].dr_dir[0] = 0; /* not used by the target drone anyway */
            player_data[player].dr_targetLocked = FALSE; /* no active target to fire at */
            player_data[player].dr_upRotationCounter = 0;
            player_data[player].dr_rotateCounter = 0;
        }
 
        if(drone_move(player)) return;
        /* just move around */
        drone_check_directions(player, &drone_can_north, &drone_can_south, &drone_can_east, &drone_can_west, 0, 0, 0);
        drone_generate_joystickdata(player);
        break;

    case DRONE_STANDARD:
        /* the player is dead, reset all temporary variables */
        if(player_data[player].ply_lives <= 0) {
            /* reset all variables */
            player_data[player].dr_dir[0] = 0; /* not used by the standard drone anyway */
            player_data[player].dr_targetLocked = FALSE; /* no active target to fire at */
            player_data[player].dr_fieldIndex = 0; /* not used by the standard drone anyway */
            player_data[player].dr_field[0].y = 0; /* not used by the standard drone anyway */
            player_data[player].dr_upRotationCounter = 0;
            player_data[player].dr_rotateCounter = 0;
            break;
        }
 
        /* Did the drone get hit by a human? It will not attack a different drone, even in different teams! */
        if(player_data[player].ply_hitflag && player_data[player].ply_gunman != player_data[player].dr_currentTarget && player_data[player_data[player].ply_gunman].dr_type != DRONE_NINJA && player_data[player_data[player].ply_gunman].dr_type != DRONE_STANDARD) {
            if(player_data[player_data[player].ply_gunman].ply_team != player_data[player].ply_team && team_flag) {
                /* Player in a different team => attack */
                player_data[player].dr_currentTarget = player_data[player].ply_gunman;
                player_data[player].dr_dir[0] = 0; /* not used by the standard drone anyway */
                player_data[player].dr_targetLocked = FALSE; /* no active target to fire at */
                player_data[player].dr_fieldIndex = 0; /* not used by the standard drone anyway */
                player_data[player].dr_field[0].y = 0; /* not used by the standard drone anyway */
                player_data[player].dr_upRotationCounter = 0;
                player_data[player].dr_rotateCounter = 0;
            } else if(!team_flag) {
                /* no teams active => attack */
                player_data[player].dr_currentTarget = player_data[player].ply_gunman;
                player_data[player].dr_dir[0] = 0; /* not used by the standard drone anyway */
                player_data[player].dr_targetLocked = FALSE; /* no active target to fire at */
                player_data[player].dr_fieldIndex = 0; /* not used by the standard drone anyway */
                player_data[player].dr_field[0].y = 0; /* not used by the standard drone anyway */
                player_data[player].dr_upRotationCounter = 0;
                player_data[player].dr_rotateCounter = 0;
            }
        }

        /* Drone doesn't have a target in team mode? Then just move around */
        target_player = player_data[player].dr_currentTarget;
        /* BUG: test order is probably wrong, if target_player == -1. Probably harmless on an Atari ST, but still a bug */
        if(player_data[target_player].ply_lives <= 0 || (team_flag && target_player < 0)) { /* if in team mode and no target => just move */
            /* reset all variables */
            player_data[player].dr_dir[0] = 0; /* not used by the standard drone anyway */
            player_data[player].dr_targetLocked = FALSE; /* no active target to fire at */
            player_data[player].dr_upRotationCounter = 0;
            player_data[player].dr_rotateCounter = 0;
            /* just move around */
            drone_check_directions(player, &drone_can_north, &drone_can_south, &drone_can_east, &drone_can_west, 0, 0, 0);
            drone_generate_joystickdata(player);
            break;
        }

        if(drone_move(player)) return;
        drone_check_directions(player, &drone_can_north, &drone_can_south, &drone_can_east, &drone_can_west, 0, 0, 0);
        drone_sub_standard(player);
        drone_generate_joystickdata(player);
        break;
    }
}

/************************************************************
 *** Basic move in the maze without an specific target. Used by all drone types.
 *** Returns YES, if a movement was generated.
 ************************************************************/
int drone_move(int player) {
int joystickMaskAlwaysZero = 0x00; /* could have been a forward motion or the fire button. But it is not used. */

    /* If there is a target, do not do anything here! */
    if(player_data[player].dr_targetLocked)
        return NO; /* no movement generated (handle the shot instead) */

    /* An inactive nunja drone with no target and no plan is forced repositioned */
    /* My guess is that this is done to avoid that a nunja get's stuck somewhere */
    if(player_data[player].dr_isInactive && !player_data[player].dr_targetLocked && player_data[player].dr_type == DRONE_NINJA && !player_data[player].dr_dir[0]) {
        /* reset all variables */
        player_data[player].dr_dir[0] = 0;
        player_data[player].dr_fieldIndex = 0;
        player_data[player].dr_isInactive = FALSE;
        player_data[player].dr_upRotationCounter = 0;
        player_data[player].dr_rotateCounter = 0;
        drone_check_directions(player, &drone_can_north, &drone_can_south, &drone_can_east, &drone_can_west, 0, 0, 0);
        if(drone_can_north)
            drone_set_position(player, 'n');
        else if(drone_can_south)
            drone_set_position(player, 's');
        else if(drone_can_east)
            drone_set_position(player, 'e');
        else if(drone_can_west)
            drone_set_position(player, 'w');
        return NO;
    }

    /* a rotation is pending. Do not do anything else, just finish the rotation */
    if(player_data[player].dr_rotateCounter > 0) {
        player_data[player].dr_rotateCounter--;
        player_joy_table[player] = player_data[player].dr_joystick;
        return YES;
    }
    /* not sure, why a negative value is used as well, it makes no difference  */
    if(player_data[player].dr_rotateCounter < 0) {
        player_data[player].dr_rotateCounter++;
        player_joy_table[player] = player_data[player].dr_joystick;
        return YES;
    }

    /* drone is inactive and has no rotation pending (this is always the case, because dr_rotateTimerA != 0 will return before this line, see above) */
    if(player_data[player].dr_isInactive && player_data[player].dr_rotateCounter == 0) {
        /* now generate some semi-random motion */
        /* player is facing north or east */
        if(player_data[player].ply_dir == PLAYER_DIR_NORTH || player_data[player].ply_dir == PLAYER_DIR_EAST) {
            player_data[player].dr_isInactive = FALSE;
            if(_rnd(256)&1) { /* 50% chance to do a 90 degree right turn */
                player_data[player].dr_rotateCounter = (256/PLAYER_MOTION_ROTATE)/4-1; /* /4 = a quarter of a full rotation = 90 (the drone will be aligned at the end via drone_generate_joystickdata()) */
                player_joy_table[player] = player_data[player].dr_joystick = joystickMaskAlwaysZero|JOYSTICK_RIGHT;
            } else if(_rnd(256)&1) { /* 25% chance to do a 90 degree left turn */
                player_data[player].dr_rotateCounter = -((256/PLAYER_MOTION_ROTATE)/4-1); /* /4 = a quarter of a full rotation = 90 (the drone will be aligned at the end via drone_generate_joystickdata()) */
                player_joy_table[player] = player_data[player].dr_joystick = joystickMaskAlwaysZero|JOYSTICK_LEFT;
            } else { /* 25% chance to do a 180 degree turn */
                player_data[player].dr_rotateCounter = (256/PLAYER_MOTION_ROTATE)/2-2; /* /2 = half of a full rotation = 180 degree turn (the drone will be aligned at the end via drone_generate_joystickdata()) */
                player_joy_table[player] = player_data[player].dr_joystick = JOYSTICK_LEFT;
            }

        /* player is facing south or west */
        } else if(player_data[player].ply_dir == PLAYER_DIR_SOUTH || player_data[player].ply_dir == PLAYER_DIR_WEST) {
            player_data[player].dr_isInactive = FALSE;
            if(_rnd(256)&1) { /* 50% chance to do a 90 degree left turn */
                player_data[player].dr_rotateCounter = -((256/PLAYER_MOTION_ROTATE)/4-1); /* /4 = a quarter of a full rotation = 90 degree turn (the drone will be aligned at the end via drone_generate_joystickdata()) */
                player_joy_table[player] = player_data[player].dr_joystick = joystickMaskAlwaysZero|JOYSTICK_LEFT;
            } else if(_rnd(256)&1) { /* 25% chance to do a 90 degree right turn */
                player_data[player].dr_rotateCounter = (256/PLAYER_MOTION_ROTATE)/4-1; /* /4 = a quarter of a full rotation = 90 degree turn (the drone will be aligned at the end via drone_generate_joystickdata()) */
                player_joy_table[player] = player_data[player].dr_joystick = joystickMaskAlwaysZero|JOYSTICK_RIGHT;
            } else { /* 25% chance to do a 180 degree turn */
                player_data[player].dr_rotateCounter = (256/PLAYER_MOTION_ROTATE)/2-2; /* /2 = half of a full rotation = 180 degree turn (the drone will be aligned at the end via drone_generate_joystickdata()) */
                player_joy_table[player] = player_data[player].dr_joystick = JOYSTICK_RIGHT;
            }

        } else {
            /* player is at an angle != 90 degrees */
            drone_sub_findMoveToTarget(player);
            player_data[player].dr_isInactive = FALSE;
            return NO;
        }
        return YES;
    }

    /* upleft or upright turn ongoing? */
    /* The drone only needs to be turned by 45 degrees, */
    /* then drone_generate_joystickdata() will take over */
    /* and continue the rotation till the drone is 90 degree */
    /* aligned at the end. */
    if(player_data[player].dr_upRotationCounter) {
        player_joy_table[player] = player_data[player].dr_joystick;
        player_data[player].dr_upRotationCounter--;
        return YES;
    }

    return NO;
}

/************************************************************
 *** Check if fields around the player/coordinates are open
 ************************************************************/
void drone_check_directions(int player,int *canNorthPtr,int *canSouthPtr,int *canEastPtr,int *canWestPtr,int useAltCoord,int altYField,int altXField) {
int xField;
int yField;
int x;
int y;

    if(!useAltCoord) {
        y = player_data[player].ply_y;
        x = player_data[player].ply_x;
        yField = (y >> MAZE_FIELD_SHIFT)|1;
        xField = (x >> MAZE_FIELD_SHIFT)|1;
    } else {
        yField = altYField;
        xField = altXField;
    }
    *canNorthPtr = *canSouthPtr = *canEastPtr = *canWestPtr = FALSE;
    if(get_maze_data(yField-1, xField, 0) == MAZE_FIELD_EMPTY) *canNorthPtr = TRUE;
    if(get_maze_data(yField+1, xField, 0) == MAZE_FIELD_EMPTY) *canSouthPtr = TRUE;
    if(get_maze_data(yField, xField-1, 0) == MAZE_FIELD_EMPTY) *canWestPtr = TRUE;
    if(get_maze_data(yField, xField+1, 0) == MAZE_FIELD_EMPTY) *canEastPtr = TRUE;
}

/************************************************************
 *** Try to move towards the target. This is an unfair algorithm,
 *** because it relies on the knowledge where the target is at
 *** all times. In return: it has no idea about how the maze
 *** looks like.
 ************************************************************/
void drone_sub_findMoveToTarget(int player) {
int target_player;
int targetX;
int targetY;
int playerX;
int playerY;
int targetDistanceX;
int targetDistanceY;

    drone_needs2GoNorth = drone_needs2GoSouth = drone_needs2GoEast = drone_needs2GoWest = FALSE;
    drone_can_north = drone_can_south = drone_can_east = drone_can_west = FALSE;

    target_player = player_data[player].dr_currentTarget;
    /* position of the drone */
    playerY = player_data[player].ply_y;
    playerX = player_data[player].ply_x;
    /* position of the target */
    targetY = player_data[target_player].ply_y;
    targetX = player_data[target_player].ply_x;
    player_data[player].dr_targetLocked = FALSE; /* no active target to fire at */
    player_data[player].dr_fireDirection = -1; /* no fire direction (only set, never read) */
    /* distance between us and the target (a human player would not have this info...) */
    targetDistanceY = targetY-playerY;
    targetDistanceX = targetX-playerX;

    if(player_data[target_player].ply_lives <= 0) /* target is dead? */
        player_data[player].dr_targetLocked = FALSE; /* no active target to fire at (already reset above) */

    /* try to turn in the direction the target is. This is an extremely simple algorithm */
    /* to find a target in the maze, also an unfair one, because it relies on the knowledge */
    /* where the target is - a human doesn't have that information.  */
    drone_check_directions(player, &drone_can_north, &drone_can_south, &drone_can_east, &drone_can_west, 0, 0, 0);

    if(targetDistanceY == 0) { /* drone at the same Y position as the target? */
        if(targetDistanceX < 0 && drone_can_west) { /* target is in the west? */
            drone_needs2GoWest = TRUE; /* try to go there */
        } else if(targetDistanceX > 0 && drone_can_east) { /* target is in the east? */
            drone_needs2GoEast = TRUE; /* try to go there */
        }

    } else if(targetDistanceX == 0) { /* drone at the same X position as the target? */
        if(targetDistanceY < 0 && drone_can_north) { /* target in the north? */
            drone_needs2GoNorth = TRUE; /* try to go there */
        } else if(targetDistanceY > 0 && drone_can_south) { /* target in the south? */
            drone_needs2GoSouth = TRUE; /* try to go there */
        }

    } else {
        if(targetDistanceY < 0) { /* target in the north => try to go there */
            if(drone_can_north) {
                drone_needs2GoNorth = TRUE;
            } else if(targetDistanceX < 0 && drone_can_west) { /* north is blocked, but target is also in the west?  */
                drone_needs2GoWest = TRUE;
            } else if(targetDistanceX > 0 && drone_can_east) { /* north is blocked, but target is also in the east? */
                drone_needs2GoEast = TRUE;
            }
        } else if(targetDistanceY > 0) { /* target in the south => try to go there */
            if(drone_can_south) {
                drone_needs2GoSouth = TRUE;
            } else if(targetDistanceX < 0 && drone_can_west) { /* south is blocked, but target is also in the west? */
                drone_needs2GoWest = TRUE;
            } else if(targetDistanceX > 0 && drone_can_east) { /* south is blocked, but target is also in the east? */
                drone_needs2GoEast = TRUE;
            }
        }
    }

    /* no direct move found and no planned move as well? */
    if(!drone_needs2GoNorth && !drone_needs2GoSouth && !drone_needs2GoEast && !drone_needs2GoWest && !player_data[player].dr_dir[0]) {
        /* fix the rotation of the drone */
        if(player_data[player].ply_dir > PLAYER_DIR_NORTH && player_data[player].ply_dir <= PLAYER_DIR_NORTHEAST && drone_can_north) {
            player_data[player].ply_dir = PLAYER_DIR_NORTH;
            drone_needs2GoNorth = TRUE;
        } else if(player_data[player].ply_dir > PLAYER_DIR_NORTHEAST && player_data[player].ply_dir <= PLAYER_DIR_SOUTHEAST && drone_can_east) {
            player_data[player].ply_dir = PLAYER_DIR_EAST;
            drone_needs2GoEast = TRUE;
        } else if(player_data[player].ply_dir > PLAYER_DIR_SOUTHEAST && player_data[player].ply_dir <= PLAYER_DIR_SOUTHWEST && drone_can_south) {
            player_data[player].ply_dir = PLAYER_DIR_SOUTH;
            drone_needs2GoSouth = TRUE;
        } else if(player_data[player].ply_dir > PLAYER_DIR_SOUTHWEST && player_data[player].ply_dir <= PLAYER_DIR_NORTHWEST && drone_can_west) {
            player_data[player].ply_dir = PLAYER_DIR_WEST;
            drone_needs2GoWest = TRUE;
        } else if(player_data[player].ply_dir > PLAYER_DIR_NORTHWEST && player_data[player].ply_dir < (PLAYER_DIR_NORTH+256) && drone_can_north) {
            player_data[player].ply_dir = PLAYER_DIR_NORTH;
            drone_needs2GoNorth = TRUE;
        }
    }

    /* is there a plan? If so => leave now! */
    if(player_data[player].dr_dir[0]) return;

    /* set the direction based on our previous checks */
    if(drone_needs2GoNorth) {
        player_data[player].ply_dir = PLAYER_DIR_NORTH;
        drone_can_south = drone_can_east = drone_can_west = FALSE;
        drone_can_north = TRUE;
        return;
    }
    if(drone_needs2GoSouth) {
        player_data[player].ply_dir = PLAYER_DIR_SOUTH;
        drone_can_north = drone_can_east = drone_can_west = FALSE;
        drone_can_south = TRUE;
        return;
    }
    if(drone_needs2GoEast) {
        player_data[player].ply_dir = PLAYER_DIR_EAST;
        drone_can_north = drone_can_south = drone_can_west = FALSE;
        drone_can_east = TRUE;
        return;
    }
    if(drone_needs2GoWest) {
        player_data[player].ply_dir = PLAYER_DIR_WEST;
        drone_can_north = drone_can_south = drone_can_east = FALSE;
        drone_can_west = TRUE;
    }
}

/************************************************************
 *** void drone_sub_ninja(int player)
 ************************************************************/
void drone_sub_ninja(int player) {
int target_player;
int fieldIndex;
int targetFieldX;
int targetFieldY;
int playerFieldX;
int playerFieldY;
int deltaX;
int deltaY;

    drone_needs2GoNorth = drone_needs2GoSouth = drone_needs2GoEast = drone_needs2GoWest = FALSE;
    target_player = player_data[player].dr_currentTarget;
    /* position of the player */
    playerFieldY = (player_data[player].ply_y >> MAZE_FIELD_SHIFT)|1;
    playerFieldX = (player_data[player].ply_x >> MAZE_FIELD_SHIFT)|1;
    /* position of the target */
    targetFieldY = (player_data[target_player].ply_y >> MAZE_FIELD_SHIFT)|1;
    targetFieldX = (player_data[target_player].ply_x >> MAZE_FIELD_SHIFT)|1;
    /* distance between us and the target (a human player would not have this info...) */
    deltaY = targetFieldY-playerFieldY;
    deltaX = targetFieldX-playerFieldX;

    drone_check_directions(player, &drone_can_north, &drone_can_south, &drone_can_east, &drone_can_west, 0, 0, 0);

    /* If the drone doesn't have a plan (!player_data[player].dr_dir[0]), */
    /* then do a similar search as the standard drone for the target, */
    /* if that is not possible, make a plan how to get to the target (ninja only feature) */
    if(deltaY == 0 && !player_data[player].dr_dir[0]) {
        if(deltaX < 0) {
            if(drone_can_west) {
                drone_needs2GoWest = TRUE;
                if(drone_isTargetIsVisibleWest(player, 0, 0)) {
                    if(drone_aim2target(player)) {
                        player_data[player].dr_targetLocked = TRUE;
                    } else if(player_data[player].dr_targetLocked) {
                        drone_sub_findMoveToTarget(player);
                    }
                } else if(player_data[player].dr_targetLocked) {
                    drone_sub_findMoveToTarget(player);
                }
            } else {
                /* drone wants to go west, but can't */
                if(drone_sub_ninja_plan(player, 'w')) { /* try via north/south to walk around the obstruction  */
                } else if((_rnd(256)&1) && drone_sub_ninja_north(player, 'w')) {
                } else if(drone_sub_ninja_north(player, 'e')) {
                } else if(drone_sub_ninja_north(player, 'w')) {
                } else if(drone_sub_ninja_south(player, 'w')) {
                } else if(drone_sub_ninja_east(player, 'n')) {
                } else {
                    drone_sub_ninja_east(player, 's');
                }
            }
        } else if(deltaX > 0) {
            if(drone_can_east) {
                drone_needs2GoEast = TRUE;
                if(drone_isTargetIsVisibleEast(player, 0, 0)) {
                    if(drone_aim2target(player)) {
                        player_data[player].dr_targetLocked = TRUE;
                    } else if(player_data[player].dr_targetLocked) {
                        drone_sub_findMoveToTarget(player);
                    }
                } else if(player_data[player].dr_targetLocked) {
                    drone_sub_findMoveToTarget(player);
                }
            } else {
                /* drone wants to go east, but can't */
                if(drone_sub_ninja_plan(player, 'e')) { /* try via north/south to walk around the obstruction */
                } else if((_rnd(256)&1) && drone_sub_ninja_north(player, 'e')) {
                } else if(drone_sub_ninja_north(player, 'w')) {
                } else if(drone_sub_ninja_north(player, 'e')) {
                } else if(drone_sub_ninja_south(player, 'e')) {
                } else if(drone_sub_ninja_west(player, 'n')) {
                } else {
                    drone_sub_ninja_west(player, 's');
                }
            }
        }
    } else if(deltaX == 0 && !player_data[player].dr_dir[0]) {
        if(deltaY < 0) {
            if(drone_can_north) {
                drone_needs2GoNorth = TRUE;
                if(drone_isTargetIsVisibleNorth(player, 0, 0)) {
                    if(drone_aim2target(player)) {
                        player_data[player].dr_targetLocked = TRUE;
                    } else if(player_data[player].dr_targetLocked) {
                        drone_sub_findMoveToTarget(player);
                    }
                } else if(player_data[player].dr_targetLocked) {
                    drone_sub_findMoveToTarget(player);
                }
            } else {
                /* drone wants to go north, but can't */
                if(drone_sub_ninja_plan(player, 'n')) { /* try via west/east to walk around the obstruction */
                } else if((_rnd(256)&1) && drone_sub_ninja_west(player, 's')) {
                } else if(drone_sub_ninja_west(player, 'n')) {
                } else if(drone_sub_ninja_west(player, 's')) {
                } else if(drone_sub_ninja_east(player, 'n')) {
                } else if(drone_sub_ninja_south(player, 'e')) {
                } else {
                    drone_sub_ninja_south(player, 'w');
                }
            }
        } else if(deltaY > 0) {
            if(drone_can_south) {
                drone_needs2GoSouth = TRUE;
                if(drone_isTargetIsVisibleSouth(player, 0, 0)) {
                    if(drone_aim2target(player)) {
                        player_data[player].dr_targetLocked = TRUE;
                    } else if(player_data[player].dr_targetLocked) {
                        drone_sub_findMoveToTarget(player);
                    }
                } else if(player_data[player].dr_targetLocked) {
                    drone_sub_findMoveToTarget(player);
                }
            } else {
                /* drone wants to go south, but can't */
                if(drone_sub_ninja_plan(player, 's')) { /* try via west/east to walk around the obstruction */
                } else if((_rnd(256)&1) && drone_sub_ninja_east(player, 'n')) {
                } else if(drone_sub_ninja_east(player, 's')) {
                } else if(drone_sub_ninja_east(player, 'n')) {
                } else if(drone_sub_ninja_west(player, 's')) {
                } else if(drone_sub_ninja_north(player, 'e')) {
                } else {
                    drone_sub_ninja_north(player, 'w');
                }
            }
        }
    } else if(deltaY < 0 && !player_data[player].dr_dir[0]) {
        if(player_data[player].dr_targetLocked) {
            drone_sub_findMoveToTarget(player);
        } else if(drone_can_north) {
            drone_needs2GoNorth = TRUE;
        } else if(deltaX < 0 && drone_can_west) {
            drone_needs2GoWest = TRUE;
        } else if(deltaX > 0 && drone_can_east) {
            drone_needs2GoEast = TRUE;
        } else if(!drone_can_north && deltaX > 0 && !drone_can_east) {
            if((_rnd(256)&1) && drone_sub_ninja_west(player, 's')) {
            } else if(drone_sub_ninja_west(player, 'n')) {
            } else if(drone_sub_ninja_west(player, 's')) {
            } else if(drone_sub_ninja_south(player, 'e')) {
            } else {
                drone_sub_ninja_south(player, 'w');
            }
        } else if(!drone_can_north && deltaX < 0 && !drone_can_west) {
            if((_rnd(256)&1) && drone_sub_ninja_east(player, 's')) {
            } else if(drone_sub_ninja_east(player, 'n')) {
            } else if(drone_sub_ninja_east(player, 's')) {
            } else if(drone_sub_ninja_south(player, 'w')) {
            } else {
                drone_sub_ninja_south(player, 'e');
            }
        }
    } else if(deltaY > 0 && !player_data[player].dr_dir[0]) {
        if(player_data[player].dr_targetLocked) {
            drone_sub_findMoveToTarget(player);
        } else if(drone_can_south) {
            drone_needs2GoSouth = TRUE;
        } else if(deltaX < 0 && drone_can_west) {
            drone_needs2GoWest = TRUE;
        } else if(deltaX > 0 && drone_can_east) {
            drone_needs2GoEast = TRUE;
        } else if(!drone_can_south && deltaX < 0 && !drone_can_west) {
            if((_rnd(256)&1) && drone_sub_ninja_east(player, 'n')) {
            } else if(drone_sub_ninja_east(player, 's')) {
            } else if(drone_sub_ninja_east(player, 'n')) {
            } else if(drone_sub_ninja_north(player, 'e')) {
            } else {
                drone_sub_ninja_north(player, 'w');
            }
        } else if(!drone_can_south && deltaX > 0 && !drone_can_east) {
            if((_rnd(256)&1) && drone_sub_ninja_west(player, 'n')) {
            } else if(drone_sub_ninja_west(player, 's')) {
            } else if(drone_sub_ninja_west(player, 'n')) {
            } else if(drone_sub_ninja_north(player, 'w')) {
            } else {
                drone_sub_ninja_north(player, 'e');
            }
        }
    }

    /* Does the drone have a plan? Execute that plan! */
    if(player_data[player].dr_dir[0]) {

        /* current position in our action list */
        fieldIndex = player_data[player].dr_fieldIndex;
        /* did the player reach the destination field? */
        if(player_data[player].dr_field[fieldIndex].y == playerFieldY && player_data[player].dr_field[fieldIndex].x == playerFieldX) {
            /* then increment to the next position */
            player_data[player].dr_fieldIndex++;
            fieldIndex = player_data[player].dr_fieldIndex;
            player_data[player].dr_fieldResetTimer = 0;
        }

        /* After 78 steps, we do time-out and give up */
        if(player_data[player].dr_fieldResetTimer++ > 78) {
            player_data[player].dr_field[fieldIndex].y = 0;
            player_data[player].dr_fieldResetTimer = 0;
        }

        if(player_data[player].dr_field[fieldIndex].y == 0 || player_data[player].dr_dir[fieldIndex] == -1) {
            /* reset motion plan */
            player_data[player].dr_dir[0] = 0;
            player_data[player].dr_fieldIndex = 0;
            player_data[player].dr_upRotationCounter = 0;
            player_data[player].dr_rotateCounter = 0;
            /* take the last destination as our target destination */
            if(fieldIndex > 0) {
                if(player_data[player].dr_dir[fieldIndex-1] == (PLAYER_DIR_NORTH+256)) {
                    drone_needs2GoNorth = TRUE;
                } else if(player_data[player].dr_dir[fieldIndex-1] == PLAYER_DIR_EAST) {
                    drone_needs2GoEast = TRUE;
                } else if(player_data[player].dr_dir[fieldIndex-1] == PLAYER_DIR_SOUTH) {
                    drone_needs2GoSouth = TRUE;
                } else if(player_data[player].dr_dir[fieldIndex-1] == PLAYER_DIR_WEST) {
                    drone_needs2GoWest = TRUE;
                }
            } else { /* BUG: this block is identical to the one above, which is a problem, because fieldIndex-1 == -1 here. */
#if 0
                if(player_data[player].dr_dir[fieldIndex-1] == (PLAYER_DIR_NORTH+256)) {
                    drone_needs2GoNorth = TRUE;
                } else if(player_data[player].dr_dir[fieldIndex-1] == PLAYER_DIR_EAST) {
                    drone_needs2GoEast = TRUE;
                } else if(player_data[player].dr_dir[fieldIndex-1] == PLAYER_DIR_SOUTH) {
                    drone_needs2GoSouth = TRUE;
                } else if(player_data[player].dr_dir[fieldIndex-1] == PLAYER_DIR_WEST) {
                    drone_needs2GoWest = TRUE;
                }
#endif
            }
        } else if(player_data[player].dr_dir[fieldIndex] == (PLAYER_DIR_NORTH+256) && drone_can_north) { /* plan: going north (and player can go north) */
            if(fieldIndex == 0 && player_data[player].ply_dir) { /* if player is not aligned north, do so */
                drone_set_position(player, 'n');
            }
            drone_needs2GoNorth = TRUE;
            if(deltaX == 0 && deltaY < 0) {
                if(drone_isTargetIsVisibleNorth(player, 0, 0)) { /* can we see the target? */
                    if(drone_aim2target(player)) { /* yes => aim and lock */
                        player_data[player].dr_targetLocked = TRUE;
                        /* reset motion plan */
                        player_data[player].dr_dir[0] = 0;
                        player_data[player].dr_fieldIndex = 0;
                        player_data[player].dr_upRotationCounter = 0;
                        player_data[player].dr_rotateCounter = 0;
                    } else if(player_data[player].dr_targetLocked) {
                        drone_sub_findMoveToTarget(player);
                    }
                } else if(player_data[player].dr_targetLocked) {
                    drone_sub_findMoveToTarget(player);
                }
            }
        } else if(player_data[player].dr_dir[fieldIndex] == PLAYER_DIR_EAST && drone_can_east) { /* plan: going east (and player can go east) */
            if(fieldIndex == 0 && player_data[player].ply_dir != PLAYER_DIR_EAST) { /* if player is not aligned east, do so */
                drone_set_position(player, 'e');
            }
            drone_needs2GoEast = TRUE;
            if(deltaY == 0 && deltaX > 0) {
                if(drone_isTargetIsVisibleEast(player, 0, 0)) { /* can we see the target? */
                    if(drone_aim2target(player)) { /* yes => aim and lock */
                        player_data[player].dr_targetLocked = TRUE;
                        /* reset motion plan */
                        player_data[player].dr_dir[0] = 0;
                        player_data[player].dr_fieldIndex = 0;
                        player_data[player].dr_upRotationCounter = 0;
                        player_data[player].dr_rotateCounter = 0;
                    } else if(player_data[player].dr_targetLocked) {
                        drone_sub_findMoveToTarget(player);
                    }
                } else if(player_data[player].dr_targetLocked) {
                    drone_sub_findMoveToTarget(player);
                }
            }
        } else if(player_data[player].dr_dir[fieldIndex] == PLAYER_DIR_SOUTH && drone_can_south) { /* plan: going south (and player can go south) */
            if(player_data[player].ply_dir != PLAYER_DIR_SOUTH && fieldIndex == 0) { /* if player is not aligned south, do so */
                drone_set_position(player, 's');
            }
            drone_needs2GoSouth = TRUE;
            if(deltaX == 0 && deltaY > 0) {
                if(drone_isTargetIsVisibleSouth(player, 0, 0)) { /* can we see the target? */
                    if(drone_aim2target(player)) { /* yes => aim and lock */
                        player_data[player].dr_targetLocked = TRUE;
                        /* reset motion plan */
                        player_data[player].dr_dir[0] = 0;
                        player_data[player].dr_fieldIndex = 0;
                        player_data[player].dr_upRotationCounter = 0;
                        player_data[player].dr_rotateCounter = 0;
                    } else if(player_data[player].dr_targetLocked) {
                        drone_sub_findMoveToTarget(player);
                    }
                } else if(player_data[player].dr_targetLocked) {
                    drone_sub_findMoveToTarget(player);
                }
            }
        } else if(player_data[player].dr_dir[fieldIndex] == PLAYER_DIR_WEST && drone_can_west) { /* plan: going west (and player can go west) */
            if(player_data[player].ply_dir != PLAYER_DIR_WEST && fieldIndex == 0) { /* if player is not aligned west, do so */
                drone_set_position(player, 'w');
            }
            drone_needs2GoWest = TRUE;
            if(deltaY == 0 && deltaX < 0) {
                if(drone_isTargetIsVisibleWest(player, 0, 0)) { /* can we see the target? */
                    if(drone_aim2target(player)) { /* yes => aim and lock */
                        player_data[player].dr_targetLocked = TRUE;
                        /* reset motion plan */
                        player_data[player].dr_dir[0] = 0;
                        player_data[player].dr_fieldIndex = 0;
                        player_data[player].dr_upRotationCounter = 0;
                        player_data[player].dr_rotateCounter = 0;
                    } else if(player_data[player].dr_targetLocked) {
                        drone_sub_findMoveToTarget(player);
                    }
                } else if(player_data[player].dr_targetLocked) {
                    drone_sub_findMoveToTarget(player);
                }
            }
        }
    }

    /* setup variables for the joystick movement generation code */
    if(drone_needs2GoNorth) {
        drone_can_south = drone_can_east = drone_can_west = FALSE;
        drone_can_north = TRUE;
    } else if(drone_needs2GoSouth) {
        drone_can_north = drone_can_east = drone_can_west = FALSE;
        drone_can_south = TRUE;
    } else if(drone_needs2GoEast) {
        drone_can_north = drone_can_south = drone_can_west = FALSE;
        drone_can_east = TRUE;
    } else if(drone_needs2GoWest) {
        drone_can_north = drone_can_south = drone_can_east = FALSE;
        drone_can_west = TRUE;
    }
}

/************************************************************
 *** This code moves the drone in the current direction
 *** as long as that direction is possible. A non target drone
 *** tries to take corners, if possible.
 *** If a movement is blocked, this code will change direction
 *** to one that is possible.
 *** If the drone is not 90-degree aligned, it does nothing.
 *** This will continue an existing rotation motion, till
 *** alignment is archived.
 ************************************************************/
void drone_generate_joystickdata(int player) {
int dir;

    dir = player_data[player].ply_dir;

    /* is there a target locked? */
    if(player_data[player].dr_targetLocked) {
        /* don't do anything but shoot! */
        player_joy_table[player] = JOYSTICK_BUTTON;
        return;
    }

    /* generate motion into the direction the player is facing, if the direction is possible */
    /* A target drone always goes up, while the others will try to go around a corner, if there */
    /* is a chance to do so, e.g. if north and east are possible, it will go north-east. */
    if(dir == PLAYER_DIR_NORTH && drone_can_north) {
        if(player_data[player].dr_type == DRONE_TARGET) {
            drone_move_up(player);
        } else {
            /* tendency to go east, because it is tested first */
            if(drone_can_east) {
                drone_move_upright(player);
            } else if(drone_can_west) {
                drone_move_upleft(player);
            } else {
                drone_move_up(player);
            }
        }
        return;
    }
    if(dir == PLAYER_DIR_EAST && drone_can_east) {
        if(player_data[player].dr_type == DRONE_TARGET) {
            drone_move_up(player);
        } else {
            /* tendency to go south, because it is tested first */
            if(drone_can_south) {
                drone_move_upright(player);
            } else if(drone_can_north) {
                drone_move_upleft(player);
            } else {
                drone_move_up(player);
            }
        }
        return;
    }
    if(dir == PLAYER_DIR_SOUTH && drone_can_south) {
        if(player_data[player].dr_type == DRONE_TARGET) {
            drone_move_up(player);
        } else {
            /* tendency to go east, because it is tested first */
            if(drone_can_east) {
                drone_move_upleft(player);
            } else if(drone_can_west) {
                drone_move_upright(player);
            } else {
                drone_move_up(player);
            }
        }
        return;
    }
    if(dir == PLAYER_DIR_WEST && drone_can_west) {
        if(player_data[player].dr_type == DRONE_TARGET) {
            drone_move_up(player);
        } else {
            /* tendency to go south, because it is tested first */
            if(drone_can_south) {
                drone_move_upleft(player);
            } else if(drone_can_north) {
                drone_move_upright(player);
            } else {
                drone_move_up(player);
            }
        }
        return;
    }

    /* Drone faces a certain direction, but this direction is not possible. */
    /* In these cases it tries to turn left/right - and if this doesn't work either: turn around */
    if(dir == PLAYER_DIR_NORTH && !drone_can_north) {
        /* tendency to go east, because it is tested first */
        if(drone_can_east) {
            drone_move_upright(player);
        } else if(drone_can_west) {
            drone_move_upleft(player);
        } else {
            drone_turn_around(player);
        }
        return;
    }
    if(dir == PLAYER_DIR_EAST && !drone_can_east) {
        /* tendency to go north, because it is tested first */
        if(drone_can_north) {
            drone_move_upleft(player);
        } else if(drone_can_south) {
            drone_move_upright(player);
        } else {
            drone_turn_around(player);
        }
        return;
    }
    if(dir == PLAYER_DIR_SOUTH && !drone_can_south) {
        /* tendency to go east, because it is tested first */
        if(drone_can_east) {
            drone_move_upleft(player);
        } else if(drone_can_west) {
            drone_move_upright(player);
        } else {
            drone_turn_around(player);
        }
        return;
    }
    if(dir == PLAYER_DIR_WEST && !drone_can_west) {
        /* tendency to go north, because it is tested first */
        if(drone_can_north) {
            drone_move_upright(player);
        } else if(drone_can_south) {
            drone_move_upleft(player);
        } else {
            drone_turn_around(player);
        }
        return;
    }
}

/************************************************************
 *** Adjust the direction of a drone to aim at the target.
 *** There is a 800 unit cut-off (ab bit more than 3 fields away)
 *** or if the target is dead (duh). This allows shooting
 *** drones from 3.5 fiels away, as long as they don't move.
 *** They will not fire back!
 ************************************************************/
int drone_aim2target(int player) {
int target_player;
int deltaX;
int deltaY;

    target_player = player_data[player].dr_currentTarget;

    deltaY = player_data[target_player].ply_y-player_data[player].ply_y;
    deltaX = player_data[target_player].ply_x-player_data[player].ply_x;
    /* player too far away (a bit more than 3 fields away) or player dead? */
    if(abs(deltaY) > 800 || abs(deltaX) > 800 || player_data[target_player].ply_lives <= 0)
        return NO;

    /* if there is no target to fire at, the fire direction matches the moving direction */
    /* This property is only set, but never read */
    if(!player_data[player].dr_targetLocked)
        player_data[player].dr_fireDirection = player_data[player].ply_dir;

    /* aim for the target player */
    player_data[player].ply_dir = drone_delta_into_direction(deltaY, deltaX);

    return YES;
}

/************************************************************
 *** Calculate an angle between two objects which are deltaY/deltaX apart
 ***
 *** This function could be implemented with floating point math as:
 *** return 0xff & ((int)(atan2(deltaY, deltaX) * 256 / (2 * M_PI) + 64)); 
 *** As you can see: atan2() then converted from radian to 256 degrees and
 *** finally turned by 90 degrees to the left, because north is 0 in MIDImaze 
 ************************************************************/
int drone_delta_into_direction(int deltaY,int deltaX) {
int angle;
int deltaXIsPositive;
int deltaYIsPositive;

    deltaYIsPositive = deltaY >= 0;
    deltaY = abs(deltaY);
    deltaXIsPositive = deltaX >= 0;
    deltaX = abs(deltaX);
    if(deltaX <= deltaY) {
        angle = drone_angle_table[muls_divs(32, deltaX, deltaY)];
    } else {
        angle = 64-drone_angle_table[muls_divs(32, deltaY, deltaX)];
    }

    /* generate the angle for one of the 4 quadrants */
    switch((deltaYIsPositive<<1)+deltaXIsPositive) {
    case 0: angle += 128; break;
    case 1: angle = 128-angle; break; 
    case 2: angle = 256-angle; break;
    case 3: break;
    }

    /* flip the angle, so that it is a normal angle like everywhere else throughout the app (0 = north) */
    /* (BTW: deltaY=deltaX=0 will result in 128, which might be a bit weird) */
    return (128-angle) & 0xff;
}

/************************************************************
 *** Check if a target can be seen in the north on the same width as the player
 ************************************************************/
int drone_isTargetIsVisibleNorth(int player,int fieldY,int fieldX) {
int target_player;
int playerFieldX;
int playerFieldY;
int targetFieldX;
int targetFieldY;

    target_player = player_data[player].dr_currentTarget;
    if(player_data[target_player].ply_lives <= 0) /* target is dead */
        return NO;
    targetFieldY = (player_data[target_player].ply_y >> MAZE_FIELD_SHIFT)|1;
    targetFieldX = (player_data[target_player].ply_x >> MAZE_FIELD_SHIFT)|1;
    playerFieldY = fieldY ? fieldY : (player_data[player].ply_y >> MAZE_FIELD_SHIFT)|1;
    playerFieldX = fieldX ? fieldX : (player_data[player].ply_x >> MAZE_FIELD_SHIFT)|1;
    if(targetFieldX != playerFieldX || playerFieldY < targetFieldY) /* not the the same X-field or target is behind the player? */
        return NO;
    if(playerFieldY <= 1) /* left the maze? */
        return NO;
    if(targetFieldX == playerFieldX && targetFieldY == playerFieldY) /* already on the same field? */
        return YES;
    /* scan up till we either find the player or a wall */
    while(get_maze_data(playerFieldY-1, playerFieldX, 0) == MAZE_FIELD_EMPTY && targetFieldY != playerFieldY) {
        playerFieldY -= 2;
        if(playerFieldY == 0) break; /* This test should not be necessary, because the maze is surrounded by walls anyway */
    }
    if(targetFieldY != playerFieldY) /* did we find the target? */
        return NO;
    return YES;
}

/************************************************************
 *** Check if a target can be seen in the south on the same width as the player
 ************************************************************/
int drone_isTargetIsVisibleSouth(int player,int fieldY,int fieldX) {
int target_player;
int playerFieldX;
int playerFieldY;
int targetFieldX;
int targetFieldY;

    target_player = player_data[player].dr_currentTarget;
    if(player_data[target_player].ply_lives <= 0) /* target is dead */
        return NO;
    targetFieldY = (player_data[target_player].ply_y >> MAZE_FIELD_SHIFT)|1;
    targetFieldX = (player_data[target_player].ply_x >> MAZE_FIELD_SHIFT)|1;
    playerFieldY = fieldY ? fieldY : (player_data[player].ply_y >> MAZE_FIELD_SHIFT)|1;
    playerFieldX = fieldX ? fieldX : (player_data[player].ply_x >> MAZE_FIELD_SHIFT)|1;
    if(targetFieldX != playerFieldX || playerFieldY > targetFieldY) /* not the the same X-field or target is behind the player? */
        return NO;
    if(playerFieldY > MAZE_MAX_SIZE-1) /* left the maze? */
        return NO;
    if(targetFieldX == playerFieldX && targetFieldY == playerFieldY) /* already on the same field? */
        return YES;
    /* scan down till we either find the player or a wall */
    while(get_maze_data(playerFieldY+1, playerFieldX, 0) == MAZE_FIELD_EMPTY && targetFieldY != playerFieldY) {
        playerFieldY += 2;
        if(playerFieldY > MAZE_MAX_SIZE-1) break; /* This test should not be necessary, because the maze is surrounded by walls anyway */
    }
    if(targetFieldY != playerFieldY) /* did we find the target? */
        return NO;
    return YES;
}

/************************************************************
 *** Check if a target can be seen in the east on the same height as the player
 ************************************************************/
int drone_isTargetIsVisibleEast(int player,int fieldY,int fieldX) {
int target_player;
int playerFieldX;
int playerFieldY;
int targetFieldX;
int targetFieldY;

    target_player = player_data[player].dr_currentTarget;
    if(player_data[target_player].ply_lives <= 0) /* target is dead */
        return NO;
    targetFieldX = (player_data[target_player].ply_x >> MAZE_FIELD_SHIFT)|1;
    targetFieldY = (player_data[target_player].ply_y >> MAZE_FIELD_SHIFT)|1;
    playerFieldY = fieldY ? fieldY : (player_data[player].ply_y >> MAZE_FIELD_SHIFT)|1;
    playerFieldX = fieldX ? fieldX : (player_data[player].ply_x >> MAZE_FIELD_SHIFT)|1;
    if(targetFieldY != playerFieldY || targetFieldX < playerFieldX) /* not the the same Y-field or target is behind the player? */
        return NO;
    if(playerFieldX > MAZE_MAX_SIZE-1) /* left the maze? */
        return NO;
    if(targetFieldX == playerFieldX && targetFieldY == playerFieldY) /* already on the same field? */
        return YES;
    /* scan right till we either find the player or a wall */
    while(get_maze_data(playerFieldY, playerFieldX+1, 0) == MAZE_FIELD_EMPTY && targetFieldX != playerFieldX) {
        playerFieldX += 2;
        if(playerFieldX > MAZE_MAX_SIZE-1) break; /* This test should not be necessary, because the maze is surrounded by walls anyway */
    }
    if(targetFieldX != playerFieldX) /* did we find the target? */
        return NO;
    return YES;
}

/************************************************************
 *** Check if a target can be seen in the west on the same height as the player
 ************************************************************/
int drone_isTargetIsVisibleWest(int player,int fieldY,int fieldX) {
int target_player;
int playerFieldX;
int playerFieldY;
int targetFieldX;
int targetFieldY;

    target_player = player_data[player].dr_currentTarget;
    if(player_data[target_player].ply_lives <= 0) /* target is dead */
        return NO;
    targetFieldX = (player_data[target_player].ply_x >> MAZE_FIELD_SHIFT)|1;
    targetFieldY = (player_data[target_player].ply_y >> MAZE_FIELD_SHIFT)|1;
    playerFieldY = fieldY ? fieldY : (player_data[player].ply_y >> MAZE_FIELD_SHIFT)|1;
    playerFieldX = fieldX ? fieldX : (player_data[player].ply_x >> MAZE_FIELD_SHIFT)|1;
    if(targetFieldY != playerFieldY || playerFieldX < targetFieldX) /* not the the same Y-field or target is behind the player? */
        return NO;
    if(playerFieldX <= 1) /* left the maze? */
        return NO;
    if(targetFieldX == playerFieldX && targetFieldY == playerFieldY) /* already on the same field? */
        return YES;
    /* scan left till we either find the player or a wall */
    while(get_maze_data(playerFieldY, playerFieldX-1, 0) == MAZE_FIELD_EMPTY && targetFieldX != playerFieldX) {
        playerFieldX -= 2;
        if(playerFieldX == 0) break; /* This test should not be necessary, because the maze is surrounded by walls anyway */
    }
    if(targetFieldX != playerFieldX) /* did we find the target? */
        return NO;
    return YES;
}

/************************************************************
 *** Unlock a drone by forcing a position/direction adjustment
 ************************************************************/
void drone_set_position(int player,int viewCompassDirChar) {
int playerFieldX;
int playerFieldY;

    playerFieldY = (player_data[player].ply_y >> MAZE_FIELD_SHIFT)|1;
    playerFieldX = (player_data[player].ply_x >> MAZE_FIELD_SHIFT)|1;
    switch(viewCompassDirChar) {
    case 'n':
        player_data[player].ply_dir = PLAYER_DIR_NORTH;
        player_data[player].ply_y = playerFieldY << MAZE_FIELD_SHIFT;
        player_data[player].ply_x = playerFieldX << MAZE_FIELD_SHIFT;
        set_object(player, player_data[player].ply_y, player_data[player].ply_x);
        player_data[player].ply_plist = MAZE_FIELD_EMPTY;
        drone_can_south = drone_can_east = drone_can_west = FALSE;
        drone_can_north = TRUE;
        break;
    case 's':
        player_data[player].ply_dir = PLAYER_DIR_SOUTH;
        player_data[player].ply_y = playerFieldY << MAZE_FIELD_SHIFT;
        player_data[player].ply_x = playerFieldX << MAZE_FIELD_SHIFT;
        set_object(player, player_data[player].ply_y, player_data[player].ply_x);
        player_data[player].ply_plist = MAZE_FIELD_EMPTY;
        drone_can_north = drone_can_east = drone_can_west = FALSE;
        drone_can_south = TRUE;
        break;
    case 'e':
        player_data[player].ply_dir = PLAYER_DIR_EAST;
        player_data[player].ply_y = playerFieldY << MAZE_FIELD_SHIFT;
        player_data[player].ply_x = playerFieldX << MAZE_FIELD_SHIFT;
        set_object(player, player_data[player].ply_y, player_data[player].ply_x);
        player_data[player].ply_plist = MAZE_FIELD_EMPTY;
        drone_can_north = drone_can_south = drone_can_west = FALSE;
        drone_can_east = TRUE;
        break;
    case 'w':
        player_data[player].ply_dir = PLAYER_DIR_WEST;
        player_data[player].ply_y = playerFieldY << MAZE_FIELD_SHIFT;
        player_data[player].ply_x = playerFieldX << MAZE_FIELD_SHIFT;
        set_object(player, player_data[player].ply_y, player_data[player].ply_x);
        player_data[player].ply_plist = MAZE_FIELD_EMPTY;
        drone_can_north = drone_can_south = drone_can_east = FALSE;
        drone_can_west = TRUE;
        break;
    default:
        return;
    }
}

/************************************************************
 *** void drone_sub_standard(int player)
 ************************************************************/
void drone_sub_standard(int player) {
int target_player;
int targetDistanceX;
int targetDistanceY;
int targetFieldX;
int targetFieldY;
int playerFieldX;
int playerFieldY;

    /* The target is either the player that shot this standard drone (only humans will be a target) or the current target */
    target_player = (player_data[player].ply_hitflag && (player_data[player_data[player].ply_gunman].dr_type != DRONE_NINJA || player_data[player_data[player].ply_gunman].dr_type != DRONE_STANDARD)) ? player_data[player].ply_gunman : player_data[player].dr_currentTarget;
    /* position of the player */
    playerFieldY = (player_data[player].ply_y >> MAZE_FIELD_SHIFT)|1;
    playerFieldX = (player_data[player].ply_x >> MAZE_FIELD_SHIFT)|1;
    /* position of the target */
    targetFieldY = (player_data[target_player].ply_y >> MAZE_FIELD_SHIFT)|1;
    targetFieldX = (player_data[target_player].ply_x >> MAZE_FIELD_SHIFT)|1;
    /* distance between us and the target (a human player would not have this info...) */
    targetDistanceY = targetFieldY-playerFieldY;
    targetDistanceX = targetFieldX-playerFieldX;
    
    /* was the player hit right now? Change the direction _now_  */
    if(player_data[player].ply_hitflag) {
        player_data[player].ply_dir = drone_delta_into_direction(targetDistanceY, targetDistanceX);
        return;
    }

    /* This code looks down an axis for the target, if found */
    /* it aims and locks at the target. */
    /* If the target is just _one_ field off, the drone will not */
    /* see it. This is why standard drones can be easily show */
    /* diagonally! */
    /* If a locked target can no longer be seen, the lock is lost */
    /* and the drone starts to search again for the target.  */
    if(targetDistanceY == 0) {
        if(targetDistanceX < 0) { /* target is in the west */
            if(drone_isTargetIsVisibleWest(player, 0, 0)) {
                if(drone_aim2target(player)) {
                    player_data[player].dr_targetLocked = TRUE;
                } else if(player_data[player].dr_targetLocked) {
                    drone_sub_findMoveToTarget(player);
                    player_data[player].dr_targetLocked = FALSE;
                }
            } else if(player_data[player].dr_targetLocked) {
                drone_sub_findMoveToTarget(player);
                player_data[player].dr_targetLocked = FALSE;
            }
        } else if(targetDistanceX > 0) { /* target is in the east */
            if(drone_isTargetIsVisibleEast(player, 0, 0)) {
                if(drone_aim2target(player)) {
                    player_data[player].dr_targetLocked = TRUE;
                } else if(player_data[player].dr_targetLocked) {
                    drone_sub_findMoveToTarget(player);
                    player_data[player].dr_targetLocked = FALSE;
                }
            } else if(player_data[player].dr_targetLocked) {
                drone_sub_findMoveToTarget(player);
                player_data[player].dr_targetLocked = FALSE;
            }
        }
    } else if(targetDistanceX == 0) {
        if(targetDistanceY < 0) { /* target is in the north */
            if(drone_isTargetIsVisibleNorth(player, 0, 0)) {
                if(drone_aim2target(player)) {
                    player_data[player].dr_targetLocked = TRUE;
                } else if(player_data[player].dr_targetLocked) {
                    drone_sub_findMoveToTarget(player);
                    player_data[player].dr_targetLocked = FALSE;
                }
            } else if(player_data[player].dr_targetLocked) {
                drone_sub_findMoveToTarget(player);
                player_data[player].dr_targetLocked = FALSE;
            }
        } else if(targetDistanceY > 0) { /* target is in the south */
            if(drone_isTargetIsVisibleSouth(player, 0, 0)) {
                if(drone_aim2target(player)) {
                    player_data[player].dr_targetLocked = TRUE;
                } else if(player_data[player].dr_targetLocked) {
                    drone_sub_findMoveToTarget(player);
                    player_data[player].dr_targetLocked = FALSE;
                }
            } else if(player_data[player].dr_targetLocked) {
                drone_sub_findMoveToTarget(player);
                player_data[player].dr_targetLocked = FALSE;
            }
        }
    } else {
        /* lost track of our target? */
        if(player_data[player].dr_targetLocked) {
            player_data[player].dr_targetLocked = FALSE; /* reset and search again */
            drone_sub_findMoveToTarget(player);
        }
    }
}

/************************************************************
 *** int drone_sub_ninja_a(int player,int viewCompassDirChar)
 ************************************************************/
int drone_sub_ninja_plan(int player,int wantedDirChar) {
int target_player;
int playerFieldX;
int playerFieldY;
int targetFieldX;
int targetFieldY;
int canWest;
int canEast;
int canSouth;
int canNorth;

    target_player = player_data[player].dr_currentTarget;
    targetFieldY = (player_data[target_player].ply_y >> MAZE_FIELD_SHIFT)|1;
    targetFieldX = (player_data[target_player].ply_x >> MAZE_FIELD_SHIFT)|1;
    playerFieldY = (player_data[player].ply_y >> MAZE_FIELD_SHIFT)|1;
    playerFieldX = (player_data[player].ply_x >> MAZE_FIELD_SHIFT)|1;
    player_data[player].dr_fieldIndex = 0;
    player_data[player].dr_upRotationCounter = 0;
    player_data[player].dr_rotateCounter = 0;
    player_data[player].dr_dir[0] = 0;
    player_data[player].dr_field[0].y = 0;
    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 0, 0, 0);

    if(wantedDirChar == 'n') {
        if(canWest) { /* Move West, North (check for target), East (check for target), ... */
            player_data[player].dr_dir[0] = PLAYER_DIR_WEST;
            do {
                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                if(!canWest) break;
                playerFieldX -= 2;
                if(playerFieldX <= 0) break;
                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                if(!canSouth && !canNorth && !canWest) { /* dead end? */
                    playerFieldX = (player_data[player].ply_x >> MAZE_FIELD_SHIFT)|1; /* reset player position and exit */
                    break;
                }
                if(canNorth) {
                    player_data[player].dr_field[0].y = playerFieldY;
                    player_data[player].dr_field[0].x = playerFieldX;
                    player_data[player].dr_dir[1] = PLAYER_DIR_NORTH+256;
                    do {
                        drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                        if(!canNorth) {
                            playerFieldY = player_data[player].dr_field[0].y;
                            break;
                        }
                        playerFieldY -= 2;
                        if(playerFieldY <= 0) {
                            playerFieldY = player_data[player].dr_field[0].y;
                            break;
                        }
                        drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                        if(!canWest && !canNorth && !canEast) { /* dead end? */
                            playerFieldY = player_data[player].dr_field[0].y;
                            break;
                        }
                        if(canNorth && drone_isTargetIsVisibleNorth(player, playerFieldY, playerFieldX)) {
                            player_data[player].dr_field[1].y = playerFieldY;
                            player_data[player].dr_field[1].x = playerFieldX;
                            player_data[player].dr_field[2].y = 0;
                            player_data[player].dr_dir[2] = -1;
                            return YES;
                        }
                        if(canEast) {
                            player_data[player].dr_field[1].y = playerFieldY;
                            player_data[player].dr_field[1].x = playerFieldX;
                            player_data[player].dr_field[2].y = 0;
                            player_data[player].dr_dir[2] = PLAYER_DIR_EAST;
                            player_data[player].dr_dir[3] = -1;
                            if(drone_isTargetIsVisibleEast(player, playerFieldY, playerFieldX)) {
                                player_data[player].dr_field[2].y = playerFieldY;
                                player_data[player].dr_field[2].x = playerFieldX+2;
                                player_data[player].dr_field[3].y = 0;
                                return YES;
                            }
                            do {
                                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                if(!canEast) {
                                    playerFieldX = player_data[player].dr_field[1].x;
                                    break;
                                }
                                playerFieldX += 2;
                                if(playerFieldX > MAZE_MAX_SIZE-1) {
                                    playerFieldX = player_data[player].dr_field[1].x;
                                    break;
                                }
                                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                if(!canNorth && !canSouth && !canEast) { /* dead end? */
                                    playerFieldX = player_data[player].dr_field[1].x;
                                    break;
                                }
                                if(!canEast && targetFieldY < playerFieldY && !canNorth) {
                                    playerFieldX = player_data[player].dr_field[1].x;
                                    break;
                                }
                                if(!canEast && targetFieldY > playerFieldY && !canSouth) {
                                    playerFieldX = player_data[player].dr_field[1].x;
                                    break;
                                }
                                if(canNorth || canSouth) {
                                    player_data[player].dr_field[2].y = playerFieldY;
                                    player_data[player].dr_field[2].x = playerFieldX;
                                    player_data[player].dr_field[3].y = 0;
                                    if(targetFieldY < playerFieldY && canNorth) {
                                        player_data[player].dr_dir[3] = PLAYER_DIR_NORTH+256;
                                        player_data[player].dr_dir[4] = -1;
                                        do {
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(!canNorth) {
                                                playerFieldY = player_data[player].dr_field[2].y;
                                                break;
                                            }
                                            playerFieldY -= 2;
                                            if(playerFieldY <= 0) break;
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(drone_isTargetIsVisibleNorth(player, playerFieldY, playerFieldX)) {
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                            if(!canWest && !canNorth && !canEast) { /* dead end? */
                                                playerFieldY = player_data[player].dr_field[2].y;
                                                break;
                                            }
                                            if(targetFieldX < playerFieldX && canWest) {
                                                player_data[player].dr_dir[4] = PLAYER_DIR_WEST;
                                                player_data[player].dr_dir[5] = -1;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                            if(targetFieldX > playerFieldX && canEast) {
                                                player_data[player].dr_dir[4] = PLAYER_DIR_EAST;
                                                player_data[player].dr_dir[5] = -1;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                        } while(canNorth);
                                    } else if(targetFieldY > playerFieldY && canSouth) {
                                        player_data[player].dr_dir[3] = PLAYER_DIR_SOUTH;
                                        player_data[player].dr_dir[4] = -1;
                                        do {
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(!canSouth) {
                                                playerFieldY = player_data[player].dr_field[2].y;
                                                break;
                                            }
                                            playerFieldY += 2;
                                            if(playerFieldY > MAZE_MAX_SIZE-1) break;
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(drone_isTargetIsVisibleSouth(player, playerFieldY, playerFieldX)) {
                                                player_data[player].dr_field[4].y = playerFieldY;
                                                player_data[player].dr_field[4].x = playerFieldX;
                                                player_data[player].dr_field[5].y = 0;
                                                return YES;
                                            }
                                            if(!canEast && !canWest && !canSouth) { /* dead end? */
                                                playerFieldY = player_data[player].dr_field[2].x;
                                                break;
                                            }
                                            if(targetFieldX < playerFieldX && canWest) {
                                                player_data[player].dr_dir[4] = PLAYER_DIR_WEST;
                                                player_data[player].dr_dir[5] = -1;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                            if(targetFieldX > playerFieldX && canEast) {
                                                player_data[player].dr_dir[4] = PLAYER_DIR_EAST;
                                                player_data[player].dr_dir[5] = -1;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                        } while(canSouth);
                                    }
                                }
                            } while(canEast);
                            player_data[player].dr_dir[0] = 0;
                            player_data[player].dr_fieldIndex = 0;
                            break;
                        }
                    } while(canNorth);
                    player_data[player].dr_dir[0] = 0;
                    player_data[player].dr_fieldIndex = 0;
                    break;
                }
            } while(canWest);
            player_data[player].dr_dir[0] = 0;
            player_data[player].dr_fieldIndex = 0;
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 0, 0, 0);
            playerFieldY = (player_data[player].ply_y >> MAZE_FIELD_SHIFT)|1; /* reset player position */
            playerFieldX = (player_data[player].ply_x >> MAZE_FIELD_SHIFT)|1;
        }
        if(canEast) {
            player_data[player].dr_dir[0] = PLAYER_DIR_EAST;
            do {
                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                if(!canEast) break;
                playerFieldX += 2;
                if(playerFieldX > MAZE_MAX_SIZE-1) break;
                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                if(!canNorth && !canEast && !canSouth) break; /* dead end? */
                if(canNorth) {
                    player_data[player].dr_field[0].y = playerFieldY;
                    player_data[player].dr_field[0].x = playerFieldX;
                    player_data[player].dr_dir[1] = PLAYER_DIR_NORTH+256;
                    do {
                        drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                        if(!canNorth) {
                            playerFieldY = player_data[player].dr_field[0].y;
                            break;
                        }
                        playerFieldY -= 2;
                        if(playerFieldY <= 0) {
                            playerFieldY = player_data[player].dr_field[0].y;
                            break;
                        }
                        drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                        if(!canWest && !canNorth && !canEast) { /* dead end? */
                            playerFieldY = player_data[player].dr_field[0].y;
                            break;
                        }
                        if(canNorth && drone_isTargetIsVisibleNorth(player, playerFieldY, playerFieldX)) {
                            player_data[player].dr_field[1].y = playerFieldY;
                            player_data[player].dr_field[1].x = playerFieldX;
                            player_data[player].dr_field[2].y = 0;
                            player_data[player].dr_dir[2] = -1;
                            return YES;
                        }
                        if(canWest) {
                            player_data[player].dr_field[1].y = playerFieldY;
                            player_data[player].dr_field[1].x = playerFieldX;
                            player_data[player].dr_field[2].y = 0;
                            player_data[player].dr_dir[2] = PLAYER_DIR_WEST;
                            player_data[player].dr_dir[3] = -1;
                            if(drone_isTargetIsVisibleWest(player, playerFieldY, playerFieldX)) {
                                player_data[player].dr_field[2].y = playerFieldY;
                                player_data[player].dr_field[2].x = playerFieldX-2;
                                player_data[player].dr_field[3].y = 0;
                                return YES;
                            }
                            do {
                                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                if(!canWest) {
                                    playerFieldX = player_data[player].dr_field[1].x;
                                    break;
                                }
                                playerFieldX -= 2;
                                if(playerFieldX <= 0) {
                                    playerFieldX = player_data[player].dr_field[1].x;
                                    break;
                                }
                                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                if(!canSouth && !canNorth && !canWest) { /* dead end? */
                                    playerFieldX = player_data[player].dr_field[1].x;
                                    break;
                                }
                                if(!canNorth && targetFieldX < playerFieldX && !canWest) {
                                    playerFieldX = player_data[player].dr_field[1].x;
                                    break;
                                }
                                if(!canSouth && targetFieldX > playerFieldX && !canWest) {
                                    playerFieldX = player_data[player].dr_field[1].x;
                                    break;
                                }
                                if(canNorth || canSouth) {
                                    player_data[player].dr_field[2].y = playerFieldY;
                                    player_data[player].dr_field[2].x = playerFieldX;
                                    player_data[player].dr_field[3].y = 0;
                                    if(targetFieldY < playerFieldY && canNorth) {
                                        player_data[player].dr_dir[3] = PLAYER_DIR_NORTH+256;
                                        player_data[player].dr_dir[4] = -1;
                                        do {
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(!canNorth) {
                                                playerFieldY = player_data[player].dr_field[2].y;
                                                break;
                                            }
                                            playerFieldY -= 2;
                                            if(playerFieldY <= 0) {
                                                playerFieldY = player_data[player].dr_field[2].y;
                                                break;
                                            }
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(drone_isTargetIsVisibleNorth(player, playerFieldY, playerFieldX)) {
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                            if(!canEast && !canNorth && !canWest) { /* dead end? */
                                                playerFieldY = player_data[player].dr_field[2].y;
                                                break;
                                            }
                                            if(targetFieldX < playerFieldX && canWest) {
                                                player_data[player].dr_dir[4] = PLAYER_DIR_WEST;
                                                player_data[player].dr_dir[5] = -1;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                            if(targetFieldX > playerFieldX && canEast) {
                                                player_data[player].dr_dir[4] = PLAYER_DIR_EAST;
                                                player_data[player].dr_dir[5] = -1;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                        } while(canNorth);
                                    } else if(targetFieldY > playerFieldY && canSouth) {
                                        player_data[player].dr_dir[3] = PLAYER_DIR_SOUTH;
                                        player_data[player].dr_dir[4] = -1;
                                        do {
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(!canSouth) {
                                                playerFieldY = player_data[player].dr_field[2].y;
                                                break;
                                            }
                                            playerFieldY += 2;
                                            if(playerFieldY > MAZE_MAX_SIZE-1) {
                                                playerFieldY = player_data[player].dr_field[2].y;
                                                break;
                                            }
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(drone_isTargetIsVisibleSouth(player, playerFieldY, playerFieldX)) {
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                            if(!canEast && !canWest && !canSouth) { /* dead end? */
                                                playerFieldY = player_data[player].dr_field[2].y;
                                                break;
                                            }
                                            if(targetFieldX < playerFieldX && canWest) {
                                                player_data[player].dr_dir[4] = PLAYER_DIR_WEST;
                                                player_data[player].dr_dir[5] = -1;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                            if(targetFieldX > playerFieldX && canEast) {
                                                player_data[player].dr_dir[4] = PLAYER_DIR_EAST;
                                                player_data[player].dr_dir[5] = -1;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                        } while(canSouth);
                                    }
                                }
                            } while(canEast);
                            player_data[player].dr_dir[0] = 0;
                            player_data[player].dr_fieldIndex = 0;
                            break;
                        }
                    } while(canNorth);
                    player_data[player].dr_dir[0] = 0;
                    player_data[player].dr_fieldIndex = 0;
                    return NO;
                }
            } while(canWest);
        }
        player_data[player].dr_dir[0] = 0;
        player_data[player].dr_fieldIndex = 0;
        return NO;
    }
    if(wantedDirChar == 's') {
        if(canWest) {
            player_data[player].dr_dir[0] = PLAYER_DIR_WEST;
            do {
                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                if(!canWest) break;
                playerFieldX -= 2;
                if(playerFieldX <= 0) {
                    playerFieldX = (player_data[player].ply_x >> MAZE_FIELD_SHIFT)|1; /* reset player position and exit */
                    break;
                }
                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                if(!canSouth && !canNorth && !canWest) { /* dead end? */
                    playerFieldX = (player_data[player].ply_x >> MAZE_FIELD_SHIFT)|1; /* reset player position and exit */
                    break;
                }
                if(canSouth) {
                    player_data[player].dr_field[0].y = playerFieldY;
                    player_data[player].dr_field[0].x = playerFieldX;
                    player_data[player].dr_dir[1] = PLAYER_DIR_SOUTH;
                    do {
                        drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                        if(!canSouth) {
                            playerFieldY = player_data[player].dr_field[0].y;
                            break;
                        }
                        playerFieldY += 2;
                        if(playerFieldY > MAZE_MAX_SIZE-1) {
                            playerFieldY = player_data[player].dr_field[0].y;
                            break;
                        }
                        drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                        if(!canWest && !canSouth && !canEast) { /* dead end? */
                            playerFieldY = player_data[player].dr_field[0].y;
                            break;
                        }
                        if(canSouth && drone_isTargetIsVisibleSouth(player, playerFieldY, playerFieldX)) {
                            player_data[player].dr_field[1].y = playerFieldY;
                            player_data[player].dr_field[1].x = playerFieldX;
                            player_data[player].dr_field[2].y = 0;
                            player_data[player].dr_dir[2] = -1;
                            return YES;
                        }
                        if(canEast) {
                            player_data[player].dr_field[1].y = playerFieldY;
                            player_data[player].dr_field[1].x = playerFieldX;
                            player_data[player].dr_field[2].y = 0;
                            player_data[player].dr_dir[2] = PLAYER_DIR_EAST;
                            player_data[player].dr_dir[3] = -1;
                            if(drone_isTargetIsVisibleEast(player, playerFieldY, playerFieldX)) {
                                player_data[player].dr_field[2].y = playerFieldY;
                                player_data[player].dr_field[2].x = playerFieldX+2;
                                player_data[player].dr_field[3].y = 0;
                                return YES;
                            }
                            do {
                                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                if(!canEast) {
                                    playerFieldX = player_data[player].dr_field[1].x;
                                    break;
                                }
                                playerFieldX += 2;
                                if(playerFieldX > MAZE_MAX_SIZE-1) {
                                    playerFieldX = player_data[player].dr_field[1].x;
                                    break;
                                }
                                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                if(!canNorth && !canSouth && !canEast) { /* dead end? */
                                    playerFieldX = player_data[player].dr_field[1].x;
                                    break;
                                }
                                if(!canEast && targetFieldY < playerFieldY && !canNorth) {
                                    playerFieldX = player_data[player].dr_field[1].x;
                                    break;
                                }
                                if(!canEast && targetFieldY > playerFieldY && !canSouth) {
                                    playerFieldX = player_data[player].dr_field[1].x;
                                    break;
                                }
                                if(canNorth || canSouth) {
                                    player_data[player].dr_field[2].y = playerFieldY;
                                    player_data[player].dr_field[2].x = playerFieldX;
                                    player_data[player].dr_field[3].y = 0;
                                    if(targetFieldY < playerFieldY && canNorth) {
                                        player_data[player].dr_dir[3] = PLAYER_DIR_NORTH+256;
                                        player_data[player].dr_dir[4] = -1;
                                        do {
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(!canNorth) {
                                                playerFieldY = player_data[player].dr_field[2].y;
                                                break;
                                            }
                                            playerFieldY -= 2;
                                            if(playerFieldY < 0) {
                                                playerFieldY = player_data[player].dr_field[2].y;
                                                break;
                                            }
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(drone_isTargetIsVisibleNorth(player, playerFieldY, playerFieldX)) {
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                            if(!canWest && !canNorth && !canEast) { /* dead end? */
                                                playerFieldY = player_data[player].dr_field[2].y;
                                                break;
                                            }
                                            if(targetFieldX < playerFieldX && canWest) {
                                                player_data[player].dr_dir[4] = PLAYER_DIR_WEST;
                                                player_data[player].dr_dir[5] = -1;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[4].y = 0;
                                            } else {
                                                if(targetFieldX > playerFieldX && canEast) {
                                                    player_data[player].dr_dir[4] = PLAYER_DIR_EAST;
                                                    player_data[player].dr_dir[5] = -1;
                                                    player_data[player].dr_field[3].x = playerFieldX;
                                                    player_data[player].dr_field[3].y = playerFieldY;
                                                    player_data[player].dr_field[4].y = 0;
                                                    return YES;
                                                }
                                            }
                                        } while(canNorth);
                                    } else if(targetFieldY > playerFieldY && canSouth) {
                                        player_data[player].dr_dir[3] = PLAYER_DIR_SOUTH;
                                        player_data[player].dr_dir[4] = -1;
                                        do {
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(!canSouth) {
                                                playerFieldY = player_data[player].dr_field[2].y;
                                                break;
                                            }
                                            playerFieldY += 2;
                                            if(playerFieldY > MAZE_MAX_SIZE-1) {
                                                playerFieldY = player_data[player].dr_field[2].y;
                                                break;
                                            }
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(drone_isTargetIsVisibleSouth(player, playerFieldY, playerFieldX)) {
                                                player_data[player].dr_field[4].y = playerFieldY;
                                                player_data[player].dr_field[4].x = playerFieldX;
                                                player_data[player].dr_field[5].y = 0;
                                                return YES;
                                            }
                                            if(!canEast && !canWest && !canSouth) { /* dead end? */
                                                playerFieldY = player_data[player].dr_field[2].x;
                                                break;
                                            }
                                            if(targetFieldX < playerFieldX && canWest) {
                                                player_data[player].dr_dir[4] = PLAYER_DIR_WEST;
                                                player_data[player].dr_dir[5] = -1;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                            if(targetFieldX > playerFieldX && canEast) {
                                                player_data[player].dr_dir[4] = PLAYER_DIR_EAST;
                                                player_data[player].dr_dir[5] = -1;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                        } while(canSouth);
                                    }
                                }
                            } while(canEast);
                            player_data[player].dr_dir[0] = 0;
                            player_data[player].dr_fieldIndex = 0;
                            break;
                        }
                    } while(canSouth);
                    player_data[player].dr_dir[0] = 0;
                    player_data[player].dr_fieldIndex = 0;
                    break;
                }
            } while(canWest);
            player_data[player].dr_dir[0] = 0;
            player_data[player].dr_fieldIndex = 0;
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 0, 0, 0);
            playerFieldY = (player_data[player].ply_y >> MAZE_FIELD_SHIFT)|1; /* reset player position */
            playerFieldX = (player_data[player].ply_x >> MAZE_FIELD_SHIFT)|1;
        }
        if(canEast) {
            player_data[player].dr_dir[0] = PLAYER_DIR_EAST;
            do {
                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                if(!canEast) break;
                playerFieldX += 2;
                if(playerFieldX > MAZE_MAX_SIZE-1) break;
                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                if(!canNorth && !canEast && !canSouth) break; /* dead end? */
                if(canSouth) {
                    player_data[player].dr_field[0].y = playerFieldY;
                    player_data[player].dr_field[0].x = playerFieldX;
                    player_data[player].dr_dir[1] = PLAYER_DIR_SOUTH;
                    do {
                        drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                        if(!canSouth) {
                            playerFieldY = player_data[player].dr_field[0].y;
                            break;
                        }
                        playerFieldY += 2;
                        if(playerFieldY > MAZE_MAX_SIZE-1) {
                            playerFieldY = player_data[player].dr_field[0].y;
                            break;
                        }
                        drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                        if(!canWest && !canSouth && !canEast) { /* dead end? */
                            playerFieldY = player_data[player].dr_field[0].y;
                            break;
                        }
                        if(canSouth && drone_isTargetIsVisibleSouth(player, playerFieldY, playerFieldX)) {
                            player_data[player].dr_field[1].y = playerFieldY;
                            player_data[player].dr_field[1].x = playerFieldX;
                            player_data[player].dr_field[2].y = 0;
                            player_data[player].dr_dir[2] = -1;
                            return YES;
                        }
                        if(canWest) {
                            player_data[player].dr_field[1].y = playerFieldY;
                            player_data[player].dr_field[1].x = playerFieldX;
                            player_data[player].dr_field[2].y = 0;
                            player_data[player].dr_dir[2] = PLAYER_DIR_WEST;
                            player_data[player].dr_dir[3] = -1;
                            if(drone_isTargetIsVisibleWest(player, playerFieldY, playerFieldX)) {
                                player_data[player].dr_field[2].y = playerFieldY;
                                player_data[player].dr_field[2].x = playerFieldX-2;
                                player_data[player].dr_field[3].y = 0;
                                return YES;
                            }
                            do {
                                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                if(!canWest) {
                                    playerFieldX = player_data[player].dr_field[1].x;
                                    break;
                                }
                                playerFieldX -= 2;
                                if(playerFieldX <= 0) {
                                    playerFieldX = player_data[player].dr_field[1].x;
                                    break;
                                }
                                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                if(!canSouth && !canNorth && !canWest) { /* dead end? */
                                    playerFieldX = player_data[player].dr_field[1].x;
                                    break;
                                }
                                if(!canNorth && targetFieldX < playerFieldX && !canWest) {
                                    playerFieldX = player_data[player].dr_field[1].x;
                                    break;
                                }
                                if(!canSouth && targetFieldX > playerFieldX && !canWest) {
                                    playerFieldX = player_data[player].dr_field[1].x;
                                    break;
                                }
                                if(canNorth || canSouth) {
                                    player_data[player].dr_field[2].y = playerFieldY;
                                    player_data[player].dr_field[2].x = playerFieldX;
                                    player_data[player].dr_field[3].y = 0;
                                    if(targetFieldY < playerFieldY && canNorth) {
                                        player_data[player].dr_dir[3] = PLAYER_DIR_NORTH+256;
                                        player_data[player].dr_dir[4] = -1;
                                        do {
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(!canNorth) {
                                                playerFieldY = player_data[player].dr_field[2].y;
                                                break;
                                            }
                                            playerFieldY -= 2;
                                            if(playerFieldY <= 0) {
                                                playerFieldY = player_data[player].dr_field[2].y;
                                                break;
                                            }
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(drone_isTargetIsVisibleNorth(player, playerFieldY, playerFieldX)) {
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                            if(!canEast && !canNorth && !canWest) { /* dead end? */
                                                playerFieldY = player_data[player].dr_field[2].y;
                                                break;
                                            }
                                            if(targetFieldX < playerFieldX && canWest) {
                                                player_data[player].dr_dir[4] = PLAYER_DIR_WEST;
                                                player_data[player].dr_dir[5] = -1;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                            if(targetFieldX > playerFieldX && canEast) {
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[4].y = 0;
                                                player_data[player].dr_dir[3] = PLAYER_DIR_EAST;
                                                player_data[player].dr_dir[4] = -1;
                                                return YES;
                                            }
                                        } while(canNorth);
                                    } else if(targetFieldY > playerFieldY && canSouth) {
                                        player_data[player].dr_dir[3] = PLAYER_DIR_SOUTH;
                                        player_data[player].dr_dir[4] = -1;
                                        do {
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(!canSouth) {
                                                playerFieldY = player_data[player].dr_field[2].y;
                                                break;
                                            }
                                            playerFieldY += 2;
                                            if(playerFieldY > MAZE_MAX_SIZE-1) {
                                                playerFieldY = player_data[player].dr_field[2].y;
                                                break;
                                            }
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(drone_isTargetIsVisibleSouth(player, playerFieldY, playerFieldX)) {
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                            if(!canEast && !canWest && !canSouth) { /* dead end? */
                                                playerFieldY = player_data[player].dr_field[2].y;
                                                break;
                                            }
                                            if(targetFieldX < playerFieldX && canWest) {
                                                player_data[player].dr_dir[4] = PLAYER_DIR_WEST;
                                                player_data[player].dr_dir[5] = -1;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                            if(targetFieldX > playerFieldX && canEast) {
                                                player_data[player].dr_dir[4] = PLAYER_DIR_EAST;
                                                player_data[player].dr_dir[5] = -1;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                        } while(canSouth);
                                    }
                                }
                            } while(canEast);
                            player_data[player].dr_dir[0] = 0;
                            player_data[player].dr_fieldIndex = 0;
                            break;
                        }
                    } while(canSouth);
                    player_data[player].dr_dir[0] = 0;
                    player_data[player].dr_fieldIndex = 0;
                    return NO;
                }
            } while(canWest);
        }
        player_data[player].dr_dir[0] = 0;
        player_data[player].dr_fieldIndex = 0;
        return NO;
    }
    if(wantedDirChar == 'w') {
        if(canSouth) {
            player_data[player].dr_dir[0] = PLAYER_DIR_SOUTH;
            do {
                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                if(!canSouth) break;
                playerFieldY += 2;
                if(playerFieldY > MAZE_MAX_SIZE-1) {
                    playerFieldY = (player_data[player].ply_y >> MAZE_FIELD_SHIFT)|1;
                    break;
                }
                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                if(!canSouth && !canEast && !canWest) { /* dead end? */
                    playerFieldY = (player_data[player].ply_y >> MAZE_FIELD_SHIFT)|1;
                    break;
                }
                if(canWest) {
                    player_data[player].dr_field[0].y = playerFieldY;
                    player_data[player].dr_field[0].x = playerFieldX;
                    player_data[player].dr_dir[1] = PLAYER_DIR_WEST;
                    do {
                        drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                        if(!canWest) {
                            playerFieldX = player_data[player].dr_field[0].x;
                            break;
                        }
                        playerFieldX -= 2;
                        if(playerFieldX <= 0) {
                            playerFieldX = player_data[player].dr_field[0].x;
                            break;
                        }
                        drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                        if(!canWest && !canNorth && !canSouth) { /* dead end? */
                            playerFieldX = player_data[player].dr_field[0].x;
                            break;
                        }
                        if(canWest && drone_isTargetIsVisibleWest(player, playerFieldY, playerFieldX)) {
                            player_data[player].dr_field[1].y = playerFieldY;
                            player_data[player].dr_field[1].x = playerFieldX;
                            player_data[player].dr_field[2].y = 0;
                            player_data[player].dr_dir[2] = -1;
                            return YES;
                        }
                        if(canNorth) {
                            player_data[player].dr_field[1].y = playerFieldY;
                            player_data[player].dr_field[1].x = playerFieldX;
                            player_data[player].dr_field[2].y = 0;
                            player_data[player].dr_dir[2] = PLAYER_DIR_NORTH+256;
                            player_data[player].dr_dir[3] = -1;
                            if(drone_isTargetIsVisibleNorth(player, playerFieldY, playerFieldX)) {
                                player_data[player].dr_field[2].y = playerFieldY-2;
                                player_data[player].dr_field[2].x = playerFieldX;
                                player_data[player].dr_field[3].y = 0;
                                return YES;
                            }
                            do {
                                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                if(!canNorth) {
                                    playerFieldY = player_data[player].dr_field[1].y;
                                    break;
                                }
                                playerFieldY -= 2;
                                if(playerFieldY < 0) {
                                    playerFieldY = player_data[player].dr_field[0].y;
                                    break;
                                }
                                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                if(!canNorth && !canWest && !canEast) { /* dead end? */
                                    playerFieldY = player_data[player].dr_field[1].y;
                                    break;
                                }
                                if(!canNorth && targetFieldX < playerFieldX && !canWest) {
                                    playerFieldY = player_data[player].dr_field[1].y;
                                    break;
                                }
                                if(!canNorth && targetFieldX > playerFieldX && !canEast) {
                                    playerFieldY = player_data[player].dr_field[1].y;
                                    break;
                                }
                                if(canWest || canEast) {
                                    player_data[player].dr_field[2].y = playerFieldY;
                                    player_data[player].dr_field[2].x = playerFieldX;
                                    player_data[player].dr_field[3].y = 0;
                                    if(targetFieldX < playerFieldX && canWest) {
                                        player_data[player].dr_dir[3] = PLAYER_DIR_WEST;
                                        player_data[player].dr_dir[4] = -1;
                                        do {
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(!canWest) {
                                                playerFieldX = player_data[player].dr_field[2].x;
                                                break;
                                            }
                                            playerFieldX -= 2;
                                            if(playerFieldX <= 0) {
                                                playerFieldX = player_data[player].dr_field[2].x;
                                                break;
                                            }
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(drone_isTargetIsVisibleWest(player, playerFieldY, playerFieldX)) {
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                            if(!canWest && !canNorth && !canSouth) { /* dead end? */
                                                playerFieldX = player_data[player].dr_field[2].x;
                                                break;
                                            }
                                            if(targetFieldY < playerFieldY && canNorth) {
                                                player_data[player].dr_dir[4] = PLAYER_DIR_NORTH+256;
                                                player_data[player].dr_dir[5] = -1;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                            if(targetFieldY > playerFieldY && canSouth) {
                                                player_data[player].dr_dir[4] = PLAYER_DIR_SOUTH;
                                                player_data[player].dr_dir[5] = -1;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                        } while(canWest);
                                    } else if(targetFieldX > playerFieldX && canEast) {
                                        player_data[player].dr_dir[3] = PLAYER_DIR_EAST;
                                        player_data[player].dr_dir[4] = -1;
                                        do {
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(!canEast) {
                                                playerFieldX = player_data[player].dr_field[2].x;
                                                break;
                                            }
                                            playerFieldX += 2;
                                            if(playerFieldX > MAZE_MAX_SIZE-1) {
                                                playerFieldX = player_data[player].dr_field[2].x;
                                                break;
                                            }
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(drone_isTargetIsVisibleEast(player, playerFieldY, playerFieldX)) {
                                                player_data[player].dr_field[4].y = playerFieldY;
                                                player_data[player].dr_field[4].x = playerFieldX;
                                                player_data[player].dr_field[5].y = 0;
                                                return YES;
                                            }
                                            if(!canEast && !canNorth && !canSouth) { /* dead end? */
                                                playerFieldX = player_data[player].dr_field[2].x;
                                                break;
                                            }
                                            if(targetFieldY < playerFieldY && canNorth) {
                                                player_data[player].dr_dir[4] = PLAYER_DIR_NORTH+256;
                                                player_data[player].dr_dir[5] = -1;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                            if(targetFieldY > playerFieldY && canSouth) {
                                                player_data[player].dr_dir[4] = PLAYER_DIR_SOUTH;
                                                player_data[player].dr_dir[5] = -1;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                        } while(canEast);
                                    }
                                }
                            } while(canNorth);
                            player_data[player].dr_dir[0] = 0;
                            player_data[player].dr_fieldIndex = 0;
                            break;
                        }
                    } while(canWest);
                    player_data[player].dr_dir[0] = 0;
                    player_data[player].dr_fieldIndex = 0;
                    break;
                }
            } while(canSouth);
            player_data[player].dr_dir[0] = 0;
            player_data[player].dr_fieldIndex = 0;
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 0, 0, 0);
            playerFieldY = (player_data[player].ply_y >> MAZE_FIELD_SHIFT)|1; /* reset player position */
            playerFieldX = (player_data[player].ply_x >> MAZE_FIELD_SHIFT)|1;
        }
        if(canNorth) {
            player_data[player].dr_dir[0] = PLAYER_DIR_NORTH+256;
            do {
                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                if(!canNorth) break;
                playerFieldY -= 2;
                if(playerFieldY < 0) break;
                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                if(!canNorth && !canEast && !canWest) break; /* dead end? */
                if(canWest) {
                    player_data[player].dr_field[0].y = playerFieldY;
                    player_data[player].dr_field[0].x = playerFieldX;
                    player_data[player].dr_dir[1] = PLAYER_DIR_WEST;
                    player_data[player].dr_dir[2] = -1;
                    do {
                        drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                        if(!canWest) {
                            playerFieldX = player_data[player].dr_field[0].x;
                            break;
                        }
                        playerFieldX -= 2;
                        if(playerFieldX <= 0) {
                            playerFieldX = player_data[player].dr_field[0].x;
                            break;
                        }
                        drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                        if(!canWest && !canNorth && !canSouth) { /* dead end? */
                            playerFieldX = player_data[player].dr_field[0].x;
                            break;
                        }
                        if(canWest && drone_isTargetIsVisibleWest(player, playerFieldY, playerFieldX)) {
                            player_data[player].dr_field[1].y = playerFieldY;
                            player_data[player].dr_field[1].x = playerFieldX;
                            player_data[player].dr_field[2].y = 0;
                            player_data[player].dr_dir[2] = -1;
                            return YES;
                        }
                        if(canSouth) {
                            player_data[player].dr_field[1].y = playerFieldY;
                            player_data[player].dr_field[1].x = playerFieldX;
                            player_data[player].dr_field[2].y = 0;
                            player_data[player].dr_dir[2] = PLAYER_DIR_SOUTH;
                            player_data[player].dr_dir[3] = -1;
                            if(drone_isTargetIsVisibleSouth(player, playerFieldY, playerFieldX)) {
                                player_data[player].dr_field[2].y = playerFieldY+2;
                                player_data[player].dr_field[2].x = playerFieldX;
                                player_data[player].dr_field[3].y = 0;
                                return YES;
                            }
                            do {
                                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                if(!canSouth) {
                                    playerFieldY = player_data[player].dr_field[0].y;
                                    break;
                                }
                                playerFieldY += 2;
                                if(playerFieldY > MAZE_MAX_SIZE-1) {
                                    playerFieldY = player_data[player].dr_field[0].y;
                                    break;
                                }
                                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                if(!canSouth && !canEast && !canWest) { /* dead end? */
                                    playerFieldY = player_data[player].dr_field[1].y;
                                    break;
                                }
                                if(!canSouth && targetFieldX < playerFieldX && !canWest) {
                                    playerFieldY = player_data[player].dr_field[1].y;
                                    break;
                                }
                                if(!canSouth && targetFieldX > playerFieldX && !canEast) {
                                    playerFieldY = player_data[player].dr_field[1].y;
                                    break;
                                }
                                if(canWest || canEast) {
                                    player_data[player].dr_field[2].y = playerFieldY;
                                    player_data[player].dr_field[2].x = playerFieldX;
                                    player_data[player].dr_field[3].y = 0;
                                    if(targetFieldX < playerFieldX && canWest) {
                                        player_data[player].dr_dir[3] = PLAYER_DIR_WEST;
                                        player_data[player].dr_dir[4] = -1;
                                        do {
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(!canWest) {
                                                playerFieldX = player_data[player].dr_field[2].x;
                                                break;
                                            }
                                            playerFieldX -= 2;
                                            if(playerFieldX <= 0) {
                                                playerFieldX = player_data[player].dr_field[2].x;
                                                break;
                                            }
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(drone_isTargetIsVisibleWest(player, playerFieldY, playerFieldX)) {
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                            if(!canWest && !canNorth && !canSouth) { /* dead end? */
                                                playerFieldX = player_data[player].dr_field[2].x;
                                                break;
                                            }
                                            if(targetFieldY < playerFieldY && canNorth) {
                                                player_data[player].dr_dir[4] = PLAYER_DIR_NORTH+256;
                                                player_data[player].dr_dir[5] = -1;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                            if(targetFieldY > playerFieldY && canSouth) {
                                                player_data[player].dr_dir[4] = PLAYER_DIR_SOUTH;
                                                player_data[player].dr_dir[5] = -1;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                        } while(canWest);
                                    } else if(targetFieldX > playerFieldX && canEast) {
                                        player_data[player].dr_dir[3] = PLAYER_DIR_EAST;
                                        player_data[player].dr_dir[4] = -1;
                                        do {
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(!canEast) {
                                                playerFieldX = player_data[player].dr_field[2].x;
                                                break;
                                            }
                                            playerFieldX += 2;
                                            if(playerFieldX > MAZE_MAX_SIZE-1) {
                                                playerFieldX = player_data[player].dr_field[2].x;
                                                break;
                                            }
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(drone_isTargetIsVisibleEast(player, playerFieldY, playerFieldX)) {
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                            if(!canEast && !canNorth && !canSouth) { /* dead end? */
                                                playerFieldX = player_data[player].dr_field[2].x;
                                                break;
                                            }
                                            if(targetFieldY < playerFieldY && canNorth) {
                                                player_data[player].dr_dir[4] = PLAYER_DIR_NORTH+256;
                                                player_data[player].dr_dir[5] = -1;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                            if(targetFieldY > playerFieldY && canSouth) {
                                                player_data[player].dr_dir[4] = PLAYER_DIR_SOUTH;
                                                player_data[player].dr_dir[5] = -1;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                        } while(canEast);
                                    }
                                }
                            } while(canSouth);
                            player_data[player].dr_dir[0] = 0;
                            player_data[player].dr_fieldIndex = 0;
                            break;
                        }
                    } while(canWest);
                    player_data[player].dr_dir[0] = 0;
                    player_data[player].dr_fieldIndex = 0;
                    return NO;
                }
            } while(canNorth);
        }
        player_data[player].dr_dir[0] = 0;
        player_data[player].dr_fieldIndex = 0;
        return NO;
    }
    if(wantedDirChar == 'e') {
        if(canSouth) {
            player_data[player].dr_dir[0] = PLAYER_DIR_SOUTH;
            player_data[player].dr_dir[1] = -1;
            do {
                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                if(!canSouth) break;
                playerFieldY += 2;
                if(playerFieldY > MAZE_MAX_SIZE-1) {
                    playerFieldY = (player_data[player].ply_y >> MAZE_FIELD_SHIFT)|1;
                    break;
                }
                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                if(!canSouth && !canEast && !canWest) { /* dead end? */
                    playerFieldY = (player_data[player].ply_y >> MAZE_FIELD_SHIFT)|1;
                    break;
                }
                if(canEast) {
                    player_data[player].dr_field[0].y = playerFieldY;
                    player_data[player].dr_field[0].x = playerFieldX;
                    player_data[player].dr_dir[1] = PLAYER_DIR_EAST;
                    player_data[player].dr_dir[2] = -1;
                    do {
                        drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                        if(!canEast) {
                            playerFieldX = player_data[player].dr_field[0].x;
                            break;
                        }
                        playerFieldX += 2;
                        if(playerFieldX > MAZE_MAX_SIZE-1) {
                            playerFieldX = player_data[player].dr_field[0].x;
                            break;
                        }
                        drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                        if(!canEast && !canNorth && !canSouth) { /* dead end? */
                            playerFieldX = player_data[player].dr_field[0].x;
                            break;
                        }
                        if(canEast && drone_isTargetIsVisibleEast(player, playerFieldY, playerFieldX)) {
                            player_data[player].dr_field[1].y = playerFieldY;
                            player_data[player].dr_field[1].x = playerFieldX;
                            player_data[player].dr_field[2].y = 0;
                            player_data[player].dr_dir[2] = -1;
                            return YES;
                        }
                        if(canNorth) {
                            player_data[player].dr_field[1].y = playerFieldY;
                            player_data[player].dr_field[1].x = playerFieldX;
                            player_data[player].dr_field[2].y = 0;
                            player_data[player].dr_dir[2] = PLAYER_DIR_NORTH+256;
                            player_data[player].dr_dir[3] = -1;
                            if(drone_isTargetIsVisibleNorth(player, playerFieldY, playerFieldX)) {
                                player_data[player].dr_field[2].y = playerFieldY-2;
                                player_data[player].dr_field[2].x = playerFieldX;
                                player_data[player].dr_field[3].y = 0;
                                return YES;
                            }
                            do {
                                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                if(!canNorth) {
                                    playerFieldY = player_data[player].dr_field[1].y;
                                    break;
                                }
                                playerFieldY -= 2;
                                if(playerFieldY < 0) {
                                    playerFieldY = player_data[player].dr_field[1].y;
                                    break;
                                }
                                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                if(!canNorth && !canWest && !canEast) { /* dead end? */
                                    playerFieldY = player_data[player].dr_field[1].y;
                                    break;
                                }
                                if(!canNorth && targetFieldX < playerFieldX && !canWest) {
                                    playerFieldY = player_data[player].dr_field[1].y;
                                    break;
                                }
                                if(!canNorth && targetFieldX > playerFieldX && !canEast) {
                                    playerFieldY = player_data[player].dr_field[1].y;
                                    break;
                                }
                                if(canWest || canEast) {
                                    player_data[player].dr_field[2].y = playerFieldY;
                                    player_data[player].dr_field[2].x = playerFieldX;
                                    player_data[player].dr_field[3].y = 0;
                                    if(targetFieldX < playerFieldX && canWest) {
                                        player_data[player].dr_dir[3] = PLAYER_DIR_WEST;
                                        player_data[player].dr_dir[4] = -1;
                                        do {
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(!canWest) {
                                                playerFieldX = player_data[player].dr_field[2].x;
                                                break;
                                            }
                                            playerFieldX -= 2;
                                            if(playerFieldX <= 0) {
                                                playerFieldX = player_data[player].dr_field[2].x;
                                                break;
                                            }
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(drone_isTargetIsVisibleWest(player, playerFieldY, playerFieldX)) {
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                            if(!canWest && !canNorth && !canSouth) { /* dead end? */
                                                playerFieldX = player_data[player].dr_field[2].x;
                                                break;
                                            }
                                            if(targetFieldY < playerFieldY && canNorth) {
                                                player_data[player].dr_dir[4] = PLAYER_DIR_NORTH+256;
                                                player_data[player].dr_dir[5] = -1;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                            if(targetFieldY > playerFieldY && canSouth) {
                                                player_data[player].dr_dir[4] = PLAYER_DIR_SOUTH;
                                                player_data[player].dr_dir[5] = -1;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                        } while(canWest);
                                    } else if(targetFieldX > playerFieldX && canEast) {
                                        player_data[player].dr_dir[3] = PLAYER_DIR_EAST;
                                        player_data[player].dr_dir[4] = -1;
                                        do {
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(!canEast) {
                                                playerFieldX = player_data[player].dr_field[2].x;
                                                break;
                                            }
                                            playerFieldX += 2;
                                            if(playerFieldX > MAZE_MAX_SIZE-1) {
                                                playerFieldX = player_data[player].dr_field[2].x;
                                                break;
                                            }
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(drone_isTargetIsVisibleEast(player, playerFieldY, playerFieldX)) {
                                                player_data[player].dr_field[4].y = playerFieldY;
                                                player_data[player].dr_field[4].x = playerFieldX;
                                                player_data[player].dr_field[5].y = 0;
                                                return YES;
                                            }
                                            if(!canEast && !canNorth && !canSouth) { /* dead end? */
                                                playerFieldX = player_data[player].dr_field[2].x;
                                                break;
                                            }
                                            if(targetFieldY < playerFieldY && canNorth) {
                                                player_data[player].dr_dir[4] = PLAYER_DIR_NORTH+256;
                                                player_data[player].dr_dir[5] = -1;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                            if(targetFieldY > playerFieldY && canSouth) {
                                                player_data[player].dr_dir[4] = PLAYER_DIR_SOUTH;
                                                player_data[player].dr_dir[5] = -1;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                        } while(canEast);
                                    }
                                }
                            } while(canNorth);
                            player_data[player].dr_dir[0] = 0;
                            player_data[player].dr_fieldIndex = 0;
                            break;
                        }
                    } while(canEast);
                    player_data[player].dr_dir[0] = 0;
                    player_data[player].dr_fieldIndex = 0;
                    break;
                }
            } while(canSouth);
            player_data[player].dr_dir[0] = 0;
            player_data[player].dr_fieldIndex = 0;
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 0, 0, 0);
            playerFieldY = (player_data[player].ply_y >> MAZE_FIELD_SHIFT)|1; /* reset player position */
            playerFieldX = (player_data[player].ply_x >> MAZE_FIELD_SHIFT)|1;
        }
        if(canNorth) {
            player_data[player].dr_dir[0] = PLAYER_DIR_NORTH+256;
            do {
                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                if(!canNorth) break;
                playerFieldY -= 2;
                if(playerFieldY < 0) break;
                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                if(canEast) {
                    player_data[player].dr_field[0].y = playerFieldY;
                    player_data[player].dr_field[0].x = playerFieldX;
                    player_data[player].dr_dir[1] = PLAYER_DIR_EAST;
                    do {
                        drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                        if(!canEast) {
                            playerFieldX = player_data[player].dr_field[0].x;
                            break;
                        }
                        playerFieldX += 2;
                        if(playerFieldX > MAZE_MAX_SIZE-1) {
                            playerFieldX = player_data[player].dr_field[0].x;
                            break;
                        }
                        drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                        if(!canEast && !canNorth && !canSouth) { /* dead end? */
                            playerFieldX = player_data[player].dr_field[0].x;
                            break;
                        }
                        if(canEast && drone_isTargetIsVisibleEast(player, playerFieldY, playerFieldX)) {
                            player_data[player].dr_field[1].y = playerFieldY;
                            player_data[player].dr_field[1].x = playerFieldX;
                            player_data[player].dr_field[2].y = 0;
                            player_data[player].dr_dir[2] = -1;
                            return YES;
                        }
                        if(canSouth) {
                            player_data[player].dr_field[1].y = playerFieldY;
                            player_data[player].dr_field[1].x = playerFieldX;
                            player_data[player].dr_field[2].y = 0;
                            player_data[player].dr_dir[2] = PLAYER_DIR_SOUTH;
                            player_data[player].dr_dir[3] = -1;
                            do {
                                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                if(!canSouth) {
                                    playerFieldY = player_data[player].dr_field[1].y;
                                    break;
                                }
                                playerFieldY += 2;
                                if(playerFieldY > MAZE_MAX_SIZE-1) {
                                    playerFieldY = player_data[player].dr_field[0].y;
                                    break;
                                }
                                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                if(canSouth && drone_isTargetIsVisibleSouth(player, playerFieldY, playerFieldX)) {
                                    player_data[player].dr_field[2].y = playerFieldY+2;
                                    player_data[player].dr_field[2].x = playerFieldX;
                                    player_data[player].dr_field[3].y = 0;
                                    return YES;
                                }
                                if(!canSouth && !canEast && !canWest) { /* dead end? */
                                    playerFieldY = player_data[player].dr_field[1].y;
                                    break;
                                }
                                if(!canSouth && targetFieldX < playerFieldX && !canWest) {
                                    playerFieldY = player_data[player].dr_field[1].y;
                                    break;
                                }
                                if(!canSouth && targetFieldX > playerFieldX && !canEast) {
                                    playerFieldY = player_data[player].dr_field[1].y;
                                    break;
                                }
                                if(canWest || canEast) {
                                    player_data[player].dr_field[2].y = playerFieldY;
                                    player_data[player].dr_field[2].x = playerFieldX;
                                    player_data[player].dr_field[3].y = 0;
                                    if(targetFieldX < playerFieldX && canWest) {
                                        player_data[player].dr_dir[3] = PLAYER_DIR_WEST;
                                        player_data[player].dr_dir[4] = -1;
                                        do {
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(!canWest) {
                                                playerFieldX = player_data[player].dr_field[2].x;
                                                break;
                                            }
                                            playerFieldX -= 2;
                                            if(playerFieldX <= 0) {
                                                playerFieldX = player_data[player].dr_field[2].x;
                                                break;
                                            }
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(drone_isTargetIsVisibleWest(player, playerFieldY, playerFieldX)) {
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                            if(!canWest && !canNorth && !canSouth) { /* dead end? */
                                                playerFieldX = player_data[player].dr_field[2].x;
                                                break;
                                            }
                                            if(targetFieldY < playerFieldY && canNorth) {
                                                player_data[player].dr_dir[4] = PLAYER_DIR_NORTH+256;
                                                player_data[player].dr_dir[5] = -1;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                            if(targetFieldY > playerFieldY && canSouth) {
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[4].y = 0;
                                                player_data[player].dr_dir[3] = PLAYER_DIR_SOUTH;
                                                player_data[player].dr_dir[4] = -1;
                                                return YES;
                                            }
                                        } while(canWest);
                                    } else if(targetFieldX > playerFieldX && canEast) {
                                        player_data[player].dr_dir[3] = PLAYER_DIR_EAST;
                                        player_data[player].dr_dir[4] = -1;
                                        do {
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(!canEast) {
                                                playerFieldX = player_data[player].dr_field[2].x;
                                                break;
                                            }
                                            playerFieldX += 2;
                                            if(playerFieldX > MAZE_MAX_SIZE-1) {
                                                playerFieldX = player_data[player].dr_field[2].x;
                                                break;
                                            }
                                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                            if(drone_isTargetIsVisibleEast(player, playerFieldY, playerFieldX)) {
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                            if(!canEast && !canNorth && !canSouth) { /* dead end? */
                                                playerFieldX = player_data[player].dr_field[2].x;
                                                break;
                                            }
                                            if(targetFieldY < playerFieldY && canNorth) {
                                                player_data[player].dr_dir[4] = PLAYER_DIR_NORTH+256;
                                                player_data[player].dr_dir[5] = -1;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                            if(targetFieldY > playerFieldY && canSouth) {
                                                player_data[player].dr_dir[4] = PLAYER_DIR_SOUTH;
                                                player_data[player].dr_dir[5] = -1;
                                                player_data[player].dr_field[3].y = playerFieldY;
                                                player_data[player].dr_field[3].x = playerFieldX;
                                                player_data[player].dr_field[4].y = 0;
                                                return YES;
                                            }
                                        } while(canEast);
                                    }
                                }
                            } while(canSouth);
                            player_data[player].dr_dir[0] = 0;
                            player_data[player].dr_fieldIndex = 0;
                            break;
                        }
                    } while(canEast);
                    player_data[player].dr_dir[0] = 0;
                    player_data[player].dr_fieldIndex = 0;
                    return NO;
                }
            } while(canNorth);
        }
        player_data[player].dr_dir[0] = 0;
        player_data[player].dr_fieldIndex = 0;
        return NO;
    }
    player_data[player].dr_dir[0] = 0;
    player_data[player].dr_fieldIndex = 0;
    return NO;
}

/************************************************************
 *** int drone_sub_ninja_north(int player,int viewCompassDirChar)
 ************************************************************/
int drone_sub_ninja_north(int player,int wantedDirChar) {
int target_player;
int targetPlayerFieldX;
int playerFieldX;
int playerFieldY;
int canWest;
int canEast;
int canSouth;
int canNorth;

    player_data[player].dr_fieldIndex = 0;
    player_data[player].dr_upRotationCounter = 0;
    player_data[player].dr_rotateCounter = 0;
    player_data[player].dr_dir[0] = 0;
    player_data[player].dr_field[0].y = 0;
    target_player = player_data[player].dr_currentTarget;
    playerFieldY = (player_data[player].ply_y >> MAZE_FIELD_SHIFT)|1;
    playerFieldX = (player_data[player].ply_x >> MAZE_FIELD_SHIFT)|1;
    targetPlayerFieldX = (player_data[target_player].ply_x >> MAZE_FIELD_SHIFT)|1;
    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 0, 0, 0);
    if(wantedDirChar == 'e' && !canEast && !canNorth && canWest) {
        do {
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(!canWest) break;
            playerFieldX -= 2;
            if(playerFieldX <= 0) break;
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(canNorth) {
                player_data[player].dr_field[0].y = playerFieldY;
                player_data[player].dr_field[0].x = playerFieldX;
                do {
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canNorth) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    playerFieldY -= 2;
                    if(playerFieldY <= 0) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canNorth && !canEast) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    if(canEast) {
                        player_data[player].dr_field[1].y = playerFieldY;
                        player_data[player].dr_field[1].x = playerFieldX;
                        do {
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canEast) {
                                playerFieldX = player_data[player].dr_field[1].x;
                                break;
                            }
                            playerFieldX += 2;
                            if(playerFieldX > MAZE_MAX_SIZE-1) {
                                playerFieldX = player_data[player].dr_field[1].x;
                                break;
                            }
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canSouth && !canEast) {
                                playerFieldX = player_data[player].dr_field[1].x;
                                break;
                            }
                            if(canSouth) {
                                player_data[player].dr_field[2].y = playerFieldY;
                                player_data[player].dr_field[2].x = playerFieldX;
                                player_data[player].dr_field[3].y = playerFieldY+2;
                                player_data[player].dr_field[3].x = playerFieldX;
                                player_data[player].dr_field[4].y = 0;
                                player_data[player].dr_dir[0] = PLAYER_DIR_WEST;
                                player_data[player].dr_dir[1] = PLAYER_DIR_NORTH+256;
                                player_data[player].dr_dir[2] = PLAYER_DIR_EAST;
                                player_data[player].dr_dir[3] = PLAYER_DIR_SOUTH;
                                player_data[player].dr_dir[4] = -1;
                                player_data[player].dr_fieldIndex = 0;
                                return YES;
                            }
                        } while(canEast);
                        player_data[player].dr_dir[0] = 0;
                        player_data[player].dr_fieldIndex = 0;
                        return NO;
                    }
                } while(canNorth);
                player_data[player].dr_dir[0] = 0;
                player_data[player].dr_fieldIndex = 0;
                return NO;
            }
        } while(canWest);
        player_data[player].dr_dir[0] = 0;
        player_data[player].dr_fieldIndex = 0;
        return NO;
    }
    if(wantedDirChar == 'w' && !canWest && !canNorth && canEast) {
        do {
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(!canEast) break;
            playerFieldX += 2;
            if(playerFieldX > MAZE_MAX_SIZE-1) break;
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(canNorth) {
                player_data[player].dr_field[0].y = playerFieldY;
                player_data[player].dr_field[0].x = playerFieldX;
                do {
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canNorth) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    playerFieldY -= 2;
                    if(playerFieldY <= 0) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canNorth && !canWest) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    if(canWest) {
                        player_data[player].dr_field[1].y = playerFieldY;
                        player_data[player].dr_field[1].x = playerFieldX;
                        do {
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canWest) {
                                playerFieldX = player_data[player].dr_field[1].x;
                                break;
                            }
                            playerFieldX -= 2;
                            if(playerFieldX < 0) {
                                playerFieldX = player_data[player].dr_field[1].x;
                                break;
                            }
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canWest && !canSouth) {
                                playerFieldX = player_data[player].dr_field[1].x;
                                break;
                            }
                            if(canSouth) {
                                player_data[player].dr_field[2].y = playerFieldY;
                                player_data[player].dr_field[2].x = playerFieldX;
                                player_data[player].dr_field[3].y = playerFieldY+2;
                                player_data[player].dr_field[3].x = playerFieldX;
                                player_data[player].dr_field[4].y = 0;
                                player_data[player].dr_dir[0] = PLAYER_DIR_EAST;
                                player_data[player].dr_dir[1] = PLAYER_DIR_NORTH+256;
                                player_data[player].dr_dir[2] = PLAYER_DIR_WEST;
                                player_data[player].dr_dir[3] = PLAYER_DIR_SOUTH;
                                player_data[player].dr_dir[4] = -1;
                                player_data[player].dr_fieldIndex = 0;
                                return YES;
                            }
                        } while(canWest);
                        player_data[player].dr_dir[0] = 0;
                        player_data[player].dr_fieldIndex = 0;
                        return NO;
                    }
                } while(canNorth);
                player_data[player].dr_dir[0] = 0;
                player_data[player].dr_fieldIndex = 0;
                return NO;
            }
        } while(canEast);
        player_data[player].dr_dir[0] = 0;
        player_data[player].dr_fieldIndex = 0;
        return NO;
    }
    if(canNorth) {
        do {
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(!canNorth) break;
            playerFieldY -= 2;
            if(playerFieldY < 0) break;
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(wantedDirChar == 'w' && canWest) {
                player_data[player].dr_field[0].y = playerFieldY;
                player_data[player].dr_field[0].x = playerFieldX;
                do {
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canWest) {
                        playerFieldX = player_data[player].dr_field[0].x;
                        break;
                    }
                    playerFieldX -= 2;
                    if(playerFieldX <= 0) {
                        playerFieldX = player_data[player].dr_field[0].x;
                        break;
                    }
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canWest && !canSouth) {
                        playerFieldX = player_data[player].dr_field[0].x;
                        break;
                    }
                    if(canSouth) {
                        player_data[player].dr_field[1].y = playerFieldY;
                        player_data[player].dr_field[1].x = playerFieldX;
                        do {
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canSouth) {
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            playerFieldY += 2;
                            if(playerFieldY > MAZE_MAX_SIZE-1) {
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canSouth && !canEast && !canWest) { /* dead end? */
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            if(playerFieldX == targetPlayerFieldX || canSouth || canEast || canWest) {
                                player_data[player].dr_field[2].y = playerFieldY;
                                player_data[player].dr_field[2].x = playerFieldX;
                                player_data[player].dr_field[3].y = 0;
                                player_data[player].dr_dir[0] = PLAYER_DIR_NORTH+256;
                                player_data[player].dr_dir[1] = PLAYER_DIR_WEST;
                                player_data[player].dr_dir[2] = PLAYER_DIR_SOUTH;
                                player_data[player].dr_dir[3] = -1;
                                player_data[player].dr_fieldIndex = 0;
                                return YES;
                            }
                        } while(canSouth);
                    }
                } while(canWest);
            } else {
                if(wantedDirChar == 'e' && canEast) {
                    player_data[player].dr_field[0].y = playerFieldY;
                    player_data[player].dr_field[0].x = playerFieldX;
                    do {
                        drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                        if(!canEast) {
                            playerFieldX = player_data[player].dr_field[0].x;
                            break;
                        }
                        playerFieldX += 2;
                        if(playerFieldX > MAZE_MAX_SIZE-1) {
                            playerFieldX = player_data[player].dr_field[0].x;
                            break;
                        }
                        drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                        if(!canEast && !canSouth) {
                            playerFieldX = player_data[player].dr_field[0].x;
                            break;
                        }
                        if(canSouth) {
                            player_data[player].dr_field[1].y = playerFieldY;
                            player_data[player].dr_field[1].x = playerFieldX;
                            do {
                                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                if(!canSouth) {
                                    playerFieldY = player_data[player].dr_field[1].y;
                                    break;
                                }
                                playerFieldY += 2;
                                if(playerFieldY > MAZE_MAX_SIZE-1) {
                                    playerFieldY = player_data[player].dr_field[1].y;
                                    break;
                                }
                                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                if(!canEast && !canSouth && !canWest) { /* dead end? */
                                    playerFieldY = player_data[player].dr_field[1].y;
                                    break;
                                }
                                if(playerFieldX == targetPlayerFieldX || canSouth || canEast || canWest) {
                                    player_data[player].dr_field[2].y = playerFieldY;
                                    player_data[player].dr_field[2].x = playerFieldX;
                                    player_data[player].dr_field[3].y = 0;
                                    player_data[player].dr_dir[0] = PLAYER_DIR_NORTH+256;
                                    player_data[player].dr_dir[1] = PLAYER_DIR_EAST;
                                    player_data[player].dr_dir[2] = PLAYER_DIR_SOUTH;
                                    player_data[player].dr_dir[3] = -1;
                                    player_data[player].dr_fieldIndex = 0;
                                    return YES;
                                }
                            } while(canSouth);
                        }
                    } while(canEast);
                }
            }
        } while(canNorth);
    }
    playerFieldY = (player_data[player].ply_y >> MAZE_FIELD_SHIFT)|1;
    playerFieldX = (player_data[player].ply_x >> MAZE_FIELD_SHIFT)|1;
    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 0, 0, 0);
    if(wantedDirChar == 'w' && canNorth) {
        do {
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(!canNorth) break;
            playerFieldY -= 2;
            if(playerFieldY <= 0) break;
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(canEast) {
                player_data[player].dr_field[0].y = playerFieldY;
                player_data[player].dr_field[0].x = playerFieldX;
                do {
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canEast) {
                        playerFieldX = player_data[player].dr_field[0].x;
                        break;
                    }
                    playerFieldX += 2;
                    if(playerFieldX > MAZE_MAX_SIZE-1) {
                        playerFieldX = player_data[player].dr_field[0].x;
                        break;
                    }
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canEast && !canNorth) {
                        playerFieldX = player_data[player].dr_field[0].x;
                        break;
                    }
                    if(canNorth) {
                        player_data[player].dr_field[1].y = playerFieldY;
                        player_data[player].dr_field[1].x = playerFieldX;
                        do {
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canNorth) {
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            playerFieldY -= 2;
                            if(playerFieldY <= 0) {
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canNorth && !canWest) {
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            if(canWest) {
                                player_data[player].dr_field[2].y = playerFieldY;
                                player_data[player].dr_field[2].x = playerFieldX;
                                player_data[player].dr_field[3].y = playerFieldY;
                                player_data[player].dr_field[3].x = playerFieldX-2;
                                player_data[player].dr_field[4].y = 0;
                                player_data[player].dr_dir[0] = PLAYER_DIR_NORTH+256;
                                player_data[player].dr_dir[1] = PLAYER_DIR_EAST;
                                player_data[player].dr_dir[2] = PLAYER_DIR_NORTH+256;
                                player_data[player].dr_dir[3] = PLAYER_DIR_WEST;
                                player_data[player].dr_dir[4] = -1;
                                player_data[player].dr_fieldIndex = 0;
                                return YES;
                            }
                        } while(canNorth);
                        player_data[player].dr_dir[0] = 0;
                        player_data[player].dr_fieldIndex = 0;
                        return NO;
                    }
                } while(canEast);
                player_data[player].dr_dir[0] = 0;
                player_data[player].dr_fieldIndex = 0;
                return NO;
            }
        } while(canNorth);
        player_data[player].dr_dir[0] = 0;
        player_data[player].dr_fieldIndex = 0;
        return NO;
    }
    if(wantedDirChar == 'e' && canNorth) {
        do {
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(!canNorth) break;
            playerFieldY -= 2;
            if(playerFieldY <= 0) {
                playerFieldY = player_data[player].dr_field[0].y;
                break;
            }
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(canWest) {
                player_data[player].dr_field[0].y = playerFieldY;
                player_data[player].dr_field[0].x = playerFieldX;
                do {
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canWest) {
                        playerFieldX = player_data[player].dr_field[0].x;
                        break;
                    }
                    playerFieldX -= 2;
                    if(playerFieldX <= 0) {
                        playerFieldX = player_data[player].dr_field[0].x;
                        break;
                    }
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canWest && !canNorth) {
                        playerFieldX = player_data[player].dr_field[0].x;
                        break;
                    }
                    if(canNorth) {
                        player_data[player].dr_field[1].y = playerFieldY;
                        player_data[player].dr_field[1].x = playerFieldX;
                        do {
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canNorth) {
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            playerFieldY -= 2;
                            if(playerFieldY <= 0) {
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canNorth && !canEast) {
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            if(canEast) {
                                player_data[player].dr_field[2].y = playerFieldY;
                                player_data[player].dr_field[2].x = playerFieldX;
                                player_data[player].dr_field[3].y = playerFieldY;
                                player_data[player].dr_field[3].x = playerFieldX+2;
                                player_data[player].dr_field[4].y = 0;
                                player_data[player].dr_dir[0] = PLAYER_DIR_NORTH+256;
                                player_data[player].dr_dir[1] = PLAYER_DIR_WEST;
                                player_data[player].dr_dir[2] = PLAYER_DIR_NORTH+256;
                                player_data[player].dr_dir[3] = PLAYER_DIR_EAST;
                                player_data[player].dr_dir[4] = -1;
                                player_data[player].dr_fieldIndex = 0;
                                return YES;
                            }
                        } while(canNorth);
                        player_data[player].dr_dir[0] = 0;
                        player_data[player].dr_fieldIndex = 0;
                        return NO;
                    }
                } while(canWest);
                player_data[player].dr_dir[0] = 0;
                player_data[player].dr_fieldIndex = 0;
                return NO;
            }
        } while(canNorth);
        player_data[player].dr_dir[0] = 0;
        player_data[player].dr_fieldIndex = 0;
        return NO;
    }
    player_data[player].dr_dir[0] = 0;
    player_data[player].dr_fieldIndex = 0;
    return NO;
}

/************************************************************
 *** int drone_sub_ninja_south(int player,int viewCompassDirChar)
 ************************************************************/
int drone_sub_ninja_south(int player,int wantedDirChar) {
int target_player;
int targetPlayerFieldX;
int playerFieldX;
int playerFieldY;
int canWest;
int canEast;
int canSouth;
int canNorth;

    player_data[player].dr_fieldIndex = 0;
    player_data[player].dr_upRotationCounter = 0;
    player_data[player].dr_rotateCounter = 0;
    player_data[player].dr_dir[0] = 0;
    player_data[player].dr_field[0].y = 0;
    target_player = player_data[player].dr_currentTarget;
    playerFieldY = (player_data[player].ply_y >> MAZE_FIELD_SHIFT)|1;
    playerFieldX = (player_data[player].ply_x >> MAZE_FIELD_SHIFT)|1;
    targetPlayerFieldX = (player_data[target_player].ply_x >> MAZE_FIELD_SHIFT)|1;
    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 0, 0, 0);
    if(wantedDirChar == 'e' && !canEast && !canSouth && canWest) {
        do {
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(!canWest) break;
            playerFieldX -= 2;
            if(playerFieldX <= 0) break;
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(canSouth) {
                player_data[player].dr_field[0].y = playerFieldY;
                player_data[player].dr_field[0].x = playerFieldX;
                do {
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canSouth) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    playerFieldY += 2;
                    if(playerFieldY > MAZE_MAX_SIZE-1) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canSouth && !canEast) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    if(canEast) {
                        player_data[player].dr_field[1].y = playerFieldY;
                        player_data[player].dr_field[1].x = playerFieldX;
                        player_data[player].dr_field[2].y = playerFieldY;
                        player_data[player].dr_field[2].x = playerFieldX+2;
                        player_data[player].dr_field[3].y = 0;
                        player_data[player].dr_dir[0] = PLAYER_DIR_WEST;
                        player_data[player].dr_dir[1] = PLAYER_DIR_SOUTH;
                        player_data[player].dr_dir[2] = PLAYER_DIR_EAST;
                        player_data[player].dr_dir[3] = -1;
                        player_data[player].dr_fieldIndex = 0;
                        return YES;
                    }
                } while(canSouth);
                player_data[player].dr_dir[0] = 0;
                player_data[player].dr_fieldIndex = 0;
                return NO;
            }
        } while(canWest);
        player_data[player].dr_dir[0] = 0;
        player_data[player].dr_fieldIndex = 0;
        return NO;
    }
    if(wantedDirChar == 'w' && !canWest && !canSouth && canEast) {
        do {
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(!canEast) break;
            playerFieldX += 2;
            if(playerFieldX > MAZE_MAX_SIZE-1) break;
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(canSouth) {
                player_data[player].dr_field[0].y = playerFieldY;
                player_data[player].dr_field[0].x = playerFieldX;
                do {
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canSouth) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    playerFieldY += 2;
                    if(playerFieldY > MAZE_MAX_SIZE-1) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canSouth && !canWest) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    if(canWest) {
                        player_data[player].dr_field[1].y = playerFieldY;
                        player_data[player].dr_field[1].x = playerFieldX;
                        player_data[player].dr_field[2].y = playerFieldY;
                        player_data[player].dr_field[2].x = playerFieldX-2;
                        player_data[player].dr_field[3].y = 0;
                        player_data[player].dr_dir[0] = PLAYER_DIR_EAST;
                        player_data[player].dr_dir[1] = PLAYER_DIR_SOUTH;
                        player_data[player].dr_dir[2] = PLAYER_DIR_WEST;
                        player_data[player].dr_dir[3] = -1;
                        player_data[player].dr_fieldIndex = 0;
                        return YES;
                    }
                } while(canSouth);
                player_data[player].dr_dir[0] = 0;
                player_data[player].dr_fieldIndex = 0;
                return NO;
            }
        } while(canEast);
        player_data[player].dr_dir[0] = 0;
        player_data[player].dr_fieldIndex = 0;
        return NO;
    }
    if(canSouth) {
        do {
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(!canSouth) break;
            playerFieldY += 2;
            if(playerFieldY > MAZE_MAX_SIZE-1) break;
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(!canSouth && !canEast && !canWest) break; /* dead end? */
            if(wantedDirChar == 'w' && canWest) {
                player_data[player].dr_field[0].y = playerFieldY;
                player_data[player].dr_field[0].x = playerFieldX;
                do {
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canWest) {
                        playerFieldX = player_data[player].dr_field[0].x;
                        break;
                    }
                    playerFieldX -= 2;
                    if(playerFieldX <= 0) {
                        playerFieldX = player_data[player].dr_field[0].x;
                        break;
                    }
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canNorth && !canWest) {
                        playerFieldX = player_data[player].dr_field[0].x;
                        break;
                    }
                    if(canNorth) {
                        player_data[player].dr_field[1].y = playerFieldY;
                        player_data[player].dr_field[1].x = playerFieldX;
                        do {
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canNorth) {
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            playerFieldY -= 2;
                            if(playerFieldY <= 0) {
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canNorth && !canEast && !canWest) { /* dead end? */
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            if(playerFieldX == targetPlayerFieldX || canNorth || canEast || canWest) {
                                player_data[player].dr_field[2].y = playerFieldY;
                                player_data[player].dr_field[2].x = playerFieldX;
                                player_data[player].dr_field[3].y = 0;
                                player_data[player].dr_dir[0] = PLAYER_DIR_SOUTH;
                                player_data[player].dr_dir[1] = PLAYER_DIR_WEST;
                                player_data[player].dr_dir[2] = PLAYER_DIR_NORTH+256;
                                player_data[player].dr_dir[3] = -1;
                                player_data[player].dr_fieldIndex = 0;
                                return YES;
                            }
                        } while(canNorth);
                    }
                } while(canWest);
            } else {
                if(wantedDirChar == 'e' && canEast) {
                    player_data[player].dr_field[0].y = playerFieldY;
                    player_data[player].dr_field[0].x = playerFieldX;
                    do {
                        drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                        if(!canEast) {
                            playerFieldX = player_data[player].dr_field[0].x;
                            break;
                        }
                        playerFieldX += 2;
                        if(playerFieldX > MAZE_MAX_SIZE-1) {
                            playerFieldX = player_data[player].dr_field[0].x;
                            break;
                        }
                        drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                        if(!canNorth && !canEast) {
                            playerFieldX = player_data[player].dr_field[0].x;
                            break;
                        }
                        if(canNorth) {
                            player_data[player].dr_field[1].y = playerFieldY;
                            player_data[player].dr_field[1].x = playerFieldX;
                            do {
                                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                if(!canNorth) {
                                    playerFieldY = player_data[player].dr_field[1].y;
                                    break;
                                }
                                playerFieldY -= 2;
                                if(playerFieldY <= 0) {
                                    playerFieldY = player_data[player].dr_field[1].y;
                                    break;
                                }
                                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                if(!canNorth && !canEast && !canWest) { /* dead end? */
                                    playerFieldY = player_data[player].dr_field[1].y;
                                    break;
                                }
                                if(playerFieldX == targetPlayerFieldX || canNorth || canEast || canWest) {
                                    player_data[player].dr_field[2].y = playerFieldY;
                                    player_data[player].dr_field[2].x = playerFieldX;
                                    player_data[player].dr_field[3].y = 0;
                                    player_data[player].dr_dir[0] = PLAYER_DIR_SOUTH;
                                    player_data[player].dr_dir[1] = PLAYER_DIR_EAST;
                                    player_data[player].dr_dir[2] = PLAYER_DIR_NORTH+256;
                                    player_data[player].dr_dir[3] = -1;
                                    player_data[player].dr_fieldIndex = 0;
                                    return YES;
                                }
                            } while(canNorth);
                        }
                    } while(canEast);
                }
            }
        } while(canSouth);
    }
    playerFieldY = (player_data[player].ply_y >> MAZE_FIELD_SHIFT)|1;
    playerFieldX = (player_data[player].ply_x >> MAZE_FIELD_SHIFT)|1;
    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 0, 0, 0);
    if(wantedDirChar == 'w' && canSouth) {
        do {
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(!canSouth) break;
            playerFieldY += 2;
            if(playerFieldY > MAZE_MAX_SIZE-1) break;
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(canWest) {
                player_data[player].dr_field[0].y = playerFieldY;
                player_data[player].dr_field[0].x = playerFieldX;
                do {
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canWest) {
                        playerFieldX = player_data[player].dr_field[0].x;
                        break;
                    }
                    playerFieldX -= 2;
                    if(playerFieldX <= 0) {
                        playerFieldX = player_data[player].dr_field[0].x;
                        break;
                    }
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canWest && !canSouth) {
                        playerFieldX = player_data[player].dr_field[0].x;
                        break;
                    }
                    if(canSouth) {
                        player_data[player].dr_field[1].y = playerFieldY;
                        player_data[player].dr_field[1].x = playerFieldX;
                        do {
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canSouth) {
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            playerFieldY += 2;
                            if(playerFieldY > MAZE_MAX_SIZE-1) {
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canSouth && !canEast) {
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            if(canEast) {
                                player_data[player].dr_field[2].y = playerFieldY;
                                player_data[player].dr_field[2].x = playerFieldX;
                                player_data[player].dr_field[3].y = playerFieldY;
                                player_data[player].dr_field[3].x = playerFieldX+2;
                                player_data[player].dr_field[4].y = 0;
                                player_data[player].dr_dir[0] = PLAYER_DIR_SOUTH;
                                player_data[player].dr_dir[1] = PLAYER_DIR_WEST;
                                player_data[player].dr_dir[2] = PLAYER_DIR_SOUTH;
                                player_data[player].dr_dir[3] = PLAYER_DIR_EAST;
                                player_data[player].dr_dir[4] = -1;
                                player_data[player].dr_fieldIndex = 0;
                                return YES;
                            }
                        } while(canSouth);
                        player_data[player].dr_dir[0] = 0;
                        player_data[player].dr_fieldIndex = 0;
                        return NO;
                    }
                } while(canWest);
                player_data[player].dr_dir[0] = 0;
                player_data[player].dr_fieldIndex = 0;
                return NO;
            }
        } while(canSouth);
        player_data[player].dr_dir[0] = 0;
        player_data[player].dr_fieldIndex = 0;
        return NO;
    }
    if(wantedDirChar == 'e' && canSouth) {
        do {
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(!canSouth) break;
            playerFieldY += 2;
            if(playerFieldY > MAZE_MAX_SIZE-1) break;
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(canEast) {
                player_data[player].dr_field[0].y = playerFieldY;
                player_data[player].dr_field[0].x = playerFieldX;
                do {
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canEast) {
                        playerFieldX = player_data[player].dr_field[0].x;
                        break;
                    }
                    playerFieldX += 2;
                    if(playerFieldX > MAZE_MAX_SIZE-1) {
                        playerFieldX = player_data[player].dr_field[0].x;
                        break;
                    }
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canEast && !canSouth) {
                        playerFieldX = player_data[player].dr_field[0].x;
                        break;
                    }
                    if(canSouth) {
                        player_data[player].dr_field[1].y = playerFieldY;
                        player_data[player].dr_field[1].x = playerFieldX;
                        do {
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canSouth) {
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            playerFieldY += 2;
                            if(playerFieldY > MAZE_MAX_SIZE-1) {
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canSouth && !canWest) {
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            if(canWest) {
                                player_data[player].dr_field[2].y = playerFieldY;
                                player_data[player].dr_field[2].x = playerFieldX;
                                player_data[player].dr_field[3].y = playerFieldY;
                                player_data[player].dr_field[3].x = playerFieldX-2;
                                player_data[player].dr_field[4].y = 0;
                                player_data[player].dr_dir[0] = PLAYER_DIR_SOUTH;
                                player_data[player].dr_dir[1] = PLAYER_DIR_EAST;
                                player_data[player].dr_dir[2] = PLAYER_DIR_SOUTH;
                                player_data[player].dr_dir[3] = PLAYER_DIR_WEST;
                                player_data[player].dr_dir[4] = -1;
                                player_data[player].dr_fieldIndex = 0;
                                return YES;
                            }
                        } while(canSouth);
                        player_data[player].dr_dir[0] = 0;
                        player_data[player].dr_fieldIndex = 0;
                        return NO;
                    }
                } while(canEast);
                player_data[player].dr_dir[0] = 0;
                player_data[player].dr_fieldIndex = 0;
                return NO;
            }
        } while(canSouth);
        player_data[player].dr_dir[0] = 0;
        player_data[player].dr_fieldIndex = 0;
        return NO;
    }
    player_data[player].dr_dir[0] = 0;
    player_data[player].dr_fieldIndex = 0;
    return NO;
}

/************************************************************
 *** int drone_sub_ninja_east(int player,int viewCompassDirChar)
 ************************************************************/
int drone_sub_ninja_east(int player,int wantedDirChar) {
int target_player;
int targetPlayerFieldY;
int playerFieldX;
int playerFieldY;
int canWest;
int canEast;
int canSouth;
int canNorth;

    player_data[player].dr_fieldIndex = 0;
    player_data[player].dr_upRotationCounter = 0;
    player_data[player].dr_rotateCounter = 0;
    player_data[player].dr_dir[0] = 0;
    player_data[player].dr_field[0].y = 0;
    target_player = player_data[player].dr_currentTarget;
    playerFieldY = (player_data[player].ply_y >> MAZE_FIELD_SHIFT)|1;
    playerFieldX = (player_data[player].ply_x >> MAZE_FIELD_SHIFT)|1;
    targetPlayerFieldY = (player_data[target_player].ply_y >> MAZE_FIELD_SHIFT)|1;
    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 0, 0, 0);
    if(wantedDirChar == 'n' && !canNorth && !canEast && canSouth) {
        do {
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(!canSouth) break;
            playerFieldY += 2;
            if(playerFieldY > MAZE_MAX_SIZE-1) break;
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(canEast) {
                player_data[player].dr_field[0].y = playerFieldY;
                player_data[player].dr_field[0].x = playerFieldX;
                do {
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canEast) {
                        playerFieldX = player_data[player].dr_field[0].x;
                        break;
                    }
                    playerFieldX += 2;
                    if(playerFieldX > MAZE_MAX_SIZE-1) {
                        playerFieldX = player_data[player].dr_field[0].x;
                        break;
                    }
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canEast && !canNorth) {
                        playerFieldX = player_data[player].dr_field[0].x;
                        break;
                    }
                    if(canNorth) {
                        player_data[player].dr_field[1].y = playerFieldY;
                        player_data[player].dr_field[1].x = playerFieldX;
                        do {
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canNorth) {
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            playerFieldY -= 2;
                            if(playerFieldY <= 0) {
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canNorth && !canWest) {
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            if(canWest) {
                                player_data[player].dr_field[2].y = playerFieldY;
                                player_data[player].dr_field[2].x = playerFieldX;
                                player_data[player].dr_field[3].y = playerFieldY;
                                player_data[player].dr_field[3].x = playerFieldX-2;
                                player_data[player].dr_field[4].y = 0;
                                player_data[player].dr_dir[0] = PLAYER_DIR_SOUTH;
                                player_data[player].dr_dir[1] = PLAYER_DIR_EAST;
                                player_data[player].dr_dir[2] = PLAYER_DIR_NORTH+256;
                                player_data[player].dr_dir[3] = PLAYER_DIR_WEST;
                                player_data[player].dr_dir[4] = -1;
                                player_data[player].dr_fieldIndex = 0;
                                return YES;
                            }
                        } while(canNorth);
                        player_data[player].dr_dir[0] = 0;
                        player_data[player].dr_fieldIndex = 0;
                        return NO;
                    }
                } while(canEast);
                player_data[player].dr_dir[0] = 0;
                player_data[player].dr_fieldIndex = 0;
                return NO;
            }
        } while(canSouth);
        player_data[player].dr_dir[0] = 0;
        player_data[player].dr_fieldIndex = 0;
        return NO;
    }
    if(wantedDirChar == 's' && !canSouth && !canEast && canNorth) {
        do {
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(!canNorth) break;
            playerFieldY -= 2;
            if(playerFieldY <= 0) break;
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(canEast) {
                player_data[player].dr_field[0].y = playerFieldY;
                player_data[player].dr_field[0].x = playerFieldX;
                do {
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canEast) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    playerFieldX += 2;
                    if(playerFieldX > MAZE_MAX_SIZE-1) {
                        playerFieldX = player_data[player].dr_field[0].x;
                        break;
                    }
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canEast && !canSouth) {
                        playerFieldX = player_data[player].dr_field[0].x;
                        break;
                    }
                    if(canSouth) {
                        player_data[player].dr_field[1].y = playerFieldY;
                        player_data[player].dr_field[1].x = playerFieldX;
                        do {
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canSouth) {
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            playerFieldY += 2;
                            if(playerFieldY > MAZE_MAX_SIZE-1) {
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canSouth && !canWest) {
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            if(canWest) {
                                player_data[player].dr_field[2].y = playerFieldY;
                                player_data[player].dr_field[2].x = playerFieldX;
                                player_data[player].dr_field[3].y = playerFieldY;
                                player_data[player].dr_field[3].x = playerFieldX-2;
                                player_data[player].dr_field[4].y = 0;
                                player_data[player].dr_dir[0] = PLAYER_DIR_NORTH+256;
                                player_data[player].dr_dir[1] = PLAYER_DIR_EAST;
                                player_data[player].dr_dir[2] = PLAYER_DIR_SOUTH;
                                player_data[player].dr_dir[3] = PLAYER_DIR_WEST;
                                player_data[player].dr_dir[4] = -1;
                                player_data[player].dr_fieldIndex = 0;
                                return YES;
                            }
                        } while(canSouth);
                        player_data[player].dr_dir[0] = 0;
                        player_data[player].dr_fieldIndex = 0;
                        return NO;
                    }
                } while(canEast);
                player_data[player].dr_dir[0] = 0;
                player_data[player].dr_fieldIndex = 0;
                return NO;
            }
        } while(canNorth);
        player_data[player].dr_dir[0] = 0;
        player_data[player].dr_fieldIndex = 0;
        return NO;
    }
    if(canEast) {
        do {
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(!canEast) break;
            playerFieldX += 2;
            if(playerFieldX > MAZE_MAX_SIZE-1) break;
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(wantedDirChar == 'n' && canNorth) {
                player_data[player].dr_field[0].y = playerFieldY;
                player_data[player].dr_field[0].x = playerFieldX;
                do {
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canNorth) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    playerFieldY -= 2;
                    if(playerFieldY <= 0) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canNorth && !canWest) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    if(canWest) {
                        player_data[player].dr_field[1].y = playerFieldY;
                        player_data[player].dr_field[1].x = playerFieldX;
                        do {
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canWest) {
                                playerFieldX = player_data[player].dr_field[1].x;
                                break;
                            }
                            playerFieldX -= 2;
                            if(playerFieldX <= 0) {
                                playerFieldX = player_data[player].dr_field[1].x;
                                break;
                            }
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canWest && !canNorth && !canSouth) { /* dead end? */
                                playerFieldX = player_data[player].dr_field[1].x;
                                break;
                            }
                            if(playerFieldY == targetPlayerFieldY || canWest || canNorth || canSouth) {
                                player_data[player].dr_field[2].y = playerFieldY;
                                player_data[player].dr_field[2].x = playerFieldX;
                                player_data[player].dr_field[3].y = 0;
                                player_data[player].dr_dir[0] = PLAYER_DIR_EAST;
                                player_data[player].dr_dir[1] = PLAYER_DIR_NORTH+256;
                                player_data[player].dr_dir[2] = PLAYER_DIR_WEST;
                                player_data[player].dr_dir[3] = -1;
                                player_data[player].dr_fieldIndex = 0;
                                return YES;
                            }
                        } while(canWest);
                    }
                } while(canNorth);
            } else {
                if(wantedDirChar == 's' && canSouth) {
                    player_data[player].dr_field[0].y = playerFieldY;
                    player_data[player].dr_field[0].x = playerFieldX;
                    do {
                        drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                        if(!canSouth) break;
                        playerFieldY += 2;
                        if(playerFieldY > MAZE_MAX_SIZE-1) {
                            playerFieldY = player_data[player].dr_field[0].y;
                            break;
                        }
                        drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                        if(!canSouth && !canWest) {
                            playerFieldY = player_data[player].dr_field[0].y;
                            break;
                        }
                        if(canWest) {
                            player_data[player].dr_field[1].y = playerFieldY;
                            player_data[player].dr_field[1].x = playerFieldX;
                            do {
                                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                if(!canWest) {
                                    playerFieldX = player_data[player].dr_field[1].x;
                                    break;
                                }
                                playerFieldX -= 2;
                                if(playerFieldX <= 0) {
                                    playerFieldX = player_data[player].dr_field[1].x;
                                    break;
                                }
                                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                if(!canNorth && !canWest && !canSouth) { /* dead end? */
                                    playerFieldX = player_data[player].dr_field[1].x;
                                    break;
                                }
                                if(playerFieldY == targetPlayerFieldY || canWest || canNorth || canSouth) {
                                    player_data[player].dr_field[2].y = playerFieldY;
                                    player_data[player].dr_field[2].x = playerFieldX;
                                    player_data[player].dr_field[3].y = 0;
                                    player_data[player].dr_dir[0] = PLAYER_DIR_EAST;
                                    player_data[player].dr_dir[1] = PLAYER_DIR_SOUTH;
                                    player_data[player].dr_dir[2] = PLAYER_DIR_WEST;
                                    player_data[player].dr_dir[3] = -1;
                                    player_data[player].dr_fieldIndex = 0;
                                    return YES;
                                }
                            } while(canWest);
                        }
                    } while(canSouth);
                }
            }
        } while(canEast);
    }
    playerFieldY = (player_data[player].ply_y >> MAZE_FIELD_SHIFT)|1;
    playerFieldX = (player_data[player].ply_x >> MAZE_FIELD_SHIFT)|1;
    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 0, 0, 0);
    if(wantedDirChar == 'n' && canEast) {
        do {
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(!canEast) break;
            playerFieldX += 2;
            if(playerFieldX > MAZE_MAX_SIZE-1) break;
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(!canEast && !canSouth) break;
            if(canSouth) {
                player_data[player].dr_field[0].y = playerFieldY;
                player_data[player].dr_field[0].x = playerFieldX;
                do {
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canSouth) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    playerFieldY += 2;
                    if(playerFieldY > MAZE_MAX_SIZE-1) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canSouth && !canEast) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    if(canEast) {
                        player_data[player].dr_field[1].y = playerFieldY;
                        player_data[player].dr_field[1].x = playerFieldX;
                        do {
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canEast) {
                                playerFieldX = player_data[player].dr_field[1].x;
                                break;
                            }
                            playerFieldX += 2;
                            if(playerFieldX > MAZE_MAX_SIZE-1) {
                                playerFieldX = player_data[player].dr_field[1].x;
                                break;
                            }
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canEast && !canNorth) {
                                playerFieldX = player_data[player].dr_field[1].x;
                                break;
                            }
                            if(canNorth) {
                                player_data[player].dr_field[2].y = playerFieldY;
                                player_data[player].dr_field[2].x = playerFieldX;
                                player_data[player].dr_field[3].y = playerFieldY-2;
                                player_data[player].dr_field[3].x = playerFieldX;
                                player_data[player].dr_field[4].y = 0;
                                player_data[player].dr_dir[0] = PLAYER_DIR_EAST;
                                player_data[player].dr_dir[1] = PLAYER_DIR_SOUTH;
                                player_data[player].dr_dir[2] = PLAYER_DIR_EAST;
                                player_data[player].dr_dir[3] = PLAYER_DIR_NORTH+256;
                                player_data[player].dr_dir[4] = -1;
                                player_data[player].dr_fieldIndex = 0;
                                return YES;
                            }
                        } while(canEast);
                        player_data[player].dr_dir[0] = 0;
                        player_data[player].dr_fieldIndex = 0;
                        return NO;
                    }
                } while(canSouth);
                player_data[player].dr_dir[0] = 0;
                player_data[player].dr_fieldIndex = 0;
                return NO;
            }
        } while(canEast);
        player_data[player].dr_dir[0] = 0;
        player_data[player].dr_fieldIndex = 0;
        return NO;
    }
    if(wantedDirChar == 's' && canEast) {
        do {
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(!canEast) break;
            playerFieldX += 2;
            if(playerFieldX > MAZE_MAX_SIZE-1) break;
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(!canEast && !canNorth) break;
            if(canNorth) {
                player_data[player].dr_field[0].y = playerFieldY;
                player_data[player].dr_field[0].x = playerFieldX;
                do {
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canNorth) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    playerFieldY -= 2;
                    if(playerFieldY <= 0) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canNorth && !canEast) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    if(canEast) {
                        player_data[player].dr_field[1].y = playerFieldY;
                        player_data[player].dr_field[1].x = playerFieldX;
                        do {
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canEast) {
                                playerFieldX = player_data[player].dr_field[1].x;
                                break;
                            }
                            playerFieldX += 2;
                            if(playerFieldX > MAZE_MAX_SIZE-1) {
                                playerFieldX = player_data[player].dr_field[1].y;
                                break;
                            }
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canEast && !canSouth) {
                                playerFieldX = player_data[player].dr_field[1].x;
                                break;
                            }
                            if(canSouth) {
                                player_data[player].dr_field[2].y = playerFieldY;
                                player_data[player].dr_field[2].x = playerFieldX;
                                player_data[player].dr_field[3].y = playerFieldY+2;
                                player_data[player].dr_field[3].x = playerFieldX;
                                player_data[player].dr_field[4].y = 0;
                                player_data[player].dr_dir[0] = PLAYER_DIR_EAST;
                                player_data[player].dr_dir[1] = PLAYER_DIR_NORTH+256;
                                player_data[player].dr_dir[2] = PLAYER_DIR_EAST;
                                player_data[player].dr_dir[3] = PLAYER_DIR_SOUTH;
                                player_data[player].dr_dir[4] = -1;
                                player_data[player].dr_fieldIndex = 0;
                                return YES;
                            }
                        } while(canEast);
                        player_data[player].dr_dir[0] = 0;
                        player_data[player].dr_fieldIndex = 0;
                        return NO;
                    }
                } while(canNorth);
                player_data[player].dr_dir[0] = 0;
                player_data[player].dr_fieldIndex = 0;
                return NO;
            }
        } while(canEast);
        player_data[player].dr_dir[0] = 0;
        player_data[player].dr_fieldIndex = 0;
        return NO;
    }
    player_data[player].dr_dir[0] = 0;
    player_data[player].dr_fieldIndex = 0;
    return NO;
}

/************************************************************
 *** int drone_sub_ninja_west(int player,int viewCompassDirChar)
 ************************************************************/
int drone_sub_ninja_west(int player,int wantedDirChar) {
int target_player;
int targetPlayerFieldY;
int playerFieldX;
int playerFieldY;
int canWest;
int canEast;
int canSouth;
int canNorth;

    player_data[player].dr_fieldIndex = 0;
    player_data[player].dr_upRotationCounter = 0;
    player_data[player].dr_rotateCounter = 0;
    player_data[player].dr_dir[0] = 0;
    player_data[player].dr_field[0].y = 0;
    target_player = player_data[player].dr_currentTarget;
    playerFieldY = (player_data[player].ply_y >> MAZE_FIELD_SHIFT)|1;
    playerFieldX = (player_data[player].ply_x >> MAZE_FIELD_SHIFT)|1;
    targetPlayerFieldY = (player_data[target_player].ply_y >> MAZE_FIELD_SHIFT)|1;
    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 0, 0, 0);
    if(wantedDirChar == 'n' && !canNorth && !canWest && canSouth) {
        do {
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(!canSouth) break;
            playerFieldY += 2;
            if(playerFieldY > MAZE_MAX_SIZE-1) break;
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(!canSouth && !canWest) break;
            if(canWest) {
                player_data[player].dr_field[0].y = playerFieldY;
                player_data[player].dr_field[0].x = playerFieldX;
                do {
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canWest) {
                        playerFieldX = player_data[player].dr_field[0].x;
                        break;
                    }
                    playerFieldX -= 2;
                    if(playerFieldX <= 0) {
                        playerFieldX = player_data[player].dr_field[0].x;
                        break;
                    }
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canWest && !canNorth) {
                        playerFieldX = player_data[player].dr_field[0].x;
                        break;
                    }
                    if(canNorth) {
                        player_data[player].dr_field[1].y = playerFieldY;
                        player_data[player].dr_field[1].x = playerFieldX;
                        do {
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canNorth) {
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            playerFieldY -= 2;
                            if(playerFieldY <= 0) {
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canNorth && !canEast) {
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            if(canEast) {
                                player_data[player].dr_field[2].y = playerFieldY;
                                player_data[player].dr_field[2].x = playerFieldX;
                                player_data[player].dr_field[3].y = playerFieldY;
                                player_data[player].dr_field[3].x = playerFieldX+2;
                                player_data[player].dr_field[4].y = 0;
                                player_data[player].dr_dir[0] = PLAYER_DIR_SOUTH;
                                player_data[player].dr_dir[1] = PLAYER_DIR_WEST;
                                player_data[player].dr_dir[2] = PLAYER_DIR_NORTH+256;
                                player_data[player].dr_dir[3] = PLAYER_DIR_EAST;
                                player_data[player].dr_dir[4] = -1;
                                player_data[player].dr_fieldIndex = 0;
                                return YES;
                            }
                        } while(canNorth);
                        player_data[player].dr_dir[0] = 0;
                        player_data[player].dr_fieldIndex = 0;
                        return NO;
                    }
                } while(canWest);
                player_data[player].dr_dir[0] = 0;
                player_data[player].dr_fieldIndex = 0;
                return NO;
            }
        } while(canSouth);
        player_data[player].dr_dir[0] = 0;
        player_data[player].dr_fieldIndex = 0;
        return NO;
    }
    if(wantedDirChar == 's' && !canSouth && !canWest && canNorth) {
        do {
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(!canNorth) break;
            playerFieldY -= 2;
            if(playerFieldY <= 0) break;
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(!canNorth && !canWest) break;
            if(canWest) {
                player_data[player].dr_field[0].y = playerFieldY;
                player_data[player].dr_field[0].x = playerFieldX;
                do {
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canWest) {
                        playerFieldX = player_data[player].dr_field[0].x;
                        break;
                    }
                    playerFieldX -= 2;
                    if(playerFieldX <= 0) {
                        playerFieldX = player_data[player].dr_field[0].x;
                        break;
                    }
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canWest && !canSouth) {
                        playerFieldX = player_data[player].dr_field[0].x;
                        break;
                    }
                    if(canSouth) {
                        player_data[player].dr_field[1].y = playerFieldY;
                        player_data[player].dr_field[1].x = playerFieldX;
                        do {
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canSouth) {
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            playerFieldY += 2;
                            if(playerFieldY > MAZE_MAX_SIZE-1) {
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canSouth && !canEast) {
                                playerFieldY = player_data[player].dr_field[1].y;
                                break;
                            }
                            if(canEast) {
                                player_data[player].dr_field[2].y = playerFieldY;
                                player_data[player].dr_field[2].x = playerFieldX;
                                player_data[player].dr_field[3].y = playerFieldY;
                                player_data[player].dr_field[3].x = playerFieldX+2;
                                player_data[player].dr_field[4].y = 0;
                                player_data[player].dr_dir[0] = PLAYER_DIR_NORTH+256;
                                player_data[player].dr_dir[1] = PLAYER_DIR_WEST;
                                player_data[player].dr_dir[2] = PLAYER_DIR_SOUTH;
                                player_data[player].dr_dir[3] = PLAYER_DIR_EAST;
                                player_data[player].dr_dir[4] = -1;
                                player_data[player].dr_fieldIndex = 0;
                                return YES;
                            }
                        } while(canSouth);
                        player_data[player].dr_dir[0] = 0;
                        player_data[player].dr_fieldIndex = 0;
                        return NO;
                    }
                } while(canWest);
                player_data[player].dr_dir[0] = 0;
                player_data[player].dr_fieldIndex = 0;
                return NO;
            }
        } while(canNorth);
        player_data[player].dr_dir[0] = 0;
        player_data[player].dr_fieldIndex = 0;
        return NO;
    }
    if(canWest) {
        do {
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(!canWest) break;
            playerFieldX -= 2;
            if(playerFieldX <= 0) break;
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(wantedDirChar == 'n' && canNorth) {
                player_data[player].dr_field[0].y = playerFieldY;
                player_data[player].dr_field[0].x = playerFieldX;
                do {
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canNorth) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    playerFieldY -= 2;
                    if(playerFieldY <= 0) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canNorth && !canEast) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    if(canEast) {
                        player_data[player].dr_field[1].y = playerFieldY;
                        player_data[player].dr_field[1].x = playerFieldX;
                        do {
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canEast) {
                                playerFieldX = player_data[player].dr_field[1].x;
                                break;
                            }
                            playerFieldX += 2;
                            if(playerFieldX > MAZE_MAX_SIZE-1) {
                                playerFieldX = player_data[player].dr_field[1].x;
                                break;
                            }
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canWest && !canEast && !canNorth) { /* dead end? */
                                playerFieldX = player_data[player].dr_field[1].x;
                                break;
                            }
                            if(playerFieldY == targetPlayerFieldY || canEast || canWest || canNorth) {
                                player_data[player].dr_field[2].y = playerFieldY;
                                player_data[player].dr_field[2].x = playerFieldX;
                                player_data[player].dr_field[3].y = 0;
                                player_data[player].dr_dir[0] = PLAYER_DIR_WEST;
                                player_data[player].dr_dir[1] = PLAYER_DIR_NORTH+256;
                                player_data[player].dr_dir[2] = PLAYER_DIR_EAST;
                                player_data[player].dr_dir[3] = -1;
                                player_data[player].dr_fieldIndex = 0;
                                return YES;
                            }
                        } while(canEast);
                    }
                } while(canNorth);
            } else {
                if(wantedDirChar == 's' && canSouth) {
                    player_data[player].dr_field[0].y = playerFieldY;
                    player_data[player].dr_field[0].x = playerFieldX;
                    do {
                        drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                        if(!canSouth) {
                            playerFieldY = player_data[player].dr_field[0].y;
                            break;
                        }
                        playerFieldY += 2;
                        if(playerFieldY > MAZE_MAX_SIZE-1) {
                            playerFieldY = player_data[player].dr_field[0].y;
                            break;
                        }
                        drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                        if(!canSouth && !canEast) {
                            playerFieldY = player_data[player].dr_field[0].y;
                            break;
                        }
                        if(canEast) {
                            player_data[player].dr_field[1].y = playerFieldY;
                            player_data[player].dr_field[1].x = playerFieldX;
                            do {
                                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                if(!canEast) {
                                    playerFieldX = player_data[player].dr_field[1].x;
                                    break;
                                }
                                playerFieldX += 2;
                                if(playerFieldX > MAZE_MAX_SIZE-1) {
                                    playerFieldX = player_data[player].dr_field[1].x;
                                    break;
                                }
                                drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                                if(!canWest && !canEast && !canSouth) { /* dead end? */
                                    playerFieldX = player_data[player].dr_field[1].x;
                                    break;
                                }
                                if(playerFieldY == targetPlayerFieldY || canEast || canSouth || canWest) {
                                    player_data[player].dr_field[2].y = playerFieldY;
                                    player_data[player].dr_field[2].x = playerFieldX;
                                    player_data[player].dr_field[3].y = 0;
                                    player_data[player].dr_dir[0] = PLAYER_DIR_WEST;
                                    player_data[player].dr_dir[1] = PLAYER_DIR_SOUTH;
                                    player_data[player].dr_dir[2] = PLAYER_DIR_EAST;
                                    player_data[player].dr_dir[3] = -1;
                                    player_data[player].dr_fieldIndex = 0;
                                    return YES;
                                }
                            } while(canEast);
                        }
                    } while(canSouth);
                }
            }
        } while(canWest);
    }
    playerFieldY = (player_data[player].ply_y >> MAZE_FIELD_SHIFT)|1;
    playerFieldX = (player_data[player].ply_x >> MAZE_FIELD_SHIFT)|1;
    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 0, 0, 0);
    if(wantedDirChar == 'n' && canWest) {
        do {
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(!canWest) break;
            playerFieldX -= 2;
            if(playerFieldX <= 0) break;
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(!canWest && !canSouth) break;
            if(canSouth) {
                player_data[player].dr_field[0].y = playerFieldY;
                player_data[player].dr_field[0].x = playerFieldX;
                do {
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canSouth) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    playerFieldY += 2;
                    if(playerFieldY > MAZE_MAX_SIZE-1) {
                        playerFieldY = player_data[player].dr_field[0].x;
                        break;
                    }
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canSouth && !canWest) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    if(canWest) {
                        player_data[player].dr_field[1].y = playerFieldY;
                        player_data[player].dr_field[1].x = playerFieldX;
                        do {
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canWest) {
                                playerFieldX = player_data[player].dr_field[1].x;
                                break;
                            }
                            playerFieldX -= 2;
                            if(playerFieldX <= 0) {
                                playerFieldX = player_data[player].dr_field[1].x;
                                break;
                            }
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canWest && !canNorth) {
                                playerFieldX = player_data[player].dr_field[1].x;
                                break;
                            }
                            if(canNorth) {
                                player_data[player].dr_field[2].y = playerFieldY;
                                player_data[player].dr_field[2].x = playerFieldX;
                                player_data[player].dr_field[3].y = playerFieldY-2;
                                player_data[player].dr_field[3].x = playerFieldX;
                                player_data[player].dr_field[4].y = 0;
                                player_data[player].dr_dir[0] = PLAYER_DIR_WEST;
                                player_data[player].dr_dir[1] = PLAYER_DIR_SOUTH;
                                player_data[player].dr_dir[2] = PLAYER_DIR_WEST;
                                player_data[player].dr_dir[3] = PLAYER_DIR_NORTH+256;
                                player_data[player].dr_dir[4] = -1;
                                player_data[player].dr_fieldIndex = 0;
                                return YES;
                            }
                        } while(canWest);
                        player_data[player].dr_dir[0] = 0;
                        player_data[player].dr_fieldIndex = 0;
                        return NO;
                    }
                } while(canSouth);
                player_data[player].dr_dir[0] = 0;
                player_data[player].dr_fieldIndex = 0;
                return NO;
            }
        } while(canWest);
        player_data[player].dr_dir[0] = 0;
        player_data[player].dr_fieldIndex = 0;
        return NO;
    }
    if(wantedDirChar == 's' && canWest) {
        do {
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(!canWest) break;
            playerFieldX -= 2;
            if(playerFieldX <= 0) break;
            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
            if(!canWest && !canNorth) break;
            if(canNorth) {
                player_data[player].dr_field[0].y = playerFieldY;
                player_data[player].dr_field[0].x = playerFieldX;
                do {
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canNorth) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    playerFieldY -= 2;
                    if(playerFieldY <= 0) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                    if(!canNorth && !canWest) {
                        playerFieldY = player_data[player].dr_field[0].y;
                        break;
                    }
                    if(canWest) {
                        player_data[player].dr_field[1].y = playerFieldY;
                        player_data[player].dr_field[1].x = playerFieldX;
                        do {
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canWest) {
                                playerFieldX = player_data[player].dr_field[1].x;
                                break;
                            }
                            playerFieldX -= 2;
                            if(playerFieldX <= 0) {
                                playerFieldX = player_data[player].dr_field[1].x;
                                break;
                            }
                            drone_check_directions(player, &canNorth, &canSouth, &canEast, &canWest, 1, playerFieldY, playerFieldX);
                            if(!canWest && !canSouth) {
                                playerFieldX = player_data[player].dr_field[1].x;
                                break;
                            }
                            if(canSouth) {
                                player_data[player].dr_field[2].y = playerFieldY;
                                player_data[player].dr_field[2].x = playerFieldX;
                                player_data[player].dr_field[3].y = playerFieldY+2;
                                player_data[player].dr_field[3].x = playerFieldX;
                                player_data[player].dr_field[4].y = 0;
                                player_data[player].dr_dir[0] = PLAYER_DIR_WEST;
                                player_data[player].dr_dir[1] = PLAYER_DIR_NORTH+256;
                                player_data[player].dr_dir[2] = PLAYER_DIR_WEST;
                                player_data[player].dr_dir[3] = PLAYER_DIR_SOUTH;
                                player_data[player].dr_dir[4] = -1;
                                player_data[player].dr_fieldIndex = 0;
                                return YES;
                            }
                        } while(canWest);
                        player_data[player].dr_dir[0] = 0;
                        player_data[player].dr_fieldIndex = 0;
                        return NO;
                    }
                } while(canNorth);
                player_data[player].dr_dir[0] = 0;
                player_data[player].dr_fieldIndex = 0;
                return NO;
            }
        } while(canWest);
        player_data[player].dr_dir[0] = 0;
        player_data[player].dr_fieldIndex = 0;
        return NO;
    }
    player_data[player].dr_dir[0] = 0;
    player_data[player].dr_fieldIndex = 0;
    return NO;
}

/************************************************************
 *** A drone never moves backwards, it might have been planned, but was never implemented
 ************************************************************/
__attribute__((unused)) void drone_move_down(int player) {
    if(player_data[player].dr_targetLocked) {
        player_joy_table[player] = player_data[player].dr_joystick = JOYSTICK_BUTTON|JOYSTICK_DOWN;
    } else {
        player_joy_table[player] = player_data[player].dr_joystick = JOYSTICK_DOWN;
    }
}

/************************************************************
 *** Trigger a 90 degree turn forward right. This is done by
 *** forcing a 45 degree turn. The code in drone_generate_joystickdata()
 *** will then continue the rotation till a full 90 degree turn was done 
 ************************************************************/
void drone_move_upright(int player) {
    player_data[player].dr_upRotationCounter = (256/PLAYER_MOTION_ROTATE)/8-1; /* /8 = 45 degree turn */
    if(player_data[player].dr_targetLocked) {
        player_joy_table[player] = player_data[player].dr_joystick = JOYSTICK_BUTTON|JOYSTICK_RIGHT|JOYSTICK_UP;
    } else {
        player_joy_table[player] = player_data[player].dr_joystick = JOYSTICK_RIGHT|JOYSTICK_UP;
    }
}

/************************************************************
 *** Trigger a 90 degree turn forward left. This is done by
 *** forcing a 45 degree turn. The code in drone_generate_joystickdata()
 *** will then continue the rotation till a full 90 degree turn was done 
 ************************************************************/
void drone_move_upleft(int player) {
    player_data[player].dr_upRotationCounter = (256/PLAYER_MOTION_ROTATE)/8-1; /* /8 = 45 degree turn */
    if(player_data[player].dr_targetLocked) {
        player_joy_table[player] = player_data[player].dr_joystick = JOYSTICK_BUTTON|JOYSTICK_LEFT|JOYSTICK_UP;
    } else {
        player_joy_table[player] = player_data[player].dr_joystick = JOYSTICK_LEFT|JOYSTICK_UP;
    }
}

/************************************************************
 *** Drone moving forward
 ************************************************************/
void drone_move_up(int player) {
    if(player_data[player].dr_targetLocked) {
        player_joy_table[player] = player_data[player].dr_joystick = JOYSTICK_BUTTON|JOYSTICK_UP;
    } else {
        player_joy_table[player] = player_data[player].dr_joystick = JOYSTICK_UP;
    }
}

/************************************************************
 *** void drone_turn_around(int player)
 ************************************************************/
void drone_turn_around(int player) {
    player_data[player].dr_rotateCounter = (256/PLAYER_MOTION_ROTATE)/2-1; /* /2 = half of a full rotation = 180 degree turn */
    if(player_data[player].dr_targetLocked) {
        player_joy_table[player] = player_data[player].dr_joystick = JOYSTICK_BUTTON|JOYSTICK_RIGHT;
    } else {
        player_joy_table[player] = player_data[player].dr_joystick = JOYSTICK_RIGHT;
    }
}
