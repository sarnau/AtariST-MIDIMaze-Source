/************************************************************
 *** The Play/Team dialog
 ************************************************************/
#include "globals.h"
#include "rstest.h"

static void update_names_in_rsc(int *droneList);

static const short revive_lives_rscindices[PLAYER_MAX_LIVES] = { PREF_1LIFE, PREF_2LIVES, PREF_3LIVES };	/* how many lives after a revive: 1, 2 or 3 */
static int rsc_drones[DRONE_TYPES] = { 0, 0, 0 };	/* 3 drone types: integer values, how many of each type */
static const short drone_count_up_rscindices[DRONE_TYPES] = { DUMB_UP, PLAIN_UP, NINJA_UP };	/* 3 drone types: up arrows to increase their numbers */
static const short drone_count_down_rscindices[DRONE_TYPES] = { DUMB_DOWN, PLAIN_DOWN, NINJA_DOWN };	/* 3 drone types: down arrows to decrease their numbers */
static const short drone_count_number_rscindices[DRONE_TYPES] = { DUMB_VAL, PLAIN_VAL, NINJA_VAL };	/* 3 drone types: actual number value */
static const short drone_count_box_rscindices[DRONE_TYPES] = { DUMB_BOX, PLAIN_BOX, NINJA_BOX };	/* 3 drone types: box to redraw the cell */

const short team_group_rscindices[PLAYER_MAX_COUNT][PLAYER_MAX_TEAMS] = {
	{ PLAYER_1_TEAM_1,  PLAYER_1_TEAM_2,  PLAYER_1_TEAM_3,  PLAYER_1_TEAM_4 },
	{ PLAYER_2_TEAM_1,  PLAYER_2_TEAM_2,  PLAYER_2_TEAM_3,  PLAYER_2_TEAM_4 },
	{ PLAYER_3_TEAM_1,  PLAYER_3_TEAM_2,  PLAYER_3_TEAM_3,  PLAYER_3_TEAM_4 },
	{ PLAYER_4_TEAM_1,  PLAYER_4_TEAM_2,  PLAYER_4_TEAM_3,  PLAYER_4_TEAM_4 },
	{ PLAYER_5_TEAM_1,  PLAYER_5_TEAM_2,  PLAYER_5_TEAM_3,  PLAYER_5_TEAM_4 },
	{ PLAYER_6_TEAM_1,  PLAYER_6_TEAM_2,  PLAYER_6_TEAM_3,  PLAYER_6_TEAM_4 },
	{ PLAYER_7_TEAM_1,  PLAYER_7_TEAM_2,  PLAYER_7_TEAM_3,  PLAYER_7_TEAM_4 },
	{ PLAYER_8_TEAM_1,  PLAYER_8_TEAM_2,  PLAYER_8_TEAM_3,  PLAYER_8_TEAM_4 },
	{ PLAYER_9_TEAM_1,  PLAYER_9_TEAM_2,  PLAYER_9_TEAM_3,  PLAYER_9_TEAM_4 },
	{ PLAYER_10_TEAM_1, PLAYER_10_TEAM_2, PLAYER_10_TEAM_3, PLAYER_10_TEAM_4 },
	{ PLAYER_11_TEAM_1, PLAYER_11_TEAM_2, PLAYER_11_TEAM_3, PLAYER_11_TEAM_4 },
	{ PLAYER_12_TEAM_1, PLAYER_12_TEAM_2, PLAYER_12_TEAM_3, PLAYER_12_TEAM_4 },
	{ PLAYER_13_TEAM_1, PLAYER_13_TEAM_2, PLAYER_13_TEAM_3, PLAYER_13_TEAM_4 },
	{ PLAYER_14_TEAM_1, PLAYER_14_TEAM_2, PLAYER_14_TEAM_3, PLAYER_14_TEAM_4 },
	{ PLAYER_15_TEAM_1, PLAYER_15_TEAM_2, PLAYER_15_TEAM_3, PLAYER_15_TEAM_4 },
	{ PLAYER_16_TEAM_1, PLAYER_16_TEAM_2, PLAYER_16_TEAM_3, PLAYER_16_TEAM_4 },
};

const short rsc_playername_rscindices[PLAYER_MAX_COUNT] = {
	PLAYER_1,
	PLAYER_2,
	PLAYER_3,
	PLAYER_4,
	PLAYER_5,
	PLAYER_6,
	PLAYER_7,
	PLAYER_8,
	PLAYER_9,
	PLAYER_10,
	PLAYER_11,
	PLAYER_12,
	PLAYER_13,
	PLAYER_14,
	PLAYER_15,
	PLAYER_16
};

/************************************************************
 *** void rsc_flags_revive(OBJECT *tree)
 ************************************************************/
static void rsc_flags_revive(OBJECT *tree) {
int j;
int i;

    /* set the buttons for the revive time (Fast/Slow) */
    if(revive_time == TIME_REVIVE_FAST) {
        i = REVIVE_FAST;
        j = REVIVE_SLOW;
    } else {
        i = REVIVE_SLOW;
        j = REVIVE_FAST;
    }
    tree[i].ob_state |= SELECTED;
    tree[j].ob_state &= ~SELECTED;

    /* set the 3 buttons for the revive lives (1,2,3) */
    for(i = 0; i < PLAYER_MAX_LIVES; i++) {
        if(i+1 == revive_lives) {
            tree[revive_lives_rscindices[i]].ob_state |= SELECTED;
        } else {
            tree[revive_lives_rscindices[i]].ob_state &= ~SELECTED;
        }
    }
}

/************************************************************
 *** int do_preference_form(OBJECT *tree,int possibleDroneCount,int *droneCountPtr)
 ************************************************************/
int do_preference_form(OBJECT *tree,int possibleDroneCount,int *droneCountPtr) {
int obj_index;
short fo_ch;
short fo_cw;
short fo_cy;
short fo_cx;
int j;
int i;

    /* patch the title to have the number of machine(s) */
    ((char*)tree[PREF_TITLE].ob_spec)[0] = (machines_online < 10) ? ' ' : machines_online/10+'0';
    ((char*)tree[PREF_TITLE].ob_spec)[1] = machines_online%10+'0';
    ((char*)tree[PREF_TITLE].ob_spec)[10] = (machines_online == 1) ? ' ' : 's';
    
    /* set the buttons for the reload time (Fast/Slow) */
    if(reload_time == TIME_RELOAD_FAST) {
        i = RELOAD_FAST;
        j = RELOAD_SLOW;
    } else {
        i = RELOAD_SLOW;
        j = RELOAD_FAST;
    }
    tree[i].ob_state |= SELECTED;
    tree[j].ob_state &= ~SELECTED;

    /* set the buttons for the regen time (Fast/Slow) */
    if(regen_time == TIME_REGEN_FAST) {
        i = REGEN_FAST;
        j = REGEN_SLOW;
    } else {
        i = REGEN_SLOW;
        j = REGEN_FAST;
    }
    tree[i].ob_state |= SELECTED;
    tree[j].ob_state &= ~SELECTED;

    /* update the other flags (not sure why this is a subroutine) */
    rsc_flags_revive(tree);

    /* reset all drone numbers to 0, if there are too many */
    if(rsc_drones[0]+rsc_drones[1]+rsc_drones[2] > possibleDroneCount) {
        for(i = 0; i < DRONE_TYPES; i++) {
            rsc_drones[i] = 0;
            ((char*)tree[drone_count_number_rscindices[i]].ob_spec)[0] = ' ';
            ((char*)tree[drone_count_number_rscindices[i]].ob_spec)[1] = '0';
        }
    }

    again:
    {
        form_center(tree, &fo_cx, &fo_cy, &fo_cw, &fo_ch);
        if(screen_rez == 0) { /* patch the position in color, because the dialog is too large */
            tree->ob_x = 5;
            fo_cx = 3;
            fo_cy -= 2;
            fo_cw += 4;
            fo_ch += 4;
        }
        form_dial(FMD_START, fo_cx, fo_cy, fo_cw, fo_ch, fo_cx, fo_cy, fo_cw, fo_ch);
        objc_draw(tree, 0, 999, fo_cx, fo_cy, fo_cw, fo_ch);
        for (;;) {
            /* do the play dialog */
            obj_index = form_do(tree, 0)&0x7fff;
            /* deselect the clicked exit button */
            tree[obj_index].ob_state &= ~SELECTED;
            if(obj_index == PREF_OK || obj_index == PREF_NAH) break; /* Yeah/Nah (Start or Cancel the game) */
            if(obj_index == PREF_TEAMS) {
            /* Team button? No, then we have to update the drone numbers */
                tree[PREF_TEAMS].ob_state |= SELECTED; /* select team button */
                update_names_in_rsc(rsc_drones); /* update the player names in the team dialog */
                objc_draw(tree, PREF_TEAMS, 999, fo_cx, fo_cy, fo_cw, fo_ch); /* redraw the team button */
                graf_mouse(M_OFF, NULL);
                redraw_window_background(wind_handle);
                graf_mouse(M_ON, NULL);
                form_center(rsrc_object_array[RSCTREE_TEAM_DIALOG], &fo_cx, &fo_cy, &fo_cw, &fo_ch);
                form_dial(FMD_START, fo_cx, fo_cy, fo_cw, fo_ch, fo_cx, fo_cy, fo_cw, fo_ch);
                objc_draw(rsrc_object_array[RSCTREE_TEAM_DIALOG], 0, 999, fo_cx, fo_cy, fo_cw, fo_ch);
                /* do the team dialog */
                form_do(rsrc_object_array[RSCTREE_TEAM_DIALOG], 0);
                /* deselect the exit button of the team dialog */
                rsrc_object_array[RSCTREE_TEAM_DIALOG][TEAM_OK].ob_state &= ~SELECTED;
                form_dial(FMD_FINISH, fo_cx, fo_cy, fo_cw, fo_ch, fo_cx, fo_cy, fo_cw, fo_ch);
                graf_mouse(M_OFF, NULL);
                redraw_window_background(wind_handle);
                graf_mouse(M_ON, NULL);
                goto again;
            }
            /* This code path is only called, if the up/down arrows for the drones are clicked. */
            /* MIDImaze has an auto-repeat on them. */
            for(i = 0; i < DRONE_TYPES; i++) {
                /* drone-up button clicked? */
                if(drone_count_up_rscindices[i] == obj_index) {
                    j = 1;
                    break;
                }
                /* drone-down button clicked? */
                if(drone_count_down_rscindices[i] == obj_index) {
                    j = -1;
                    break;
                }
            }
            /* in/decrement number of drones */
            rsc_drones[i] += j;
            /* clip to a value between 0 and the maximum number of drones allowed */
            if(rsc_drones[i] < 0 || rsc_drones[0]+rsc_drones[1]+rsc_drones[2] > possibleDroneCount) {
                rsc_drones[i] -= j; /* otherwise don't accept the change */
                Bconout(CON, 7); /* BELL sound */
            } else {
                /* update the number of drones in the dialog */
                ((char*)tree[drone_count_number_rscindices[i]].ob_spec)[0] = (rsc_drones[i] < 10) ? ' ' : rsc_drones[i]/10+'0';
                ((char*)tree[drone_count_number_rscindices[i]].ob_spec)[1] = rsc_drones[i]%10+'0';
            }
            /* redraw that number */
            objc_draw(tree, drone_count_box_rscindices[i], 999, fo_cx, fo_cy, fo_cw, fo_ch);
            /* ...and continue with the dialog seemlessly */
        }
    }

/* done: */
    form_dial(FMD_FINISH, fo_cx, fo_cy, fo_cw, fo_ch, fo_cx, fo_cy, fo_cw, fo_ch);
    if(obj_index == PREF_NAH) /* Nah/Cancel? */
        return FAILURE; /* => Do not start a game */

    /* update all preferences */
    reload_time = ((tree[RELOAD_FAST].ob_state & SELECTED) == SELECTED) ? TIME_RELOAD_FAST : TIME_RELOAD_SLOW;
    regen_time = ((tree[REGEN_FAST].ob_state & SELECTED) == SELECTED) ? TIME_REGEN_FAST : TIME_REGEN_SLOW;
    revive_time = ((tree[REVIVE_FAST].ob_state & SELECTED) == SELECTED) ? TIME_REVIVE_FAST : TIME_REVIVE_SLOW;
    /* is "Teams" selected? */
    if((tree[PREF_TEAMS].ob_state & SELECTED) == SELECTED) {
        team_flag = YES;
        friendly_fire = rsrc_object_array[RSCTREE_TEAM_DIALOG][FRIENDLY_FIRE].ob_state & SELECTED;
        /* assign players into the teams */
        for(i = 0; i < PLAYER_MAX_COUNT; i++) {
            for(j = 0; j < PLAYER_MAX_TEAMS; j++) {
                if((rsrc_object_array[RSCTREE_TEAM_DIALOG][team_group_rscindices[i][j]].ob_state & SELECTED) == SELECTED) {
                    player_data[i].ply_team = j;
                }
            }
        }
    } else {
        team_flag = NO;
    }
    if (tree[PREF_1LIFE].ob_state & SELECTED)
        revive_lives = 1;
    else if (tree[PREF_2LIVES].ob_state & SELECTED)
        revive_lives = 2;
    else
        revive_lives = PLAYER_MAX_LIVES;

    /* return the number of drones per type requested */
    for(i = 0; i < DRONE_TYPES; i++)
        droneCountPtr[i] = rsc_drones[i];

    return SUCCESS;
}

/************************************************************
 *** void update_names_in_rsc(int *droneList)
 ************************************************************/
void update_names_in_rsc(int *droneList) {
static const char *str_drone_type_br_array[DRONE_TYPES] = { "(Target)","(Standard)","(Ninja)" };
int i;
int j;
int mindex;
char *str_drone_name;

    for(i = machines_online; i < PLAYER_MAX_COUNT; i++)
        strcpy_srcdst("-------", (char*)rsrc_object_array[RSCTREE_TEAM_DIALOG][rsc_playername_rscindices[i]].ob_spec);
    mindex = machines_online;
    for(i = 0; i < DRONE_TYPES; i++) { /* iterate over all drone types */
        for(j = 0; droneList[i] > j; j++) { /* and then over the number of drones per type */
            str_drone_name = (char *)rsrc_object_array[RSCTREE_TEAM_DIALOG][rsc_playername_rscindices[mindex++]].ob_spec;
            strcpy_srcdst(str_drone_type_br_array[i], str_drone_name);
        }
    }
}
