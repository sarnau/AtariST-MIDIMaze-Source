/************************************************************
 *** DRAW3D.C
 ************************************************************/
#include "globals.h"

static void draw_vline(int x,int h);
static void draw_wall(int x1,int h1,int x2,int h2,int color);

static short draw_elem_count;
static struct {
    int type; /* DRAW_TYPE_WALL/DRAW_TYPE_PLAYER/DRAW_TYPE_SHOT */
    int sprite_wallcolor; /* sprite shape or wall color */
    int x; /* x position */
    int h_shadowOffset; /* height or shadowOffset */
    int x2_size; /* 2nd x or size of the shape */
    int h2_color; /* 2nd height or the color of the shape */
} draw_elem_list[100];

/************************************************************
 *** Empty the list of items to draw
 ************************************************************/
void clear_draw_list(void) {
    draw_elem_count = 0;
}

/************************************************************
 *** Add an item to draw, this has to happen front-to-back!
 *** The list will then be draw back-to-front to correctly deal
 *** with overlapping objects (especially players and shots)
 ************************************************************/
void to_draw_list(int type,int sprite_wallcolor,int x,int h_shadowOffset,int x2_size,int h2_color) {
    draw_elem_list[draw_elem_count].type = type;
    draw_elem_list[draw_elem_count].sprite_wallcolor = sprite_wallcolor;
    draw_elem_list[draw_elem_count].x = x;
    draw_elem_list[draw_elem_count].h_shadowOffset = h_shadowOffset;
    draw_elem_list[draw_elem_count].x2_size = x2_size;
    draw_elem_list[draw_elem_count].h2_color = h2_color;
    draw_elem_count++;
}

/************************************************************
 *** render all objects onto the screen
 ************************************************************/
void draw_list(void) {
int h2;
int x2;
int h1;
int x1;
/* In color there are two different wall colors, depending on if they are on the X or Y axis. */
static short wand_farb_tab[2] = { COLOR_MAGNESIUM_INDEX, COLOR_ALUMINIUM_INDEX };

    /* no color in b/w */
    /* This code should probably be part of the initialization and not during drawing */
    if(screen_rez) {
        wand_farb_tab[0] = COLOR_MAGNESIUM_INDEX;
        wand_farb_tab[1] = COLOR_MAGNESIUM_INDEX;
    }

    /* erase the whole window */
    if(screen_rez) {
        blit_clear_window_bw();
    } else {
        blit_clear_window_color();
    }

    /* draw all the items, the list has to be ordered (back to front) */
    while(draw_elem_count > 0) {
        --draw_elem_count;

        if(draw_elem_list[draw_elem_count].type == DRAW_TYPE_WALL) {
            draw_wall(x1 = draw_elem_list[draw_elem_count].x, h1 = draw_elem_list[draw_elem_count].h_shadowOffset, x2 = draw_elem_list[draw_elem_count].x2_size, h2 = draw_elem_list[draw_elem_count].h2_color, wand_farb_tab[draw_elem_list[draw_elem_count].sprite_wallcolor]);
            draw_vline(x1, h1);
            draw_vline(x2, h2);

        } else if(draw_elem_list[draw_elem_count].type == DRAW_TYPE_PLAYER) {
            draw_shape(draw_elem_list[draw_elem_count].x, draw_elem_list[draw_elem_count].x2_size, draw_elem_list[draw_elem_count].sprite_wallcolor, draw_elem_list[draw_elem_count].h_shadowOffset, draw_elem_list[draw_elem_count].h2_color);

        } else if(draw_elem_list[draw_elem_count].type == DRAW_TYPE_SHOT) {
            /* Sprite: player from behind, so there will be no face visible, just the frame */
            draw_shape(draw_elem_list[draw_elem_count].x, draw_elem_list[draw_elem_count].x2_size, BODY_SHAPE_BACK_VIEW, draw_elem_list[draw_elem_count].h_shadowOffset, draw_elem_list[draw_elem_count].h2_color);
        }
    }
    /* show the screen to the user */
    switch_screens();
}

/************************************************************
 *** Draw vertical lines that define the walls
 ************************************************************/
void draw_vline(int x,int h) {
    /* vertical line is outside of the window? */
    if(viewscreen_hcenter-viewscreen_halfwidth >= x || viewscreen_hcenter+viewscreen_halfwidth-1 <= x) return;

    /* limit the length to the window height */
    if(h > viewscreen_floor_height) h = viewscreen_floor_height;

    if(screen_rez) {
        blit_draw_vline_bw(viewscreen_sky_height-h, viewscreen_sky_height+h, x, COLOR_BLACK_INDEX);
    } else {
        blit_draw_vline_color(viewscreen_sky_height-h, viewscreen_sky_height+h, x, COLOR_BLACK_INDEX);
    }
}

/************************************************************
 *** Draw a wall element, which is always a isosceles trapezoid turned by 90 degrees:
 ***
 ***   ---------
 ***  /         \
 *** /           \
 *** -------------
 ***
 *** This means they are a recangle plus two mirrored, but identical triangles
 *** above and below. And that's exactly how it is drawn. 
 ***
 *** Wall elements are always drawn in the color resolution and doubled in height
 *** during b/w drawing (to apply a dithering)
 ************************************************************/

/*
   These three values are hard-coded for color and b/w, because in b/w double lines are drawn
   resulting in the same values than in color.
*/
#define WALL_MAX_HEIGHT 50 /* matches viewscreen_sky_height/viewscreen_floor_height */
#define WALL_CENTER_OFFSET 50 /* matches viewscreen_sky_height */
#define VIEWPORT_WIDTH 160 /* viewscreen_hcenter + viewscreen_halfwidth */

void draw_wall(int x1,int h1,int x2,int h2,int color) {
register int xe;
register int slope;
register int endX;

    /* for b/w the resolution is cut in half */
    if(screen_rez) {
        h1 >>= 1;
        h2 >>= 1;
    }

    /* make sure h2 > h1, if not swap them. */
    /* That way the slope of the diagonal is always positive */
    if(h1 > h2) {
        endX = h1; h1 = h2; h2 = endX;
        /* also swap x1 and x2 */
        endX = x1; x1 = x2; x2 = endX;
    }

    /* limit the width (not sure what this is about) */
    if(x1 == VIEWPORT_WIDTH) x1--;
    if(x2 == VIEWPORT_WIDTH) x2--;

    /* clip the height to the height of the horizon */
    if(h1 > WALL_MAX_HEIGHT) h1 = WALL_MAX_HEIGHT;

    /* draw a box as a wall (without the slope)  */
    if(screen_rez) {
        blit_fill_box_bw_double(x1, WALL_CENTER_OFFSET-h1, x2, h1+WALL_CENTER_OFFSET, color);
    } else {
        blit_fill_box_color(x1, WALL_CENTER_OFFSET-h1, x2, h1+WALL_CENTER_OFFSET, color);
    }

#if BUGFIX_DIVISION_BY_ZERO
    /* the Atari ignores a division by zero, but other machines do not. */
    /* For that reason this line was added to the original MIDImaze code. */
    if(h2==h1) return;
#endif

    /* calculate the slope of the diagonal */
    /* do avoid floating point, we multiply by 16, to have 4 bit of "fractions" */
    /* slope = (x1 - x1) * 16 / (h2 - h1) */
    slope = x2; slope -= x1; slope *= 16; slope = slope/(h2-h1);
    /* endX = x1 * 16 + slope / 2 + 16/2 (for rounding) */
    endX = x1; endX *= 16; endX += slope>>1; endX += 16/2;
    while(++h1 <= h2) {
        if(h1 > WALL_CENTER_OFFSET) break; /* top of the window area reached? */
        xe = endX;
        xe >>= 4;
        /* The wall is mirrored around the horizon line  */
        if(screen_rez) {
            blit_draw_hline_bw_double(x2, xe, WALL_CENTER_OFFSET-h1, color);
            blit_draw_hline_bw_double(x2, xe, h1+WALL_CENTER_OFFSET, color);
        } else {
            blit_draw_hline_color(x2, xe, WALL_CENTER_OFFSET-h1, color);
            blit_draw_hline_color(x2, xe, h1+WALL_CENTER_OFFSET, color);
        }
        endX += slope;
    }
}

/************************************************************
 *** Convert distance coordinate of a wall point into screen coordinates
 *** BTW: y is always negative! That's why the height has a negative sign in the formula.
 ************************************************************/
void calc_yx_to_xh(int *pinY_outX,int *pinX_outH) {
register int newH;
register int newX;

    newX = muls_divs(*pinX_outH, viewscreen_halfwidth, *pinY_outX);
    newH = viewscreen_cell_pixels; newH *= MAZE_CELL_SIZE; newH = newH / *pinY_outX;
    *pinY_outX = -newX+viewscreen_hcenter; /* = viewscreen_hcenter - (x * viewscreen_halfwidth / y) */
    *pinX_outH = -newH; /* = -(viewscreen_cell_pixels * MAZE_CELL_SIZE) / y */
}
