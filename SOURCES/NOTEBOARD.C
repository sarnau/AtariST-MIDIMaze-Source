/************************************************************
 *** Thie file manages the score board with the note lines
 ************************************************************/
#include "GLOBALS.H"

short score_table[PLAYER_MAX_COUNT];
short team_scores[PLAYER_MAX_TEAMS];
short currently_displayed_notes_score[PLAYER_MAX_COUNT];

/************************************************************
 *** Draw the note head/score for a specific player
 *** Also handles erasing a previous position 
 ************************************************************/
void draw_one_note_score(int player,int score) {
int i;
int y;
int x;

    x = player*7+4;
    y = 35-currently_displayed_notes_score[player]*3;
    if(screen_rez) {
        x <<= 1; /* double the values for b/w */
        y <<= 1;
        y++;
    }

    /* existing score being displayed?  */
    if(currently_displayed_notes_score[player] >= 0) {
        for(i = 0; i < 2; i++) { /* both screen buffer need updating */
            /* erase area of the player with black */
            if(screen_rez)
                blit_fill_box_bw(x, y-9, x+9, y, COLOR_BLACK_INDEX);
            else
                blit_fill_box_color(x, y-4, x+4, y, COLOR_BLACK_INDEX);

            /* redraw the score lines, if necesary */
            if(currently_displayed_notes_score[player] & 1) {
                if(screen_rez) {
                    blit_draw_hline_bw(x, x+9, y-4, COLOR_SILVER_INDEX);
                    blit_draw_hline_bw(x, x+9, y-5, COLOR_SILVER_INDEX);
                } else {
                    blit_draw_hline_color(x, x+4, y-2, COLOR_WHITE_INDEX);
                }
            }
            switch_logbase();
        }
    }
    /* save new position in the score table */
    currently_displayed_notes_score[player] = score;

    /* new position of the note head (player) in the score table */
    y = 35-score*3;
    if(screen_rez) y = y+y+1;

    /* Is the score reset? (no player score on this position) */
    if(score < 0) return;

    /* draw the note head twice (into both screen buffers) */
    for(i = 0; i < 2; i++) {
        if(screen_rez) {
            blit_draw_shape_bw(x, y, mapsmily_ptr+2*5, 2, 5, bw_fillpattern_table[team_flag ? team_notes_color_table[player] : color_cnv_back[player]]);
        } else {
            blit_draw_shape_color(x, y, mapsmily_ptr+1*5, 1, 5, col_setcolor_jumptable[team_flag ? team_notes_color_table[player] : color_cnv_back[player]]);
            blit_draw_shape_color(x, y, mapsmilyface_ptr+1*5, 1, 5, col_setcolor_jumptable[team_flag ? COLOR_BLACK_INDEX : color_cnv_frame[player]]);
        }
        switch_logbase();
    }
}

/************************************************************
 *** Update all scores for all players/team. Redraw them if necessary.
 ************************************************************/
void draw_all_notes_score(int playerCount) {
int i;

    set_screen_offs(score_notes_wind_offset);
    for(i = 0; i < playerCount; i++) {
        if(team_flag) {
            if(team_scores[i] != currently_displayed_notes_score[i])
                draw_one_note_score(i, team_scores[i]);
        } else {
            if(player_data[i].ply_score != currently_displayed_notes_score[i])
                draw_one_note_score(i, player_data[i].ply_score);
        }
    }
    set_screen_offs(viewscreen_wind_offset);
}

/************************************************************
 *** Animate the score table to show the overall game score
 *** This happens e.g. at the end of a game
 ************************************************************/
void update_notes_score(int playerCount) {
int notdone;
int i;

    set_screen_offs(score_notes_wind_offset);
    do {
        notdone = FALSE; /* will be set if any score still needed changes */
        for(i = 0; i < playerCount; i++) {
            if(score_table[i] > currently_displayed_notes_score[i]) {
                /* score needs to be increased compared to the current display */
                draw_one_note_score(i, currently_displayed_notes_score[i]+1);
                notdone = TRUE;
            } else if(score_table[i] < currently_displayed_notes_score[i]) {
                /* score needs to be decreased compared to the current display */
                draw_one_note_score(i, currently_displayed_notes_score[i]-1);
                notdone = TRUE;
            }
        }
        if(!notdone) break; /* leave, when nothing changed */
        Bconout(CON, 7); /* BELL sound */
        /* ~0.15s delay */
        for(i = 0; i < 10; i++)
            Vsync();
    } while(1);
    set_screen_offs(viewscreen_wind_offset);
}
