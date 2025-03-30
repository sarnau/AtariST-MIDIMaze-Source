/************************************************************
 *** RND.C
 ************************************************************/
#include "GLOBALS.H"

/************************************************************
 *** MIDImaze uses it's own random generator. This is needed
 *** because the seed needs to be shared with all slaves.
 *** This will gurantee that a "random" position of a new
 *** player is the same on all machines.
 *** For this reason the Master generates a 16-bit random number,
 *** which is used and shared as _random_seed before any game.
 ************************************************************/
short _random_seed;

static int _random(void) {
    /* This type of random generate is a "Linear congruential generators" (see Knuth), similar to the one used in the Atari ST XBIOS. Both numbers are prime, but because we use shorts to store the results, the +130253 is actually just a -819 (and not a prime) */
    _random_seed = _random_seed*6907+130253;
    return _random_seed;
}

/************************************************************
 *** Only called with 2 different values: 256 or the maze_size
 *** Because of the loop below, 256 is always returning immediately,
 *** which leaves the call to find a new player position in the
 *** maze the only one needing it.
 ************************************************************/
int _rnd(int maxVal) {
int maxUnscaledVal;
int randVal;

    /* To avoid an expensive modulo operation. But this could be faster,
       because often only a single random bit is needed. */
    maxUnscaledVal = 256/maxVal*maxVal;
    /* generate a random 8-bit value. It uses the middle bits for a better
       distribution */
    while((randVal = (_random()>>4) & 0xff) >= maxUnscaledVal) {}
    return randVal%maxVal;
}
