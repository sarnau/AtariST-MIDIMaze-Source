/************************************************************
 *** Artwork for the shapes used to decorate the smiley at a win/loose
 ************************************************************/
#include "GLOBALS.H"

/* the tongue, in case the player/team lost */
/* X___ ____ ____ ____ _X__ ____ ____ ____ */
/* XXX_ ____ ____ ___X XX__ ____ ____ ____ */
/* XXXX XXXX _XXX XXXX XX__ ____ ____ ____ */
/* XXXX XXXX _XXX XXXX ____ ____ ____ ____ */
/* _XXX XXXX XXXX XXXX ____ ____ ____ ____ */
/* __XX XXXX XXXX XXXX ____ ____ ____ ____ */
/* ___X XXXX XXXX XXX_ ____ ____ ____ ____ */
/* ____ XXXX XXXX X___ ____ ____ ____ ____ */
/* ____ __XX XX__ ____ ____ ____ ____ ____ */
static unsigned short loosershape_img[18*2] = { 0x8000,0x4000, 0xe001,0xc000, 0xff7f,0xc000, 0xff7f,0x8000, 0x7fff,0x0000, 0x3fff,0x0000, 0x1ffe,0x0000, 0x0ff8,0x0000, 0x03e0,0x0000 }; /* 18 words per image, *2 (memory space for potential b/w conversion) */
 
/* the eye lashes, in case the player/team won */
/* X___ ____ ____ __X_ */
/* XX__ ____ ____ _XX_ */
/* _XX_ ____ ____ XX__ */
/* X_XX XXXX XXXX X_X_ */
/* __X_ X_X_ X_X_ X___ */
/* ____ X_X_ X_X_ ____ */
static unsigned short blinzshape_img[6*2] = { 0x8002,0xc006,0x600c,0xbffa,0x2aa8,0x0aa0 }; /* 6 words per image, *2 (memory space for potential b/w conversion) */

unsigned short *loosershape_ptr;
unsigned short *blinzshape_ptr;

/************************************************************
 *** void init_end_shape(void)
 ************************************************************/
void init_end_shape(void) {

    loosershape_ptr = loosershape_img;
    blinzshape_ptr = blinzshape_img;
    if(screen_rez) {
        image_double_width(loosershape_ptr, 18);
        image_double_width(blinzshape_ptr, 6);
    }
}
