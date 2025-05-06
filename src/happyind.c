/************************************************************
 *** This file shows the current happiness indicator
 ************************************************************/
#include "globals.h"

#define HQUOT_WIDTH 20 /* width in words */
#define HQUOT_HEIGHT 2 /* height in lines */

static unsigned short hquot_shape_img[5*HQUOT_WIDTH*HQUOT_HEIGHT*2] = { /* 5 images, 40 words per image, *2 (memory space for potential b/w conversion) */
            /* 3 lives: smiley face (Happy) */
            /* ____ ____ XXXX XXXX ____ ____ ____ ____ */
            /* ____ __XX ____ ____ XX__ ____ ____ ____ */
            /* ____ XX__ ____ ____ __XX ____ ____ ____ */
            /* ___X ___X X___ ___X X___ X___ ____ ____ */
            /* __X_ __XX XX__ __XX XX__ _X__ ____ ____ */
            /* _X__ __XX XX__ __XX XX__ __X_ ____ ____ */
            /* _X__ __XX XX__ __XX XX__ __X_ ____ ____ */
            /* X___ __XX XX__ __XX XX__ ___X ____ ____ */
            /* X___ __XX XX__ __XX XX__ ___X ____ ____ */
            /* X___ ___X X___ ___X X___ ___X ____ ____ */
            /* X___ ____ ____ ____ ____ ___X ____ ____ */
            /* X__X X___ ____ ____ ___X X__X ____ ____ */
            /* X___ XX__ ____ ____ __XX ___X ____ ____ */
            /* _X__ _XX_ ____ ____ _XX_ __X_ ____ ____ */
            /* _X__ __XX X___ ___X XX__ __X_ ____ ____ */
            /* __X_ ____ XXXX XXXX ____ _X__ ____ ____ */
            /* ___X ____ ____ ____ ____ X___ ____ ____ */
            /* ____ XX__ ____ ____ __XX ____ ____ ____ */
            /* ____ __XX ____ ____ XX__ ____ ____ ____ */
            /* ____ ____ XXXX XXXX ____ ____ ____ ____ */
            0x00ff,0x0000, 0x0300,0xc000, 0x0c00,0x3000, 0x1181,0x8800, 0x23c3,0xc400,
            0x43c3,0xc200, 0x43c3,0xc200, 0x83c3,0xc100, 0x83c3,0xc100, 0x8181,0x8100,
            0x8000,0x0100, 0x9800,0x1900, 0x8c00,0x3100, 0x4600,0x6200, 0x4381,0xc200,
            0x20ff,0x0400, 0x1000,0x0800, 0x0c00,0x3000, 0x0300,0xc000, 0x00ff,0x0000,

            /* 2 lives: straight face (OK) */
            /* ____ ____ XXXX XXXX ____ ____ ____ ____ */
            /* ____ __XX ____ ____ XX__ ____ ____ ____ */
            /* ____ XX__ ____ ____ __XX ____ ____ ____ */
            /* ___X ___X X___ ___X X___ X___ ____ ____ */
            /* __X_ __XX XX__ __XX XX__ _X__ ____ ____ */
            /* _X__ __XX XX__ __XX XX__ __X_ ____ ____ */
            /* _X__ __XX XX__ __XX XX__ __X_ ____ ____ */
            /* X___ __XX XX__ __XX XX__ ___X ____ ____ */
            /* X___ __XX XX__ __XX XX__ ___X ____ ____ */
            /* X___ ___X X___ ___X X___ ___X ____ ____ */
            /* X___ ____ ____ ____ ____ ___X ____ ____ */
            /* X___ ____ ____ ____ ____ ___X ____ ____ */
            /* X___ ____ ____ ____ ____ ___X ____ ____ */
            /* _X__ __XX XXXX XXXX XX__ __X_ ____ ____ */
            /* _X__ _X__ ____ ____ __X_ __X_ ____ ____ */
            /* __X_ ____ ____ ____ ____ _X__ ____ ____ */
            /* ___X ____ ____ ____ ____ X___ ____ ____ */
            /* ____ XX__ ____ ____ __XX ____ ____ ____ */
            /* ____ __XX ____ ____ XX__ ____ ____ ____ */
            /* ____ ____ XXXX XXXX ____ ____ ____ ____ */
            0x00ff,0x0000, 0x0300,0xc000, 0x0c00,0x3000, 0x1181,0x8800, 0x23c3,0xc400,
            0x43c3,0xc200, 0x43c3,0xc200, 0x83c3,0xc100, 0x83c3,0xc100, 0x8181,0x8100,
            0x8000,0x0100, 0x8000,0x0100, 0x8000,0x0100, 0x43ff,0xc200, 0x4400,0x2200,
            0x2000,0x0400, 0x1000,0x0800, 0x0c00,0x3000, 0x0300,0xc000, 0x00ff,0x0000,

            /* 1 live: sad face (Hurt) */
            /* ____ ____ XXXX XXXX ____ ____ ____ ____ */
            /* ____ __XX ____ ____ XX__ ____ ____ ____ */
            /* ____ XX__ ____ ____ __XX ____ ____ ____ */
            /* ___X ___X X___ ___X X___ X___ ____ ____ */
            /* __X_ __XX XX__ __XX XX__ _X__ ____ ____ */
            /* _X__ __XX XX__ __XX XX__ __X_ ____ ____ */
            /* _X__ __XX XX__ __XX XX__ __X_ ____ ____ */
            /* X___ __XX XX__ __XX XX__ ___X ____ ____ */
            /* X___ __XX XX__ __XX XX__ ___X ____ ____ */
            /* X___ ___X X___ ___X X___ ___X ____ ____ */
            /* X___ ____ ____ ____ ____ ___X ____ ____ */
            /* X___ ____ ____ ____ ____ ___X ____ ____ */
            /* X___ ____ ____ ____ ____ ___X ____ ____ */
            /* _X__ ____ _XXX XXX_ ____ __X_ ____ ____ */
            /* _X__ ___X X___ ___X X___ __X_ ____ ____ */
            /* __X_ __X_ ____ ____ _X__ _X__ ____ ____ */
            /* ___X ____ ____ ____ ____ X___ ____ ____ */
            /* ____ XX__ ____ ____ __XX ____ ____ ____ */
            /* ____ __XX ____ ____ XX__ ____ ____ ____ */
            /* ____ ____ XXXX XXXX ____ ____ ____ ____ */
            0x00ff,0x0000, 0x0300,0xc000, 0x0c00,0x3000, 0x1181,0x8800, 0x23c3,0xc400,
            0x43c3,0xc200, 0x43c3,0xc200, 0x83c3,0xc100, 0x83c3,0xc100, 0x8181,0x8100,
            0x8000,0x0100, 0x8000,0x0100, 0x8000,0x0100, 0x407e,0x0200, 0x4181,0x8200,
            0x2200,0x4400, 0x1000,0x0800, 0x0c00,0x3000, 0x0300,0xc000, 0x00ff,0x0000,

            /* 0 lives: sick face (Unconscious) */
            /* ____ ____ XXXX XXXX ____ ____ ____ ____ */
            /* ____ __XX ____ ____ XX__ ____ ____ ____ */
            /* ____ XX__ ____ ____ __XX ____ ____ ____ */
            /* ___X ____ ____ ____ ____ X___ ____ ____ */
            /* __X_ __X_ X___ ___X _X__ _X__ ____ ____ */
            /* _X__ ___X ____ ____ X___ __X_ ____ ____ */
            /* _X__ __X_ X___ ___X _X__ __X_ ____ ____ */
            /* X___ _X__ ____ ____ __X_ ___X ____ ____ */
            /* X___ ____ ____ ____ ____ ___X ____ ____ */
            /* X___ ____ ____ ____ ____ ___X ____ ____ */
            /* X___ ____ ____ ____ ____ ___X ____ ____ */
            /* X___ ____ _XXX XXXX ____ ___X ____ ____ */
            /* X___ ___X X___ ____ XX__ ___X ____ ____ */
            /* _X__ __X_ ____ ____ __X_ __X_ ____ ____ */
            /* _X__ ____ ____ ____ ____ __X_ ____ ____ */
            /* __X_ ____ ____ ____ ____ _X__ ____ ____ */
            /* ___X ____ ____ ____ ____ X___ ____ ____ */
            /* ____ XX__ ____ ____ __XX ____ ____ ____ */
            /* ____ __XX ____ ____ XX__ ____ ____ ____ */
            /* ____ ____ XXXX XXXX ____ ____ ____ ____ */
            0x00ff,0x0000, 0x0300,0xc000, 0x0c00,0x3000, 0x1000,0x0800, 0x2281,0x4400,
            0x4100,0x8200, 0x4281,0x4200, 0x8400,0x2100, 0x8000,0x0100, 0x8000,0x0100,
            0x8000,0x0100, 0x807f,0x0100, 0x8180,0xc100, 0x4200,0x2200, 0x4000,0x0200,
            0x2000,0x0400, 0x1000,0x0800, 0x0c00,0x3000, 0x0300,0xc000, 0x00ff,0x0000,

            /* 0 lives: red tongue hanging out, when player is dead */
            /* ____ ____ ____ ____ ____ ____ ____ ____ */
            /* ____ ____ ____ ____ ____ ____ ____ ____ */
            /* ____ ____ ____ ____ ____ ____ ____ ____ */
            /* ____ ____ ____ ____ ____ ____ ____ ____ */
            /* ____ ____ ____ ____ ____ ____ ____ ____ */
            /* ____ ____ ____ ____ ____ ____ ____ ____ */
            /* ____ ____ ____ ____ ____ ____ ____ ____ */
            /* ____ ____ ____ ____ ____ ____ ____ ____ */
            /* ____ ____ ____ ____ ____ ____ ____ ____ */
            /* ____ ____ ____ ____ ____ ____ ____ ____ */
            /* ____ ____ ____ ____ ____ ____ ____ ____ */
            /* ____ ____ ____ ____ ____ ____ ____ ____ */
            /* ____ ____ _XXX XXXX ____ ____ ____ ____ */
            /* ____ ____ _XXX XXXX ____ ____ ____ ____ */
            /* ____ ____ __XX XXX_ ____ ____ ____ ____ */
            /* ____ ____ ___X XX__ ____ ____ ____ ____ */
            /* ____ ____ ____ ____ ____ ____ ____ ____ */
            /* ____ ____ ____ ____ ____ ____ ____ ____ */
            /* ____ ____ ____ ____ ____ ____ ____ ____ */
            /* ____ ____ ____ ____ ____ ____ ____ ____ */
            0x0000,0x0000, 0x0000,0x0000, 0x0000,0x0000, 0x0000,0x0000, 0x0000,0x0000,
            0x0000,0x0000, 0x0000,0x0000, 0x0000,0x0000, 0x0000,0x0000, 0x0000,0x0000,
            0x0000,0x0000, 0x0000,0x0000, 0x007f,0x0000, 0x007f,0x0000, 0x003e,0x0000,
            0x001c,0x0000, 0x0000,0x0000, 0x0000,0x0000, 0x0000,0x0000, 0x0000,0x0000 };
static unsigned short *hquot_shape_ptr;

/************************************************************
 *** void init_happiness_quotient_shape(void)
 ************************************************************/
void init_happiness_quotient_shape(void) {
    hquot_shape_ptr = hquot_shape_img;
    if(screen_rez)
        image_double_width(hquot_shape_ptr, 5*HQUOT_WIDTH*HQUOT_HEIGHT);
}

/************************************************************
 *** void update_happiness_quotient_indicator(void)
 ************************************************************/
void update_happiness_quotient_indicator(void) {
int happiness_quotient;
int shapeOffset;
int i;

    happiness_quotient = player_data[own_number].ply_lives;
    set_screen_offs(happiness_quotient_wind_offset);
    shapeOffset = 4*HQUOT_WIDTH*HQUOT_HEIGHT-happiness_quotient*HQUOT_WIDTH*HQUOT_HEIGHT; /* shape depending on the live status (0..3 is valid) */
    if(screen_rez) shapeOffset <<= 1;

    /* draw into both screen buffers */
    for(i = 0; i < 2; i++) {
        /* 12 = size of the shape (0..23 are possible; 12 => 20 lines high). If this get's change the width and height (2/4, 20) needs to change as well */
        /* ...and the red tongue will also no longer match. */
        if(screen_rez) {
            blit_draw_shape_bw(4, 38, draw_shape_bodyImageOffset[12]+shape_ball_ptr, 2*HQUOT_HEIGHT, HQUOT_WIDTH, bw_fillpattern_table[color_cnv_back[own_number]]);
            blit_draw_shape_bw(4, 38, shapeOffset+hquot_shape_ptr, 2*HQUOT_HEIGHT, HQUOT_WIDTH, bw_fillpattern_table[COLOR_BLACK_INDEX]);
        } else {
            blit_draw_shape_color(2, 19, draw_shape_bodyImageOffset[12]+shape_ball_ptr, HQUOT_HEIGHT, HQUOT_WIDTH, col_setcolor_jumptable[color_cnv_back[own_number]]);
            blit_draw_shape_color(2, 19, shapeOffset+hquot_shape_ptr, HQUOT_HEIGHT, HQUOT_WIDTH, col_setcolor_jumptable[color_cnv_frame[own_number]]);
        }
        if(happiness_quotient == 0) {
            /* draw the red tongue hanging out, if the player is dead */
            if(screen_rez)
                blit_draw_shape_bw(4, 38, hquot_shape_ptr+5*HQUOT_WIDTH*HQUOT_HEIGHT*2, 2*HQUOT_HEIGHT, HQUOT_WIDTH, bw_fillpattern_table[COLOR_MAGNESIUM_INDEX]);
            else
                blit_draw_shape_color(2, 19, hquot_shape_ptr+5*HQUOT_WIDTH*HQUOT_HEIGHT, HQUOT_HEIGHT, HQUOT_WIDTH, col_setcolor_jumptable[COLOR_RED_INDEX]);
        }
        switch_logbase();
    }
    set_screen_offs(viewscreen_wind_offset);
}
