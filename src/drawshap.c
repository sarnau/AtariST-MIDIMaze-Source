/************************************************************
 *** Draw a shape (player or shot)
 ************************************************************/
#include "globals.h"

#define FACE_COUNT 20

static void init_faces_shapes_bw(void);

#define BODY_SHADOW_BITMAPSIZE 376 /* size in words for the shadows */

static unsigned short draw_shadow_img[BODY_SHADOW_BITMAPSIZE*2]; /* *2 because in b/w the shadow size is double */
static short shape_image_shadowHeight[BODY_SHAPE_COUNT];
static short draw_shape_shadowImageOffset[BODY_SHAPE_COUNT];
short draw_shape_bodyImageOffset[BODY_SHAPE_COUNT];
long draw_shape_faceImageOffset[BODY_SHAPE_COUNT];
short draw_shape_bodyHeight[BODY_SHAPE_COUNT];
unsigned short *shape_ball_ptr;
unsigned short *shape_face_ptr;

/************************************************************
 *** void init_faces_shapes(void)
 ************************************************************/
void init_faces_shapes(void) {
int shadowHalfHeight;
int shadowHeightScale;
int shapeScale;
int shapeWidthInWords;
int lastShadowOffset;
int lastFaceOffset;
int lastBodyOffset;
int sword;
int shapeLine;
int sheight;
int shapeIndex;

    /* start with shape offset 0 for all tables */
    lastBodyOffset = lastFaceOffset = lastShadowOffset = 0;

    /* BODY_SHAPE_COUNT number of body shapes */
    for(shapeIndex = 0; shapeIndex < BODY_SHAPE_COUNT; shapeIndex++) {

        /* scaling factor for the shape (32..1) with the shapeIndex 0..23 */
        if((shapeScale = BODY_SHAPE_COUNT-shapeIndex) > 16)
            shapeScale = ((shapeScale-16)<<1)+16;

        /* the height of the shape in lines */
        /* 24 shapes: 53, 50, 47, 43, 40, 37, 33, 30, 27, 25, 23, 22, 20, 18, 17, 15, 13, 12, 10, 8, 7, 5, 3, 2  */
        draw_shape_bodyHeight[shapeIndex] = (shapeScale*40/12+1)/2;

        /* the width of the shape in 16-bit words */
        shapeWidthInWords = (shapeScale-1)/8+1;

        /* the shadow is 1/4 of the height of the object */
        /* 24 shapes: 13, 12, 11, 10, 10, 9, 8, 7, 6, 6, 5, 5, 5, 4, 4, 3, 3, 3, 2, 2, 1, 1, 1, 1 */
        if((shape_image_shadowHeight[shapeIndex] = draw_shape_bodyHeight[shapeIndex]/4) == 0)
            shape_image_shadowHeight[shapeIndex] = 1;

        /* calculate the image of the shadow based on the image of the shape itself */
        if((shadowHeightScale = shape_image_shadowHeight[shapeIndex]-1) == 0) {
            shadowHeightScale = 1;
        }
        shadowHalfHeight = shape_image_shadowHeight[shapeIndex]/2; /* pull the divison out of the loop (it's actually just a shift, so not really a performance issue) */
        for(sheight = 0; shape_image_shadowHeight[shapeIndex] > sheight; sheight++) {
            shapeLine = (draw_shape_bodyHeight[shapeIndex]+((sheight-shadowHalfHeight)*(draw_shape_bodyHeight[shapeIndex]-1)*2)/shadowHeightScale-1)/2;
            for(sword = 0; sword < shapeWidthInWords; sword++) {
                draw_shadow_img[sheight*shapeWidthInWords+lastShadowOffset+sword] = (shapeLine >= 0) ? shape_ball_ptr[shapeLine*shapeWidthInWords+lastBodyOffset+sword] : 0;
            }
        }
        /* store the word offset relative to shape_face_ptr */
        lastBodyOffset = draw_shape_bodyImageOffset[shapeIndex] = draw_shape_bodyHeight[shapeIndex]*shapeWidthInWords+lastBodyOffset;
        /* store the word offset relative to draw_shadow_img */
        lastShadowOffset = draw_shape_shadowImageOffset[shapeIndex] = shape_image_shadowHeight[shapeIndex]*shapeWidthInWords+lastShadowOffset;
        /* store the word offset relative to shape_face_ptr */
        draw_shape_faceImageOffset[shapeIndex] = draw_shape_bodyHeight[shapeIndex]*shapeWidthInWords+lastFaceOffset;

        lastFaceOffset += draw_shape_bodyHeight[shapeIndex]*shapeWidthInWords*FACE_COUNT;
    }
    if(screen_rez)
        init_faces_shapes_bw();
}

/************************************************************
 *** void init_faces_shapes_bw(void)
 ************************************************************/
void init_faces_shapes_bw(void) {
int i;

    for(i = 0; i < BODY_SHAPE_COUNT; i++) {
        draw_shape_bodyImageOffset[i] <<= 1;
        draw_shape_faceImageOffset[i] <<= 1;
        draw_shape_shadowImageOffset[i] <<= 1;
    }
    image_double_width(draw_shadow_img, BODY_SHADOW_BITMAPSIZE);
    image_double_width(shape_ball_ptr, BODY_SHAPE_BITMAPSIZE+BODY_SHAPE_BITMAPSIZE*BODY_SHAPE_FACE_COUNT);
    shape_face_ptr = shape_ball_ptr+BODY_SHAPE_BITMAPSIZE*2; /* adjust pointer for b/w shapes */
}

/************************************************************
 *** double the pixels of an image for b/w. The height stays the same,
 *** it is doubled in the drawing shape code with dithering
 ************************************************************/
void image_double_width(register unsigned short *imgPtr,register long wordCount) {
register int bitPos;
register int bitImage;

    while(--wordCount >= 0) {
        bitImage = 0;
        for(bitPos = 0; bitPos < 8; bitPos++) {
            if((imgPtr[wordCount]>>bitPos)&1)
                bitImage |= 3<<(bitPos+bitPos);
        }
        imgPtr[wordCount+wordCount+1] = bitImage;
        bitImage = 0;
        for(bitPos = 0; bitPos < 8; bitPos++) {
            if((imgPtr[wordCount]>>(bitPos+8))&1)
                bitImage |= 3<<(bitPos+bitPos);
        }
        imgPtr[wordCount+wordCount] = bitImage;
    }
}

/************************************************************
 *** void draw_shape(int x,int size(COL:0--32 B/W:0--64),int sprite(0--19),int shadowOffset(COL: 0--100 B/W 0--200),int color)
 ************************************************************/
void draw_shape(int x,int size,int sprite,int shadowOffset,int color) {
unsigned short *imageMask;
int maxHeight;
int height;
int shapeWidthInWords;
int y;
int shapeIndex;
int shadowImageOffset;

    maxHeight = viewscreen_sky_height+viewscreen_floor_height;
    if(screen_rez) size >>= 1; /* cut the resolution in half for b/w */
    /* Convert the size back into a shape index (inverted math to a line in init_faces_shapes()) */
    /* The actual height is between 2 (size = 0) and 53 (size = 32) */
    shapeIndex = (size <= 16) ? BODY_SHAPE_COUNT-size : 8-((size-15)>>1);
    /* Convert a width in pixeln into a width in words. Do the math with some examples, that's how it's done! */
    shapeWidthInWords = ((size-1)>>3)+1;
    if(screen_rez) shapeWidthInWords <<= 1; /* double the width words for b/w */

    /* draw the shadow, if visible */
    /* The shadowOffset of 50 (= on top of the horizon line) will be invisible (used for just drawing a smiley) */
    /* A smaller number will move it up (never makes sense), a larger number moves it down. */
    /* It can never be lower than maxHeight. */
    /* This could actually be an optimization: the offset of 50/100 will be drawn, but is always invisble behind the body */
    
    height = shape_image_shadowHeight[shapeIndex];
    if(screen_rez) height <<= 1; /* double the height for b/w */
    shadowOffset += height>>1; /* center shadow blob around the height line */
    shadowImageOffset = draw_shape_shadowImageOffset[shapeIndex];
    /* There is clipping: the shadow is below the bottom, but the very top is still visible */
    if(shadowOffset > maxHeight && shadowOffset-height+1 <= maxHeight) {
        /* move the offset up by the number of clipped lines */
        shadowImageOffset -= (shadowOffset-maxHeight)/(screen_rez ? 2 : 1)*shapeWidthInWords;
        height -= shadowOffset-maxHeight; /* shorten the height of the shadow by the number of clipped lines */
        shadowOffset = maxHeight; /* start drawing at the bottom line */
    }
    if(screen_rez) height >>= 1; /* cut the height in half again (it was double above), because the blitting function does the doubleling, it's not in the shape itself */

    if(shadowOffset <= maxHeight && height > 0) { /* is the top of the shadow even visible? */
        imageMask = shadowImageOffset+draw_shadow_img;
        if(screen_rez)
            blit_draw_shape_bw(x, shadowOffset, imageMask, shapeWidthInWords, height, bw_fillpattern_table[COLOR_MAGNESIUM_INDEX]);
        else
            blit_draw_shape_color(x, shadowOffset, imageMask, shapeWidthInWords, height, col_setcolor_jumptable[COLOR_DKGREEN_INDEX]);
    }

    /* draw the round shape */
    height = draw_shape_bodyHeight[shapeIndex];
    y = (screen_rez ? height : height>>1)+viewscreen_sky_height; /* center the middle of the body onto the horizon */
    shadowImageOffset = draw_shape_bodyImageOffset[shapeIndex];
    imageMask = shadowImageOffset+shape_ball_ptr;
    if(screen_rez)
        blit_draw_shape_bw(x, y, imageMask, shapeWidthInWords, height, bw_fillpattern_table[color_cnv_back[color]]);
    else
        blit_draw_shape_color(x, y, imageMask, shapeWidthInWords, height, col_setcolor_jumptable[color_cnv_back[color]]);

    /* draw frame, eyes and mouth */
    shadowImageOffset = draw_shape_bodyHeight[shapeIndex]*shapeWidthInWords*sprite;
    imageMask = shadowImageOffset+draw_shape_faceImageOffset[shapeIndex]+shape_face_ptr;
    if(screen_rez)
        blit_draw_shape_bw(x, y, imageMask, shapeWidthInWords, height, bw_fillpattern_table[color_cnv_frame[color]]);
    else
        blit_draw_shape_color(x, y, imageMask, shapeWidthInWords, height, col_setcolor_jumptable[color_cnv_frame[color]]);
}
