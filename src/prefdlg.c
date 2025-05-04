/************************************************************
 *** The Play/Team dialog
 ************************************************************/
#include "globals.h"

static void update_names_in_rsc(int *droneList);

const short team_group_rscindices[PLAYER_MAX_COUNT][PLAYER_MAX_TEAMS] = { 
        {4,5,6,7},
        {10,11,12,13},
        {16,17,18,19},
        {22,23,24,25},
        {29,30,31,32},
        {35,36,37,38},
        {41,42,43,44},
        {47,48,49,50},
        {54,55,56,57},
        {60,61,62,63},
        {66,67,68,69},
        {72,73,74,75},
        {79,80,81,82},
        {85,86,87,88},
        {91,92,93,94},
        {97,98,99,100},
        };
const short rsc_playername_rscindices[PLAYER_MAX_COUNT] = { 3,9,15,21,28,34,40,46,53,59,65,71,78,84,90,96 };

static const short revive_lives_rscindices[PLAYER_MAX_LIVES] = { 42,43,44 }; /* how many lives after a revive: 1, 2 or 3 */
static int rsc_drones[DRONE_TYPES] = { 0,0,0 }; /* 3 drone types: integer values, how many of each type */
static const short drone_count_up_rscindices[DRONE_TYPES] = { 11,16,21 }; /* 3 drone types: up arrows to increase their numbers */
static const short drone_count_down_rscindices[DRONE_TYPES] = { 12,17,22 }; /* 3 drone types: down arrows to decrease their numbers */
static const short drone_count_number_rscindices[DRONE_TYPES] = { 14,19,24 }; /* 3 drone types: actual number value */
static const short drone_count_box_rscindices[DRONE_TYPES] = { 10,15,20 }; /* 3 drone types: box to redraw the cell */

/************************************************************
 *** void rsc_flags_revive(OBJECT *tree)
 ************************************************************/
static void rsc_flags_revive(OBJECT *tree) {
int j;
int i;

    /* set the buttons for the revive time (Fast/Slow) */
    if(revive_time == TIME_REVIVE_FAST) {
        i = 39;
        j = 38;
    } else {
        i = 38;
        j = 39;
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
int fo_ch;
int fo_cw;
int fo_cy;
int fo_cx;
int j;
int i;

    /* patch the title to have the number of machine(s) */
    ((char*)tree[45].ob_spec)[0] = (machines_online < 10) ? ' ' : machines_online/10+'0';
    ((char*)tree[45].ob_spec)[1] = machines_online%10+'0';
    ((char*)tree[45].ob_spec)[10] = (machines_online == 1) ? ' ' : 's';
    
    /* set the buttons for the reload time (Fast/Slow) */
    if(reload_time == TIME_RELOAD_FAST) {
        i = 29;
        j = 30;
    } else {
        i = 30;
        j = 29;
    }
    tree[i].ob_state |= SELECTED;
    tree[j].ob_state &= ~SELECTED;

    /* set the buttons for the regen time (Fast/Slow) */
    if(regen_time == TIME_REGEN_FAST) {
        i = 34;
        j = 33;
    } else {
        i = 33;
        j = 34;
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

    do {
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
        do {
            /* do the play dialog */
            obj_index = form_do(tree, 0)&0x7fff;
            /* deselect the clicked exit button */
            tree[obj_index].ob_state &= ~SELECTED;
            if(obj_index == 49 || obj_index == 48) goto done; /* Yeah/Nah (Start or Cancel the game) */
            if(obj_index != 54) break; /* Team button? No, then we have to update the drone numbers */

            tree[54].ob_state |= SELECTED; /* select team button */
            update_names_in_rsc(rsc_drones); /* update the player names in the team dialog */
            objc_draw(tree, 54, 999, fo_cx, fo_cy, fo_cw, fo_ch); /* redraw the team button */
            graf_mouse(M_OFF, 0);
            redraw_window_background(wind_handle);
            graf_mouse(M_ON, 0);
            form_center(rsrc_object_array[RSCTREE_TEAM_DIALOG], &fo_cx, &fo_cy, &fo_cw, &fo_ch);
            form_dial(FMD_START, fo_cx, fo_cy, fo_cw, fo_ch, fo_cx, fo_cy, fo_cw, fo_ch);
            objc_draw(rsrc_object_array[RSCTREE_TEAM_DIALOG], 0, 999, fo_cx, fo_cy, fo_cw, fo_ch);
            /* do the team dialog */
            form_do(rsrc_object_array[RSCTREE_TEAM_DIALOG], 0);
            /* deselect the exit button of the team dialog */
            rsrc_object_array[RSCTREE_TEAM_DIALOG][101].ob_state &= ~SELECTED;
            form_dial(FMD_FINISH, fo_cx, fo_cy, fo_cw, fo_ch, fo_cx, fo_cy, fo_cw, fo_ch);
            graf_mouse(M_OFF, 0);
            redraw_window_background(wind_handle);
            graf_mouse(M_ON, 0);
        } while(1);
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
    } while(1);
done:
    form_dial(FMD_FINISH, fo_cx, fo_cy, fo_cw, fo_ch, fo_cx, fo_cy, fo_cw, fo_ch);
    if(obj_index == 48) /* Nah/Cancel? */
        return FAILURE; /* => Do not start a game */

    /* update all preferences */
    reload_time = ((tree[29].ob_state & SELECTED) == SELECTED) ? TIME_RELOAD_FAST : TIME_RELOAD_SLOW;
    regen_time = ((tree[34].ob_state & SELECTED) == SELECTED) ? TIME_REGEN_FAST : TIME_REGEN_SLOW;
    revive_time = ((tree[39].ob_state & SELECTED) == SELECTED) ? TIME_REVIVE_FAST : TIME_REVIVE_SLOW;
    /* is "Teams" selected? */
    if((tree[54].ob_state & SELECTED) == SELECTED) {
        team_flag = YES;
        friendly_fire = rsrc_object_array[RSCTREE_TEAM_DIALOG][102].ob_state & SELECTED;
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
    revive_lives = ((tree[42].ob_state & SELECTED) == SELECTED) ? 1 : (tree[43].ob_state & SELECTED) == SELECTED ? 2 : PLAYER_MAX_LIVES;

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
static char *str_drone_name;
int j;
int mindex;
int i;

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
