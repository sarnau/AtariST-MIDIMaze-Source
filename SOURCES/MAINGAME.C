/************************************************************
 *** The main game loop, shared by all types: master, solo, slave and MIDIcam
 ************************************************************/
#include "GLOBALS.H"

short team_notes_color_table[PLAYER_MAX_TEAMS] = { COLOR_YELLOW_INDEX, COLOR_BLUE_INDEX, COLOR_ORANGE_INDEX, COLOR_GREEN_INDEX };
/* Atari ST colors are RGB (3 bit each) */
const unsigned short colortable[16] = { 0x000, /* COLOR_BLACK_INDEX */
                                        0x566, /* COLOR_SILVER_INDEX */
                                        0x455, /* COLOR_MAGNESIUM_INDEX */
                                        0x227, /* COLOR_BLUE_INDEX */
                                        0x444, /* COLOR_GREY_INDEX */
                                        0x122, /* COLOR_DKGREEN_INDEX */
                                        0x344, /* COLOR_ALUMINIUM_INDEX */
                                        0x233, /* COLOR_STEEL_INDEX */
                                        0x770, /* COLOR_YELLOW_INDEX */
                                        0x740, /* COLOR_ORANGE_INDEX */
                                        0x403, /* COLOR_PURPLE_INDEX */
                                        0x707, /* COLOR_MAGENTA_INDEX */
                                        0x057, /* COLOR_LTBLUE_INDEX */
                                        0x060, /* COLOR_GREEN_INDEX */
                                        0x700, /* COLOR_RED_INDEX */
                                        0x777  /* COLOR_WHITE_INDEX */
                                        };
short color_cnv_back[PLAYER_MAX_COUNT];
short color_cnv_frame[PLAYER_MAX_COUNT];
short player_joy_table[PLAYER_MAX_COUNT];
short areDronesActiveFlag;
short protectionIsValid;
short display_2d_map_flag;
short we_dont_have_a_winner;
int active_drones_by_type[DRONE_TYPES];
short playerAndDroneCount;
short machines_online;
short own_number;

/************************************************************
 *** int game_loop(int isSolo,int isJoystick)
 ************************************************************/
int game_loop(int isSolo,int isJoystick) {
int bwColorFlag;
int lastJoystickButton;
int joystickButton = 0;
int savedDisplay_2d_map_flag;
int midiCamRotateIndex;
int unused_18 __attribute__((unused));
int playerIndex;
int unused_14 __attribute__((unused));
int joystickDirection = 0;
int tempVar;
int k;
int j;
int i;
int currentScore;

    playerIndex = 0;
    unused_18 = 0;
    midiCamRotateIndex = 0;
    lastJoystickButton = 0;
    bwColorFlag = FALSE; /* flashing screen, when the player was hit */
    screen_flag = 0;
    display_2d_map_flag = NO; /* 2D map is off */
    if(!protectionIsValid) {
        machines_online = -2; /* this will trigger a crash further down, because of a negative array index */
        playerAndDroneCount = -2; /* this however is useless, because it is set to the correct value a few lines below anyway. */
    }

    if(user_is_midicam) {
        own_number = 0;
        midicam_timer_counter = 50;
        midicam_autoselect_player_flag = YES;
    }

    /* reset the scores for the current game */
    set_screen_offs(score_notes_wind_offset);
    for(i = 0; i < PLAYER_MAX_COUNT; i++)
        draw_one_note_score(i, -1);

    set_screen_offs(viewscreen_wind_offset);
    playerAndDroneCount = active_drones_by_type[0]+active_drones_by_type[1]+active_drones_by_type[2]+machines_online;
    areDronesActiveFlag = playerAndDroneCount > machines_online;

    /* define the player/team colors */
    if(screen_rez) {
        for(i = 0; i < PLAYER_MAX_TEAMS; i++)
            team_notes_color_table[i] = i+COLOR_BLUE_INDEX;
    }
    for(i = 0; i < playerAndDroneCount; i++) {
        static const unsigned short color_ply_back[PLAYER_MAX_COUNT] = {  COLOR_YELLOW_INDEX, COLOR_BLUE_INDEX, COLOR_ORANGE_INDEX, COLOR_GREEN_INDEX, COLOR_MAGENTA_INDEX, COLOR_LTBLUE_INDEX, COLOR_PURPLE_INDEX, COLOR_WHITE_INDEX, COLOR_YELLOW_INDEX, COLOR_BLUE_INDEX, COLOR_ORANGE_INDEX, COLOR_GREEN_INDEX, COLOR_MAGENTA_INDEX, COLOR_LTBLUE_INDEX, COLOR_PURPLE_INDEX, COLOR_WHITE_INDEX };
        static const unsigned short color_ply_frame[PLAYER_MAX_COUNT] = {  COLOR_BLACK_INDEX, COLOR_BLACK_INDEX, COLOR_BLACK_INDEX, COLOR_BLACK_INDEX, COLOR_BLACK_INDEX, COLOR_BLACK_INDEX, COLOR_BLACK_INDEX, COLOR_BLACK_INDEX, COLOR_PURPLE_INDEX, COLOR_MAGENTA_INDEX, COLOR_PURPLE_INDEX, COLOR_MAGENTA_INDEX, COLOR_PURPLE_INDEX, COLOR_PURPLE_INDEX, COLOR_MAGENTA_INDEX, COLOR_PURPLE_INDEX };
        color_cnv_back[i] = screen_rez ? i+COLOR_BLUE_INDEX : color_ply_back[i];
        color_cnv_frame[i] = screen_rez ? COLOR_BLACK_INDEX : color_ply_frame[i];
    }

    /* assign the drone types */
    j = machines_online;
    for(k = 0; k < DRONE_TYPES; k++) {
        for(i = 0; active_drones_by_type[k] > i; i++) {
            switch(k) {
            case 0: player_data[j].dr_type = DRONE_TARGET; break;
            case 1: player_data[j].dr_type = DRONE_STANDARD; break;
            case 2: player_data[j].dr_type = DRONE_NINJA; break;
            }
            j++;
        }
    }
    /* set the drone colors om team mode */
    if(team_flag) {
        for(i = 0; i < playerAndDroneCount; i++) {
            color_cnv_back[i] = team_notes_color_table[player_data[i].ply_team];
            color_cnv_frame[i] = COLOR_BLACK_INDEX;
        }
    }

    /* for the MIDIcam we set simpler colors. These cams are invisible anyway. */
    if(user_is_midicam) {
        for(i = playerAndDroneCount; i < PLAYER_MAX_COUNT; i++) {
            color_cnv_back[i] = color_cnv_frame[i] = screen_rez ? COLOR_MAGNESIUM_INDEX : COLOR_STEEL_INDEX;
            player_data[i].ply_hitflag = FALSE;
        }
    }

    /* configure all drones */
    drone_setup(machines_online);

    if(!own_number && !user_is_midicam) {
        if(!isSolo) { /* network play */
            /* The master generates a 16-bit random number, which is shared with all players */
            /* to use as a common seed for the random generator */
            Bconout(MIDI, (int)Random());
            _random_seed = get_midi(MIDI_DEFAULT_TIMEOUT);
            if(_random_seed < 0) {
                return FAILURE;
            }
            Bconout(MIDI, (int)Random());
            tempVar = get_midi(MIDI_DEFAULT_TIMEOUT);
            if(tempVar < 0) {
                return FAILURE;
            }
            _random_seed = (_random_seed<<8)+tempVar;
        } else {
            /* a master just generates one random number seed */
            _random_seed = (Random()<<8)+Random();
        }
    } else {
        /* a slave received the random generator seed from the master */
        _random_seed = get_midi(MIDI_DEFAULT_TIMEOUT);
        if(_random_seed < 0)
            return FAILURE;
        Bconout(MIDI, _random_seed);
        tempVar = get_midi(MIDI_DEFAULT_TIMEOUT);
        if(tempVar < 0)
            return FAILURE;
        Bconout(MIDI, tempVar);
        _random_seed = (_random_seed<<8)+tempVar;
    }

    /* initialize all player/drone structures and position them into the maze */
    if(!init_all_player(playerAndDroneCount, areDronesActiveFlag))
        return -3; /* maze too small for all players */

    if(user_is_midicam) {
        /* find a good start position for the MIDIcam positions */
        position_midicam_users();
    }

    /* reset team scores */
    if(team_flag) {
        for(i = 0; i < PLAYER_MAX_TEAMS; i++)
            team_scores[i] = currently_displayed_notes_score[i] = -1;
        for(i = 0; i < playerAndDroneCount; i++)
            team_scores[player_data[i].ply_team] = 0;
    }
    currentScore = 0;

    /* reset, once we have a player/team who won */
    we_dont_have_a_winner = YES;

    /* erase the window with all kills  */
    set_screen_offs(popchart_wind_offset);
    if(screen_rez)
        blit_fill_box_bw(0, 0, 242, 74, COLOR_BLACK_INDEX);
    else
        blit_fill_box_color(0, 0, 121, 37, COLOR_BLACK_INDEX);

    switch_logbase();
    if(screen_rez)
        blit_fill_box_bw(0, 0, 242, 74, COLOR_BLACK_INDEX);
    else
        blit_fill_box_color(0, 0, 121, 37, COLOR_BLACK_INDEX);

    if(user_is_midicam) {
        /* Print an inverted MIDIcam into the "kills window" (which is not used for MIDIcam) */
        BCON_SETCURSOR(9, screen_rez ? 48 : 24);
        BCON_DEFAULT_TEXT_COLOR();
        bconout_string(" M I D I C A M ");
        switch_logbase();
        BCON_SETCURSOR(9, screen_rez ? 48 : 24);
        BCON_DEFAULT_TEXT_COLOR();
        bconout_string(" M I D I C A M ");
        switch_logbase();
    }

    /* draw the live status */
    set_screen_offs(viewscreen_wind_offset);
    update_happiness_quotient_indicator();

    /* draw the scoreboard */
    set_screen_offs(viewscreen_wind_offset);
    draw_all_notes_score(team_flag ? PLAYER_MAX_TEAMS : playerAndDroneCount);

    /* draw the maze at the start with all players visible */
    set_screen_offs(viewscreen_wind_offset);
    draw_2Dmap();
    for(i = 0; i < playerAndDroneCount; i++)
        set_ply_2Dmap(i);

    /* 5s delay to allow the player to see the map */
    for(i = 0; (screen_rez ? 350 : 300) > i; i++)
        Vsync();

    if(isJoystick)
        init_joystick();
    else
        init_mouse();

    /* remove all pending key presses */
    while(Bconstat(CON))
        Bconin(CON);

    if(user_is_midicam) {
        /* print the currently tracked user into the "kills" window under "M I D I C A M" */
        midicam_print_current_player();
    }

    /* main loop till the game is over */
    do {
        /* did the current player got hit? */
        if(player_data[own_number].ply_hitflag) {
            Dosound((void*)sound_hit_ptr);
            /* flash the background color */
            if(screen_rez) {
                Vsync();
                Setcolor(0, 1); /* invert color */
                bwColorFlag = TRUE;
            } else {
                /* flash in the color of the game who hit the player */
                Setcolor(0, colortable[color_cnv_back[player_data[own_number].ply_gunman]]);
            }
        } else {
            if(screen_rez) {
                if(bwColorFlag) {
                    bwColorFlag = FALSE;
                    Setcolor(0, 0); /* do not invert the background color */
                }
            } else {
                Setcolor(0, 0); /* reset background color to black */
            }
        }

        /* update the score board, if necessary */
        draw_all_notes_score(team_flag ? PLAYER_MAX_TEAMS : playerAndDroneCount);

        /* maze map requested? */
        if(display_2d_map_flag) {
            draw_2Dmap();
            if(user_is_midicam || DEBUG_2D_MAZE) { /* if MIDIcam, always show all players */
                for(i = 0; i < playerAndDroneCount; i++) {
                    if(player_data[i].ply_lives > 0) /* ignore dead players */
                        set_ply_2Dmap(i);
                }
            } else {
                /* only show the own player (when we are alive) */
                if(player_data[own_number].ply_lives > 0) {
                    set_ply_2Dmap(own_number);
                } else {
                    /* if the player is dead, then show all other players as well */
                    for(i = 0; i < playerAndDroneCount; i++) {
                        if(player_data[i].ply_lives > 0) /* ignore dead player */
                            set_ply_2Dmap(i);
                    }
                }
            }
            switch_screens();
            /* store all players in the maze */
            set_all_player(); /* position all players/drones into the maze (because draw_list() was not called) */
        } else {
            /* is the player actually alive? */
            if(player_data[own_number].ply_lives > 0) {
                /* then the 3D maze has to be drawn */
                make_draw_list(player_data[own_number].ply_y, player_data[own_number].ply_x, player_data[own_number].ply_dir);
                draw_list();

                /* Draw the crosshair - visible only when you are reloaded. A MIDIcam player never has one. */
                if(player_data[own_number].ply_reload == 0 && !user_is_midicam) {
                    switch_logbase();
                    if(screen_rez) {
                        blit_draw_vline_bw(99, 101, 158, COLOR_BLACK_INDEX);
                        blit_draw_vline_bw(99, 101, 162, COLOR_BLACK_INDEX);
                        blit_draw_hline_bw(159, 161, 98, COLOR_BLACK_INDEX);
                        blit_draw_hline_bw(159, 161, 102, COLOR_BLACK_INDEX);
                    } else {
                        blit_draw_vline_color(50, 51, 78, color_cnv_back[own_number]);
                        blit_draw_vline_color(50, 51, 80, color_cnv_back[own_number]);
                        blit_draw_vline_color(49, 49, 79, color_cnv_back[own_number]);
                        blit_draw_vline_color(52, 52, 79, color_cnv_back[own_number]);
                    }
                    switch_logbase();
                }

            } else {
                /* player is dead, draw the shape of the shooter */
                if(screen_rez) {
                    blit_fill_box_bw(0, 0, 319, 201, COLOR_SILVER_INDEX);
                    draw_shape(96, BODY_SHAPE_MAX_SIZE*2, BODY_SHAPE_FRONT_VIEW, BODY_SHAPE_NO_SHADOW*2, player_data[own_number].ply_gunman);
                } else {
                    blit_fill_box_color(0, 0, 159, 100, COLOR_STEEL_INDEX);
                    draw_shape(48, BODY_SHAPE_MAX_SIZE, BODY_SHAPE_FRONT_VIEW, BODY_SHAPE_NO_SHADOW, player_data[own_number].ply_gunman);
                }
                /* ...and some nice greetings */
                BCON_SETCURSOR(7, screen_rez ? 15 : 3);
                BCON_SETFCOLOR(COLOR_BLACK_INDEX);
                BCON_SETBCOLOR(screen_rez ? COLOR_SILVER_INDEX : COLOR_STEEL_INDEX);
                print_playername(player_data[own_number].ply_gunman);
                bconout_string(" says:");
                BCON_SETCURSOR(17, screen_rez ? 15 : 3);
                BCON_SETFCOLOR(COLOR_BLACK_INDEX);
                BCON_SETBCOLOR(screen_rez ? COLOR_SILVER_INDEX : COLOR_STEEL_INDEX);
                bconout_string("\"Have a nice day!\"");
                switch_screens();
                set_all_player(); /* position all players/drones into the maze (because draw_list() was not called) */
            }
        }
        /* update the score, if necessary */
        if(!user_is_midicam && player_data[own_number].ply_score != currentScore) {
            currentScore++;
            add_one_smily(currentScore, player_data[own_number].ply_looser);
        }

        /* stop, if a winner has been identified */
        if(!we_dont_have_a_winner) break;

        /* The MIDIcam player auto-rotate slowly by 90 degrees (they sit on a specific position) */
        if(user_is_midicam) {
            midiCamRotateIndex++;
            for(i = playerAndDroneCount; i < PLAYER_MAX_COUNT; i++)
                player_data[i].ply_dir += (midiCamRotateIndex&64) ? -1 : 1;
        }
        midicam_player_number = own_number;

        /* get joystick/mouse input */
        if(isJoystick) {
            joystickDirection = ask_joystick(1, &joystickButton);
            if(!user_is_midicam)
                player_joy_table[i = own_number] = (joystickButton<<4)|joystickDirection;
        } else {
            if(!user_is_midicam)
                player_joy_table[i = own_number] = ask_mouse();
        }

        if(user_is_midicam) {
            /* with the mouse-button toggle between 2D and 3D map */
            if(!lastJoystickButton && joystickButton) {
                display_2d_map_flag ^= 1;
                midicam_autoselect_player_flag = NO;
            }
            lastJoystickButton = joystickButton;

            /* left/right allows switching between active players */
            if(joystickDirection & JOYSTICK_UP) {
                if(++own_number >= PLAYER_MAX_COUNT)
                    own_number = 0;
                midicam_autoselect_player_flag = display_2d_map_flag = NO;
            }
            if(joystickDirection & JOYSTICK_DOWN) {
                if(--own_number < 0)
                    own_number = PLAYER_MAX_COUNT-1;
                midicam_autoselect_player_flag = display_2d_map_flag = NO;
            }

            /* up/down switches back to auto-selection of a player */
            if(joystickDirection & (JOYSTICK_LEFT|JOYSTICK_RIGHT)) {
                midicam_autoselect_player_flag = YES;
                midicam_timer_counter = 0;
            }
            /* check for further keyboard shortcuts used for the MIDIcam */
            midicam_keyboard_shortcuts();

        } else {
            /* regular player (not a MIDIcam) */

            if(Bconstat(CON)) {
                joystickDirection = Bconin(CON)&0xff;
                if(joystickDirection == ' ') { /* space toggles 2D and 3D map */
                    display_2d_map_flag ^= 1;
                } else if(joystickDirection == 27 && !own_number) { /* ESC allows to terminate the game */
                    player_joy_table[i] = MIDI_TERMINATE_GAME;
                }
                /* remove all pending key presses */
                while(Bconstat(CON))
                    Bconin(CON);
            }
        }

        if(user_is_midicam) {
            /* forward all MIDI bytes through our MIDIcam machine */
            i = midicam_player_count;
            do {
                player_joy_table[i] = get_midi(MIDI_DEFAULT_TIMEOUT);
                if(player_joy_table[i] < 0)
                    return FAILURE;
                Bconout(MIDI, player_joy_table[i]);
                if(--i < 0)
                    i = machines_online-1;
            } while(i != midicam_player_count);
        } else {
            if(!isSolo) {
                /* forward all joystick/key presses to all other machines in the MIDI ring */
                do {
                    Bconout(MIDI, player_joy_table[i]);
                    if(--i < 0)
                        i = machines_online-1;
                    player_joy_table[i] = get_midi(30);
                    if(player_joy_table[i] < 0)
                        return FAILURE;
                } while(i != own_number);
            }
        }

        /* move the drones */
        for(i = machines_online; i < playerAndDroneCount; i++)
            drone_action(i);

        /* reset the "player got hit" flags */
        for(i = 0; i < playerAndDroneCount; i++)
            player_data[i].ply_hitflag = FALSE;

        if(player_joy_table[0] == MIDI_TERMINATE_GAME) {
            Setcolor(0, 0); /* reset background color */
            if(!user_is_midicam && !own_number) { /* the master/solo player wants to quit the game */
                switch_logbase();
                if(isJoystick)
                    exit_joystick();
                else
                    exit_mouse();
                i = form_alert(1, "[1][ |Game suspended.| Continue?][Yeah|Nah]");
                if(isJoystick)
                    init_joystick();
                else
                    init_mouse();
                switch_logbase();
                if(isSolo) {
                    if(i != 1) /* "Nah" */
                        return -2; /* => game terminated */
                    goto check_terminate_continue;
                }
                /* send either continue or terminate to all slaves */
                Bconout(MIDI, (i == 1) ? MIDI_START_GAME : MIDI_TERMINATE_GAME);
                i = get_midi(MIDI_DEFAULT_TIMEOUT);
                if(i < 0)
                    return FAILURE;
            } else { /* show a buttonless dialog on the slaves */
                copy_screen();
                rsc_draw_buttonless_dialog(RSCTREE_SUSPENDED);
                /* forward response from the master (no timeout here) */
                i = Bconin(MIDI)&0xff;
                Bconout(MIDI, i);
                copy_screen();
                switch_logbase();
            }
            if(i != MIDI_START_GAME)
                return -2; /* => game terminated */
            check_terminate_continue:
            player_joy_table[0] = 0x00; /* remove input from Master, because MIDI_TERMINATE_GAME is not a valid joystick command */
        }

        /* save the flag, because move_player() might reset it */
        /* This is only needed for the MIDIcam */
        savedDisplay_2d_map_flag = display_2d_map_flag;

        /* move the player and all shots */
        /* every loop we start with a different player, so that player #0 will not always have the first "shot" */
        i = playerIndex;
        do {
            tempVar = move_player(i, player_joy_table[i], areDronesActiveFlag);
            if(tempVar < 0) /* player couldn't be positioned in the maze (too player players in a small maze) */
                return -3;

            if(!we_dont_have_a_winner) break;
            if(--i < 0)
                i = playerAndDroneCount-1;
        } while(i != playerIndex);
        if(++playerIndex == playerAndDroneCount)
            playerIndex = 0;

        if(user_is_midicam)
            display_2d_map_flag = savedDisplay_2d_map_flag;
    } while(1);

    Setcolor(0, 0); /* reset background color */
    /* 0.15s delay */
    for(i = 0; i < 10; i++)
        Vsync();

    /* Play the win/lost animation */
    for(i = 0; i < playerAndDroneCount; i++) {
        /* find the winner */
        if((team_flag ? team_scores[player_data[i].ply_team] : player_data[i].ply_score) == GAME_WIN_SCORE) {
            /* looser: shake left/right twice */
            static const short lose_anim[29] = { 0,1,2,3,3,2,1,0,19,18,17,17,18,19,0,1,2,3,3,2,1,0,19,18,17,17,18,19,0 };
            /* winner: turn around once */
            static const short winner_anim[29] = { 0,0,0,19,19,18,17,16,15,14,13,12,11,10,10,10,10,10,10,9,8,7,6,5,4,3,2,1,0 };
            for(j = 0; j < sizeof(lose_anim)/sizeof(lose_anim[0]); j++) {
                if(screen_rez) {
                    blit_fill_box_bw(0, 0, 319, 201, COLOR_SILVER_INDEX);
                    draw_shape(96, BODY_SHAPE_MAX_SIZE*2, team_flag ? player_data[own_number].ply_team == player_data[i].ply_team : own_number == i ? winner_anim[j] : lose_anim[j], BODY_SHAPE_NO_SHADOW*2, i);
                } else {
                    blit_fill_box_color(0, 0, 159, 100, COLOR_STEEL_INDEX);
                    draw_shape(48, BODY_SHAPE_MAX_SIZE, team_flag ? player_data[own_number].ply_team == player_data[i].ply_team : own_number == i ? winner_anim[j] : lose_anim[j], BODY_SHAPE_NO_SHADOW, i);
                }
                BCON_SETCURSOR(7, screen_rez ? 15 : 3);
                BCON_SETFCOLOR(COLOR_BLACK_INDEX);
                BCON_SETBCOLOR(screen_rez ? COLOR_SILVER_INDEX : COLOR_STEEL_INDEX);
                if(team_flag) {
                    if(player_data[own_number].ply_team == player_data[i].ply_team) {
                        bconout_string(" Your team wins!");
                    } else {
                        bconout_string(" Your team loses!");
                    }
                } else {
                    if(own_number == i) {
                        bconout_string("    You win!");
                    } else {
                        print_playername(i);
                        bconout_string(" wins!");
                    }
                }
                BCON_SETCURSOR(17, screen_rez ? 15 : 3);
                BCON_SETFCOLOR(COLOR_BLACK_INDEX);
                BCON_SETBCOLOR(screen_rez ? COLOR_SILVER_INDEX : COLOR_STEEL_INDEX);
                bconout_string("\"Have a nice day!\"");
                switch_screens();
                /* a slight delay for the animation */
                Vsync();
                if(screen_rez == 0)
                    Vsync();
            }
            break;
        }
    }

    /* increment the overall won game score (either per player/team) */
    if(team_flag) {
        if(++score_table[player_data[i].ply_team] > MAX_GAME_SCORE)
            score_table[player_data[i].ply_team] = 1;
    } else {
        if(++score_table[i] > MAX_GAME_SCORE)
            score_table[i] = 1;
    }

    /* is the player the winner or part of a winning team? */
    if(team_flag ? player_data[own_number].ply_team == player_data[i].ply_team : own_number == i) {
        switch_logbase();
        /* blink with the eye */
        if(screen_rez) {
            blit_fill_box_bw(170, 58, 189, 97, color_cnv_back[i]);
            blit_draw_shape_bw(164, 86, blinzshape_ptr+2*6, 2, 6, bw_fillpattern_table[color_cnv_frame[i]]);
        } else {
            blit_fill_box_color(85, 29, 94, 48, color_cnv_back[i]);
            blit_draw_shape_color(82, 43, blinzshape_ptr+1*6, 1, 6, col_setcolor_jumptable[color_cnv_frame[i]]);
        }
        switch_logbase();
        /* 0.15s delay */
        for(j = 0; j < 10; j++)
            Vsync();

        /* restore the normal shape without the eyelashes (this makes it a blink) */
        if(screen_rez)
            draw_shape(96, BODY_SHAPE_MAX_SIZE*2, BODY_SHAPE_FRONT_VIEW, BODY_SHAPE_NO_SHADOW*2, i);
        else
            draw_shape(48, BODY_SHAPE_MAX_SIZE, BODY_SHAPE_FRONT_VIEW, BODY_SHAPE_NO_SHADOW, i);

        switch_screens();
        /* 1.8s delay */
        for(i = 0; (screen_rez ? 127 : 109) > i; i++)
            Vsync();

    } else {
        switch_logbase();
        /* show the tongue to the looser  */
        if(screen_rez)
            blit_draw_shape_bw(142, 145, loosershape_ptr+36, 4, 9, bw_fillpattern_table[COLOR_MAGNESIUM_INDEX]);
        else
            blit_draw_shape_color(71, 72, loosershape_ptr+18, 2, 9, col_setcolor_jumptable[COLOR_RED_INDEX]);

        switch_logbase();
        /* 2s delay */
        for(i = 0; (screen_rez ? 140 : 120) > i; i++)
            Vsync();
    }
    
    /* update the team scoreboard */
    update_notes_score(team_flag ? PLAYER_MAX_TEAMS : playerAndDroneCount);

    /* remove all pending key presses */
    while(Bconstat(CON))
        Bconin(CON);

    return SUCCESS;
}
