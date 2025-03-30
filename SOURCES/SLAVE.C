/************************************************************
 *** The main loop for the slave. The features are controlled
 *** by the master via MIDI.
 ************************************************************/
#include "GLOBALS.H"


/************************************************************
 *** int slave_midicam_loop(int isMidicamFlag)
 ************************************************************/
int slave_midicam_loop(int isMidicamFlag) {
int returnCode = DISPATCH_QUIT;
int key_code;
int dontExitLoop;
int joystickActive;
int midiByte = 0;
int i;

    /* draw a window showing if we are MIDIcam or a slave */
    if((user_is_midicam = isMidicamFlag))
        rsc_draw_buttonless_dialog(RSCTREE_MIDICAM);
    else
        rsc_draw_buttonless_dialog(RSCTREE_SLAVE);

    /* make sure we get the shift status */
    _conterm_update_shift_status(1);

    joystickActive = YES;
    dontExitLoop = YES;
    while(1) {
        /* remove all pending MIDI bytes */
        while(Bconstat(MIDI))
            Bconin(MIDI);

        /* remove all pending key presses */
        while(Bconstat(CON))
            Bconin(CON);

        while(1) {
            /* echo a MIDI event to allow master detection */
            if(Bconstat(MIDI)) {
                if((midiByte = Bconin(MIDI)&0xff) == 0x00) {
                    Bconout(MIDI, 0x00);
                } else {
                    break;
                }
            }
            /* key pressed? */
            if(Bconstat(CON)) {
                key_code = (Bconin(CON)>>16) & 0x7fff; /* conterm is set to contain the modifier status in the top bits */
                if(key_code == 0x832 /* ALT-M */) {
                    joystickActive = NO;
                    redraw_window_background(wind_handle);
                    rsc_draw_buttonless_dialog(RSCTREE_MOUSE_CTRL);
                } else if(key_code == 0x824 /* ALT-J */) {
                    joystickActive = YES;
                    redraw_window_background(wind_handle);
                    rsc_draw_buttonless_dialog(RSCTREE_JOYSTICK_CTRL);
                } else if(key_code == 0x810 /* ALT-Q */) {
                    returnCode = DISPATCH_QUIT;
                    dontExitLoop = NO;
                    break;
                } else if(key_code == 0x813 /* ALT-R */) {
                    returnCode = DISPATCH_AUTOMATIC;
                    dontExitLoop = NO;
                    break;
                } else if(key_code == 0x81F /* ALT-S */) {
                    returnCode = DISPATCH_SOLO;
                    dontExitLoop = NO;
                    break;
                } else if(key_code == 0x82E /* ALT-C */) {
                    returnCode = DISPATCH_MIDICAM;
                    dontExitLoop = NO;
                    break;
                }
            }
        }
        if(!dontExitLoop) {
            redraw_window_background(wind_handle);
            break;
        }
        /* echo the MIDI byte */
        Bconout(MIDI, midiByte);

        redraw_window_background(wind_handle);

        /* About box  */
        if(midiByte == MIDI_ABOUT) {
            save_part_of_screen_for_dialog(1);
            redraw_window_background(wind_handle);
            /* disable the button, because the master controls it */
            rsrc_object_array[RSCTREE_ABOUT][5].ob_state |= DISABLED;
            rsrc_object_array[RSCTREE_ABOUT][5].ob_state &= ~SELECTED;
            rsrc_object_array[RSCTREE_ABOUT][5].ob_flags &= ~DEFAULT;
            rsc_draw_buttonless_dialog(RSCTREE_ABOUT);

        /* reset the scoreboard  */
        } else if(midiByte == MIDI_RESET_SCORE) {
            for(i = 0; i < PLAYER_MAX_COUNT; i++)
                score_table[i] = 0;

            update_notes_score(team_flag ? PLAYER_MAX_TEAMS : playerAndDroneCount);
            Setscreen(screen_ptr[1], screen_ptr[1], -1);

        /* count number of players online */
        } else if(midiByte == MIDI_COUNT_PLAYERS) {
            if((midiByte = get_midi(MIDI_DEFAULT_TIMEOUT)) < 0) break;
            if(user_is_midicam) {
                Bconout(MIDI, midiByte);
                midicam_player_count = midiByte-1;
            } else {
                Bconout(MIDI, (own_number = midiByte)+1); /* store our own number and send a new one to the next player */
            }
            if((midiByte = get_midi(MIDI_DEFAULT_TIMEOUT)) < 0) break;
            Bconout(MIDI, machines_online = midiByte);
            Bconin(MIDI); /* ignore the byte */
            Bconout(MIDI, MIDI_COUNT_PLAYERS);

        /* start/continue the game */
        } else if(midiByte == MIDI_START_GAME) {
            if(receive_datas() < 0) break;

            /* main loop for the whole game (shared with the master) */
            midiByte = game_loop(0, joystickActive);

            /* reset VT52 colors */
            BCON_DEFAULT_TEXT_COLOR();
            Vsync();
            Setcolor(0, 0); /* reset background color to black */
            copy_screen();

            if(joystickActive)
                exit_joystick();
            else
                exit_mouse();

            if(midiByte < 0) {
                if(midiByte == -1)
                    rsc_draw_buttonless_dialog(RSCTREE_MIDIRING_TIMEOUT);
                else if(midiByte == -2)
                    rsc_draw_buttonless_dialog(RSCTREE_GAME_TERMINATED);
                else
                    rsc_draw_buttonless_dialog(RSCTREE_MAZE_ERROR);
            }
            update_notes_score(team_flag ? PLAYER_MAX_TEAMS : playerAndDroneCount);
            Setscreen(screen_ptr[1], screen_ptr[1], -1);

        /* allow the user to enter a name for their player */
        } else if(midiByte == MIDI_NAME_DIALOG) {
            if((midiByte = get_midi(MIDI_DEFAULT_TIMEOUT)) < 0) break;
            if(user_is_midicam) {
                Bconout(MIDI, midiByte);
                midicam_player_count = midiByte-1;
            } else {
                Bconout(MIDI, (own_number = midiByte)+1);
            }
            if((midiByte = get_midi(MIDI_DEFAULT_TIMEOUT)) < 0) break;
            Bconout(MIDI, machines_online = midiByte);
            if(user_is_midicam) {
                for(i = 0; i < machines_online; i++) {
                    Bconout(MIDI, 0);
                    Bconin(MIDI);
                }
                midi_send_playernames();
            } else {
                playername_edit_dialog(0);
            }
        } else {
            break;
        }
    }
    _conterm_update_shift_status(0);
    return returnCode;
}

/************************************************************
 *** void playername_edit_dialog(int isSolo)
 ************************************************************/
void playername_edit_dialog(int isSolo) {
static char *playerNameStrPtr;
int fo_ch;
int fo_cw;
int fo_cy;
int fo_cx;
int i;

    playerNameStrPtr = (char*)rsrc_object_array[RSCTREE_TEAM_DIALOG][rsc_playername_rscindices[own_number]].ob_spec;
    strcpy_srcdst(playerNameStrPtr, rs_tedinfo[0].te_ptext);
    if(playerNameStrPtr[0] == '(' || playerNameStrPtr[0] == '-') { /* if not set, put in a placeholder name */
        /* The placeholder name is just Player# xx with xx being the number */
        strcpy_srcdst("Player# xx", playerNameStrPtr);
        own_number++;
        playerNameStrPtr[8] = ((own_number > 9) ? own_number/10 : own_number%10)+'0';
        playerNameStrPtr[9] = (own_number > 9) ? own_number%10+'0' : ' ';
        own_number--;
    }
    form_center(rsrc_object_array[RSCTREE_SET_NAME], &fo_cx, &fo_cy, &fo_cw, &fo_ch);
    form_dial(FMD_START, fo_cx, fo_cy, fo_cw, fo_ch, fo_cx, fo_cy, fo_cw, fo_ch);
    objc_draw(rsrc_object_array[RSCTREE_SET_NAME], 0, 999, fo_cx, fo_cy, fo_cw, fo_ch);
    /* Allow user to enter a new name */
    form_do(rsrc_object_array[RSCTREE_SET_NAME], 0);
    /* deselect the exit button */
    rsrc_object_array[RSCTREE_SET_NAME][2].ob_state &= ~SELECTED;
    form_dial(FMD_FINISH, fo_cx, fo_cy, fo_cw, fo_ch, fo_cx, fo_cy, fo_cw, fo_ch);
    if(!own_number)
        graf_mouse(M_OFF, 0);
    redraw_window_background(wind_handle);
    if(!own_number)
        graf_mouse(M_ON, 0);

    /* update the name in the team dialog (which contains the player names) */
    strcpy_srcdst(rs_tedinfo[0].te_ptext, playerNameStrPtr);
    if(isSolo) return;
    /* in ring mode, wait for all player to be done (no timeout!) */
    for(i = 0; i < machines_online; i++) {
        Bconout(MIDI, 0);
        Bconin(MIDI);
    }
    /* send playername throughout the ring for everybody */
    midi_send_playernames();
}
