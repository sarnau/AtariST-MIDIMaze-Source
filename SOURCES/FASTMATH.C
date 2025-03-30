/************************************************************
 *** 2D rotation code
 ************************************************************/
#include "GLOBALS.H"

short *sine_table;

/************************************************************
 *** Calculate the sin() of an angle (0..255) in the value range from 0..256, which is multiplied by factor
 ************************************************************/
static int fast_sin(int factor,int angle) {
    angle &= 255;
    /* our sine_table[] only has 65 entries of the first quarter of the sine wave. */
    /* The sine_table[] is loaded from disk, so that MIDImaze doesn't require any floating point math. */
    /* The rest is mirrored or flipped. Even on the Atari ST saving 382 bytes for this */
    /* at the expense of speed is weird. */
    if(angle >= 128) {
        angle -= 128;
        factor = -factor;
    }
    if(angle >= 64)
        angle = 128-angle;
    return muls_divs(factor, sine_table[angle], 256);
}

/************************************************************
 *** Calculate the cos() of an angle (0..255) in the value range from 0..256, which is multiplied by factor
 ************************************************************/
static int fast_cos(int factor,int angle) {
    return fast_sin(factor, 64-angle);
}

/************************************************************
 *** A 2D rotation of X/Y around an angle (0..255)
 *** The standard formula applies (f = angle)
 *** x' = x cos f - y sin f
 *** y' = y cos f + x sin f
 ************************************************************/
void rotate2d(int *px,int *py,int angle) {
int retX;

    retX = fast_cos(*px, angle)-fast_sin(*py, angle);
    *py = fast_sin(*px, angle)+fast_cos(*py, angle);
    *px = retX;
}
