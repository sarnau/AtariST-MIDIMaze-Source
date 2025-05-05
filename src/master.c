/************************************************************
 *** The main loop for the master or in solo mode.
 ************************************************************/
#include "globals.h"

short maze_loaded_flag = FALSE;
short dummy;

/************************************************************
 *** int master_solo_loop(int isSolo)
 ************************************************************/
int master_solo_loop(int isSolo) {
char mazePathAndName[100]; /* path + filename */
char filename[20]; /* DOS format: 12345678.012 + NUL - so, technically 13 bytes should be fine */
char filepath[64]; /* max 5 folders deep. That was the limit for the original TOS anyway. */
int returnCode = DISPATCH_QUIT;
short which_events;
short keyboard_state;
short key_code;
short joystickActive;
short midiByte = 0;
short prefReturnCode;
short buffer[8];
short pbutton;
short menuItemId;
short menuTitleId;
short dontExitLoop;
int j;
int i;

    /* joystick is the default, mouse the the optional choice */
    joystickActive = YES;

    /* remove all pending key presses */
    while(Bconstat(CON))
        Bconin(CON);

    if(isSolo)
        form_alert(1, "[1][ |This is a|  SOLO| machine.][OK]");
    else
        form_alert(1, "[1][ |This is the|  MASTER| machine.][OK]");

    graf_mouse(ARROW, NULL);
    graf_mouse(M_ON, NULL);

    own_number = 0;
    user_is_midicam = NO;

    /* Defaults for the preferences: */
    reload_time = TIME_RELOAD_FAST;
    regen_time = TIME_REGEN_FAST;
    revive_time = TIME_REVIVE_FAST;
    revive_lives = 2;
    team_flag = NO;
    /* no drones */
    active_drones_by_type[0] = active_drones_by_type[1] = active_drones_by_type[2] = 0;

    /* default path for the file selector to load a maze */
    i = Dgetdrv();
    filepath[0] = i+'A';
    filepath[1] = ':';
    Dgetpath(&filepath[2], i+1);
    for(i = 0; filepath[i]; i++) {}
    filepath[i++] = '\\';
    filepath[i++] = '*';
    filepath[i++] = '.';
    filepath[i++] = 'M';
    filepath[i++] = 'A';
    filepath[i++] = 'Z';
    filepath[i++] = 0;
    filename[0] = 0;

    /* load a maze, if there is not already a maze loaded */
    if(!maze_loaded_flag)
        maze_loaded_flag = LoadMaze("MIDIMAZE.MAZ", "MIDIMAZE.MAZ") >= 0;

    dontExitLoop = YES;
    while(dontExitLoop) {

        /* remove all pending key presses */
        while(Bconstat(CON))
            Bconin(CON);

        which_events = evnt_multi(MU_KEYBD|MU_MESAG, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, buffer, 0, 0, &dummy, &dummy, &dummy, &keyboard_state, &key_code, &dummy);

        if((which_events&MU_KEYBD) == MU_KEYBD) {
            if((key_code&0xff) == 13) goto PLAY_GAME;
            key_code = (key_code>>8)|(keyboard_state<<8);
            if(key_code == 0x832 /* ALT-M */) {
                joystickActive = NO;
                form_alert(1, "[1][ |Mouse control| selected.][OK]");
            } else if(key_code == 0x824 /* ALT-J */) {
                joystickActive = YES;
                form_alert(1, "[1][ |Joystick control|  selected.][OK]");
            } else if(key_code == 0x813 /* ALT-R */) {
                returnCode = DISPATCH_AUTOMATIC;
                dontExitLoop = NO;
            } else if(key_code == 0x81F /* ALT-S */) {
                returnCode = DISPATCH_SLAVE;
                dontExitLoop = NO;
            }

        } else { /* MU_MESAG case. Because we only asked for 2 types, we do not need to check */

            switch(buffer[0]) {
            case MN_SELECTED: /* select a menu item */
                redraw_window_background(wind_handle);
                menuTitleId = buffer[3];
                menuItemId = buffer[4];
                switch(menuTitleId) {
                case 3: /* About menu */
                    switch(menuItemId) {
                    case 7: /* About */
                        if(!isSolo) {
                            /* remove all pending MIDI bytes */
                            while(Bconstat(MIDI))
                                Bconin(MIDI);

                            Bconout(MIDI, MIDI_ABOUT);
                            get_midi(MIDI_DEFAULT_TIMEOUT);
                        }
                        save_part_of_screen_for_dialog(1);
                        graf_mouse(M_OFF, NULL);
                        redraw_window_background(wind_handle);
                        graf_mouse(M_ON, NULL);
                        form_alert(1, "[2][ |What about it?][tee-hee]");
                        rsrc_object_array[RSCTREE_ABOUT][5].ob_state &= ~(SELECTED|DISABLED);
                        rsrc_object_array[RSCTREE_ABOUT][5].ob_flags |= DEFAULT;
                        do_about_dialog(RSCTREE_ABOUT);
                        break;
                    default:
                        break;
                    }
                    menu_tnormal(rsrc_object_array[RSCTREE_MENU], 3, 1);
                    break;
                case 4: /* MAZE menu */
                    switch(menuItemId) {

                    case 16: /* Load a maze */
                        fsel_input(filepath, filename, &pbutton);
                        if(pbutton) {
                            graf_mouse(HOURGLASS, NULL);
                            /* This code is in every Atari ST app dealing with fsel_input(): */
                            /* Copy the path into a buffer */
                            i = 0;
                            j = 0;
                            while((mazePathAndName[i++] = filepath[j++]) != 0) {}
                            /* search for the last '\' (before the '*.MAZ' filter) */
                            while(!(--i <= 0 || mazePathAndName[i] == '\\')) {}
                            i++; /* move just behind that one */
                            /* now append the filename to the path */
                            j = 0;
                            while((mazePathAndName[i++] = filename[j++]) != 0) {}
                            /* ...and finally try to load the maze */
                            maze_loaded_flag = LoadMaze(mazePathAndName, filename) >= 0;
                            graf_mouse(ARROW, NULL);
                        }
                        break;

                    case 17: /* Reset Score */
                        if(!isSolo) {
                            /* remove all pending MIDI bytes */
                            while(Bconstat(MIDI))
                                Bconin(MIDI);

                            Bconout(MIDI, MIDI_RESET_SCORE);
                            get_midi(MIDI_DEFAULT_TIMEOUT);
                        }
                        for(i = 0; i < PLAYER_MAX_COUNT; i++)
                            score_table[i] = 0;
                        graf_mouse(M_OFF, NULL);
                        update_notes_score(team_flag ? PLAYER_MAX_TEAMS : playerAndDroneCount);
                        Setscreen(screen_ptr[1], screen_ptr[1], -1);
                        graf_mouse(M_ON, NULL);
                        break;

                    case 22: /* Set Names */
                        if(!isSolo) {
                            /* remove all pending MIDI bytes */
                            while(Bconstat(MIDI))
                                Bconin(MIDI);

                            Bconout(MIDI, MIDI_NAME_DIALOG);
                            if((midiByte = get_midi(MIDI_DEFAULT_TIMEOUT)) < 0) {
                                MIDIRING_BOOBOO:
                                form_alert(1, "[1][ |MIDI ring|boo-boo][Oh no!]");
                                break;
                            }
                            Bconout(MIDI, 1);
                            if((midiByte = get_midi(MIDI_DEFAULT_TIMEOUT)) < 0) goto MIDIRING_BOOBOO;
                            Bconout(MIDI, machines_online = midiByte);
                            if(get_midi(MIDI_DEFAULT_TIMEOUT) < 0) goto MIDIRING_BOOBOO;
                            if(machines_online > PLAYER_MAX_COUNT) {
                                form_alert(1, "[3][ |Too many|machines|on-line!][Oops]");
                                break;
                            }
                        } else {
                            machines_online = 1;
                        }
                        playername_edit_dialog(isSolo);
                        break;

                    case 19: /* Play */
                    PLAY_GAME:
                        wind_update(BEG_UPDATE);
                        if(!maze_loaded_flag) {
                            wind_update(END_UPDATE);
                            form_alert(1, "[3][ |Please load|a maze first][OK]");
                            break;
                        }
                        if(!isSolo) {
                            /* remove all pending MIDI bytes */
                            while(Bconstat(MIDI))
                                Bconin(MIDI);

                            /* Count the number of machines online */
                            Bconout(MIDI, MIDI_COUNT_PLAYERS);
                            if((midiByte = get_midi(MIDI_DEFAULT_TIMEOUT)) < 0) {
                              MIDIRING_BOOBOO3:
                                wind_update(END_UPDATE);
                                MIDIRING_BOOBOO2:
                                form_alert(1, "[1][ |MIDI ring|boo-boo][Oh no!]");
                                break;
                            }
                            Bconout(MIDI, 1); /* we are the first machine, then everybody increments it */
                            if((midiByte = get_midi(MIDI_DEFAULT_TIMEOUT)) < 0) goto MIDIRING_BOOBOO3;
                            /* echo the final number to everybody */
                            Bconout(MIDI, machines_online = midiByte);
                            /* and when the number comes back: ignore it (it didn't change anyway) */
                            if(get_midi(MIDI_DEFAULT_TIMEOUT) < 0) goto MIDIRING_BOOBOO3;
                            if(machines_online > PLAYER_MAX_COUNT) {
                                wind_update(END_UPDATE);
                                form_alert(1, "[3][ |Too many|machines|on-line!][Oops]");
                                break;
                            }
                        } else {
                            machines_online = 1;
                        }
                        prefReturnCode = do_preference_form(rsrc_object_array[RSCTREE_PLAY_DIALOG], PLAYER_MAX_COUNT-machines_online, active_drones_by_type);
                        wind_update(END_UPDATE);
                        menu_tnormal(rsrc_object_array[RSCTREE_MENU], 4, 1);
                        graf_mouse(M_OFF, NULL);
                        redraw_window_background(wind_handle);
                        if(!isSolo) {
                            Bconout(MIDI, MIDI_COUNT_PLAYERS);
                            if((midiByte = get_midi(MIDI_DEFAULT_TIMEOUT)) < 0)
							{
								graf_mouse(M_ON, NULL);
								goto MIDIRING_BOOBOO2;
							}
                        }
                        if(prefReturnCode < 0) {
                            graf_mouse(M_ON, NULL);
                            break;
                        }
                        if(!isSolo) {
                            Bconout(MIDI, MIDI_START_GAME);
                            if(get_midi(MIDI_DEFAULT_TIMEOUT) < 0)
							{
								graf_mouse(M_ON, NULL);
								goto MIDIRING_BOOBOO2;
							}
                        }
                        if(!isSolo) {
                            if(send_datas() < 0)
							{
								graf_mouse(M_ON, NULL);
                                goto MIDIRING_BOOBOO2;
                            }
                        }

                        /* play the game! */
                        midiByte = game_loop(isSolo, joystickActive);

                        /* Reset text colors (not sure why, because it seems to be set in every use case anyway) */
						BCON_SETFCOLOR(screen_rez ? COLOR_SILVER_INDEX : COLOR_WHITE_INDEX);
						BCON_SETBCOLOR(COLOR_BLACK_INDEX);

                        /* remove all pending key presses */
                        while(Bconstat(CON))
                            Bconin(CON);

                        copy_screen();
                        if(joystickActive)
                            exit_joystick();
                        else
                            exit_mouse();
                        graf_mouse(M_ON, NULL);
                        if(midiByte < 0) {
                            if(midiByte == -1)
                                form_alert(1, "[3][ |The MIDI ring|has timed out.][Uh-oh]");
                            else if(midiByte == -2)
                                form_alert(1, "[3][ |Game terminated.][OK]");
                            else
                                form_alert(1, "[3][ |Guru bu-bu:|(maze too small?)][ok]");
                        }
                        graf_mouse(M_OFF, NULL);
                        update_notes_score(team_flag ? PLAYER_MAX_TEAMS : playerAndDroneCount);
                        Setscreen(screen_ptr[1], screen_ptr[1], -1);
                        graf_mouse(M_ON, NULL);
                        break;

                    case 20: /* Quit */
                        midiByte = form_alert(2, "[2][ |Do you really|wanna quit?][Yeah|Nah]");
                        if(midiByte == 1) {
                            returnCode = DISPATCH_QUIT;
                            dontExitLoop = NO;
                        }
                        break;

                    default:
                        break;
                    }
                    menu_tnormal(rsrc_object_array[RSCTREE_MENU], 4, 1);
                    break;
                default:
                    break;
                }
                break;
            case WM_NEWTOP: /* AES redraw requests of the window */
            case WM_TOPPED:
            case WM_REDRAW:
                graf_mouse(M_OFF, NULL);
                redraw_window_background(buffer[3]);
                graf_mouse(M_ON, NULL);
                break;
            default:
                break;
            }
        }
    }
    graf_mouse(M_OFF, NULL);
    return returnCode;
}
