/************************************************************
 *** Setup of the game, only called once
 ************************************************************/
#include "globals.h"

static int load_datas(void);

short screen_rez; /* 0 = color, 2 = b/w - these are constants returned from Atari XBIOS Getrez() */

short viewscreen_hcenter;
short viewscreen_halfwidth;
short viewscreen_sky_height;
short viewscreen_floor_height;
short viewscreen_cell_pixels;

short viewscreen_wind_offset;
short popchart_wind_offset;
short score_notes_wind_offset;
short happiness_quotient_wind_offset;

#define SINE_TABLE_WORDS 65 /* 64+1 = a quarter of the sine wave. The rest are the same values, just mirrored and flipped */

char load_buffer[SINE_TABLE_WORDS*2+(BODY_SHAPE_BITMAPSIZE+BODY_SHAPE_BITMAPSIZE*BODY_SHAPE_FACE_COUNT)*2*2]; /* words for the sine wave, the ball shapes and the face/frame shapes in all rotation values (double the buffer the images, because the pixels are doubled in b/w later) */

/************************************************************
 *** void setup_game(void)
 ************************************************************/
void setup_game(void) {
int i;

    screen_rez = Getrez(); /* tested troughout as != 0 for b/w */
    if(screen_rez == 1) { /* 640x200 is not a supported resolution */
        form_alert(1, "[3][ |Please run in|LOW resolution|or HIGH res.][OK]");
        return;
    }
    /* constants for the viewscreen */
    viewscreen_hcenter = screen_rez ? 160 : 80; /* offset to the horizontal center of the viewscreen */
    viewscreen_halfwidth = screen_rez ? 160 : 80; /* half-width of the viewscreen (mirrored arround the horizontal center) */
    viewscreen_sky_height = screen_rez ? 100 : 50; /* height of the sky */
    viewscreen_floor_height = screen_rez ? 100 : 50; /* height of the floor */
    viewscreen_cell_pixels = screen_rez ? 40 : 20; /* height/width of a MAZE_CELL_SIZE in pixels */

    /* constants for the 4 areas on the screen (comments list the color sizes) */
    viewscreen_wind_offset = screen_rez ? 100*SCREEN_BW_LINEOFFSET+4 : 50*SCREEN_COL_LINEOFFSET+8;   /* X: 16, Y: 50, W:160, H:100 */
    popchart_wind_offset = screen_rez ? 132*SCREEN_BW_LINEOFFSET+48 : 66*SCREEN_COL_LINEOFFSET+96;   /* X:185, Y: 66, W:121, H: 37 */
    score_notes_wind_offset = screen_rez ? 20*SCREEN_BW_LINEOFFSET+48 : 10*SCREEN_COL_LINEOFFSET+96; /* X:192, Y: 10, W:114, H: 35 */
    happiness_quotient_wind_offset = screen_rez ? 32*SCREEN_BW_LINEOFFSET+32 : 16*SCREEN_COL_LINEOFFSET+64; /* X:128, Y: 16, W: 26, H:  20 */

    /* multiplication tables for screen draw */
    calc_color_mult_tab();
    calc_bw_mult_tab();

    if(init_screen() < 0) return;
    if(load_datas() < 0) return;

    /* init all shapes */
    init_faces_shapes();
    init_some_shp();
    init_happiness_quotient_shape();
    init_end_shape();

    /* calculate math tables */
    calc_sin_table();
    init_dirtable();

    /* setup the two custom sounds */
    init_sound();

    calc_drone_angle_table();

    areDronesActiveFlag = 0; /* useless, because set in game_loop() anyway */
    /* reset all scores */
    for(i = 0; i < PLAYER_MAX_COUNT; i++) {
        score_table[i] = 0;
        currently_displayed_notes_score[i] = -1;
    }
    playerAndDroneCount = 1; /* 1 solo player is always there */

    dispatch(); /* the main game loop */

    exit_aes_window();
    switch_org_scr();
    exit_sound();
}

/************************************************************
 *** Load the image artwork and the sine table
 ************************************************************/
int load_datas(void) {
    if(Fread(midimaze_d8a_filehandle, SINE_TABLE_WORDS*2+(BODY_SHAPE_BITMAPSIZE+BODY_SHAPE_BITMAPSIZE*BODY_SHAPE_FACE_COUNT)*2, &load_buffer) != SINE_TABLE_WORDS*2+(BODY_SHAPE_BITMAPSIZE+BODY_SHAPE_BITMAPSIZE*BODY_SHAPE_FACE_COUNT)*2) {
        switch_org_scr();
        form_alert(1, "[3][ |Boo-boo|reading|MIDIMAZE.D8A][OK]");
        Fclose(midimaze_d8a_filehandle);
        return FAILURE;
    }
    Fclose(midimaze_d8a_filehandle);

    /* the sine table only contains the first quarter of a full wave. */
    /* The rest is mirrored and flipped at runtime */
    sine_table = (short *)load_buffer;
    /* the ball shapes */
    shape_ball_ptr = (unsigned short *)(&load_buffer[SINE_TABLE_WORDS*2]);
    /* the face shapes (same size as the ball shapes, but for all different rotations as well) */
    shape_face_ptr = (unsigned short *)(&load_buffer[SINE_TABLE_WORDS*2+BODY_SHAPE_BITMAPSIZE*2]);

    return SUCCESS;
}
