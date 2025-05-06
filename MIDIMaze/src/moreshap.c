/************************************************************
 *** Artwork for the shapes used in the pop chart or on the notes board
 ************************************************************/
#include "globals.h"

/* ______XXXXXX____ ________________ */
/* ____XXXXXXXXXX__ ________________ */
/* __XXXXX____XXXXX ________________ */
/* _XXX_________XXX X_______________ */
/* _XX_________XX_X X_______________ */
/* XX_________XX___ XX______________ */
/* XX_______XXX____ XX______________ */
/* XX______XXX_____ XX______________ */
/* XX_____XXX______ XX______________ */
/* XX____XXX_______ XX______________ */
/* _XX___XX_______X X_______________ */
/* _XXXXXX_______XX X_______________ */
/* __XXXX______XXXX ________________ */
/* ____XXXXXXXXXX__ ________________ */
/* ______XXXXXX____ ________________ */
static unsigned short smileybuster_img[30*2] = { 0x03f0,0x0000, 0x0ffc,0x0000, 0x3e1f,0x0000, 0x7007,0x8000, 0x600d,0x8000, 0xc018,0xc000, 0xc070,0xc000, 0xc0e0,0xc000, 0xc1c0,0xc000, 0xc380,0xc000, 0x6301,0x8000, 0x7e03,0x8000, 0x3c0f,0x0000, 0x0ffc,0x0000, 0x03f0,0x0000 }; /* 30 words per image, *2 (memory space for potential b/w conversion) */


/* small ball for the notes board */
/* _XXX____________ */
/* XXXXX___________ */
/* XXXXX___________ */
/* XXXXX___________ */
/* _XXX____________ */
static unsigned short mapsmily_img[5*2] = { 0x7000,0xf800,0xf800,0xf800,0x7000 }; /* 5 words per image, *2 (memory space for potential b/w conversion) */


/* minimal face for the notes board */
/* ________________ */
/* _X_X____________ */
/* ________________ */
/* _XXX____________ */
/* ________________ */
static unsigned short mapsmilyface_img[5*2] = { 0x0000,0x5000,0x0000,0x7000,0x0000 }; /* 5 words per image, *2 (memory space for potential b/w conversion) */

unsigned short *smileybuster_ptr;
unsigned short *mapsmily_ptr;
unsigned short *mapsmilyface_ptr;

/************************************************************
 *** The crossed smiley in the score board accidentially
 *** had the cross in the wrong direction. This code flips the 18 left pixels.
 *** The reason for this is probably to avoid touching the artwork again.
 ************************************************************/
static void flip_crossedsmil_img(void) {
#ifdef __m68k__
/* we can only do this on big-endian machines */
unsigned long *imageMaskPtr;
#else
unsigned short *imageMaskPtr;
#endif
unsigned long newBitMask;
unsigned long orgBitMask;
int bits;
int i;

#ifdef __m68k__
    imageMaskPtr = (unsigned long*)smileybuster_img;
#else
    imageMaskPtr = smileybuster_img;
#endif
    for(i = 0; i < 15; i++) {
#ifdef __m68k__
        orgBitMask = imageMaskPtr[i];
#else
        orgBitMask = (imageMaskPtr[i*2]<<16)|imageMaskPtr[i*2+1];
#endif
        newBitMask = 0;
        for(bits = 0; bits < 18; bits++) {
            newBitMask >>= 1;
            if(orgBitMask&0x80000000L)
                newBitMask |= 0x80000000L;
            else
                newBitMask &= 0x7FFFFFFFL;
            orgBitMask <<= 1;
        }
#ifdef __m68k__
        imageMaskPtr[i] = newBitMask;
#else
        imageMaskPtr[i*2] = newBitMask>>16; imageMaskPtr[i*2+1] = newBitMask;
#endif
    }
}

/************************************************************
 *** void init_some_shp(void)
 ************************************************************/
void init_some_shp(void) {
    flip_crossedsmil_img();
    smileybuster_ptr = smileybuster_img;
    mapsmily_ptr = mapsmily_img;
    mapsmilyface_ptr = mapsmilyface_img;
    if(screen_rez) {
        image_double_width(smileybuster_ptr, 30);
        image_double_width(mapsmily_ptr, 5);
        image_double_width(mapsmilyface_ptr, 5);
    }
}
