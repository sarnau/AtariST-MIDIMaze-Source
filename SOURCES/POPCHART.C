/************************************************************
 *** This file manages the pop chart
 ************************************************************/
#include "globals.h"


/************************************************************
 *** Add a shot smily to the pop chart
 ************************************************************/
void add_one_smily(int score,int shotPlayer) {
unsigned short *imageMask;
int imageHeight;
int imageWidth;
int imageOffset;
int y;
int x;

    /* two rows of faces */
    /* They are filled from top-left to bottom-right */
    /* a score 0..10 is valid */
    y = (score <= 5) ? 16 : 33;
    x = ((score - 1) % 5) * 25 + 1;
    if(screen_rez) {
        x <<= 1;
        y <<= 1;
    }
    score = 16; /* size of the shape (0..23 is possible; 16 => 13 lines high). If you plan to change that: make sure to adjust the imageWidth and crossedsmil_ptr no longer works as well. */
    imageWidth = screen_rez ? 2 : 1; /* width in words */
    imageHeight = draw_shape_bodyHeight[score];
    imageOffset = draw_shape_bodyImageOffset[score];
    imageMask = imageOffset+shape_ball_ptr;
    set_screen_offs(popchart_wind_offset);
    
    /* draw the smiley in it's player color into both screen buffers */
    if(screen_rez)
        blit_draw_shape_bw(x, y, imageMask, imageWidth, imageHeight, bw_fillpattern_table[color_cnv_back[shotPlayer]]);
    else
        blit_draw_shape_color(x, y, imageMask, imageWidth, imageHeight, col_setcolor_jumptable[color_cnv_back[shotPlayer]]);
    switch_logbase();
    if(screen_rez)
        blit_draw_shape_bw(x, y, imageMask, imageWidth, imageHeight, bw_fillpattern_table[color_cnv_back[shotPlayer]]);
    else
        blit_draw_shape_color(x, y, imageMask, imageWidth, imageHeight, col_setcolor_jumptable[color_cnv_back[shotPlayer]]);

    /* draw face into the smiley in it's player color */
    /* draw the red crossed shape over the smiley also into both screen buffers */
    imageMask = draw_shape_faceImageOffset[score]+shape_face_ptr;
    if(screen_rez) {
        blit_draw_shape_bw(x, y, imageMask, imageWidth, imageHeight, bw_fillpattern_table[color_cnv_frame[shotPlayer]]);
        blit_draw_shape_bw(x-2, y+2, smileybuster_ptr+4*15, 4, 15, bw_fillpattern_table[COLOR_MAGNESIUM_INDEX]);
    } else {
        blit_draw_shape_color(x, y, imageMask, imageWidth, imageHeight, col_setcolor_jumptable[color_cnv_frame[shotPlayer]]);
        blit_draw_shape_color(x-1, y+1, smileybuster_ptr+2*15, 2, 15, col_setcolor_jumptable[COLOR_RED_INDEX]);
    }
    switch_logbase();
    if(screen_rez) {
        blit_draw_shape_bw(x, y, imageMask, imageWidth, imageHeight, bw_fillpattern_table[color_cnv_frame[shotPlayer]]);
        blit_draw_shape_bw(x-2, y+2, smileybuster_ptr+4*15, 4, 15, bw_fillpattern_table[COLOR_MAGNESIUM_INDEX]);
    } else {
        blit_draw_shape_color(x, y, imageMask, imageWidth, imageHeight, col_setcolor_jumptable[color_cnv_frame[shotPlayer]]);
        blit_draw_shape_color(x-1, y+1, smileybuster_ptr+2*15, 2, 15, col_setcolor_jumptable[COLOR_RED_INDEX]);
    }

    set_screen_offs(viewscreen_wind_offset);
}
