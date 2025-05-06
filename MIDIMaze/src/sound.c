/************************************************************
 *** Manages the two sounds MIDImaze has: a shot and a hit
 ************************************************************/
#include "globals.h"


static const unsigned char sound_shot[] = {
                                    0x07,0x34, /* Mixer Control: Channel C Noise (1=Off), Channel B Noise, Channel C Tone (0=On) */
                                    0x08,0x00, /* Channel A Amplitude Control: 0 */
                                    0x09,0x00, /* Channel B Amplitude Control */
                                    0x01,0x00, /* Channel A Freq High: 0 */
                                    0x03,0x00, /* Channel B Freq High: 0 */

                                    0x00,0x96, /* Channel A Freq Low: 150: 833Hz */
                                    0x08,0x0f, /* Channel A Amplitude Control: 15 */
                                    0x06,0x04, /* Noise Freq: 4 */
                                    0x02,0xc8, /* Channel B Freq Low: 200: 625Hz */
                                    0x09,0x0f, /* Channel B Amplitude Control: 15 */
                                    0x82,0x01, /* wait 1 tick */

                                    0x00,0x82, /* Channel A Freq Low: 130: 962Hz */
                                    0x08,0x0e, /* Channel A Amplitude Control: 14 */
                                    0x06,0x06, /* Noise Freq: 6 */
                                    0x02,0xb4, /* Channel B Freq Low: 180: 694Hz */
                                    0x09,0x0e, /* Channel B Amplitude Control: 14 */
                                    0x82,0x01, /* wait 1 tick */

                                    0x00,0x8c, /* Channel A Freq Low: 140: 893Hz */
                                    0x08,0x0d, /* Channel A Amplitude Control: 13 */
                                    0x06,0x08, /* Noise Freq: 8 */
                                    0x02,0xbe, /* Channel B Freq Low: 190: 658Hz */
                                    0x09,0x0d, /* Channel B Amplitude Control: 13 */
                                    0x82,0x01, /* wait 1 tick */

                                    0x00,0x78, /* Channel A Freq Low: 120: 1042Hz */
                                    0x08,0x0c, /* Channel A Amplitude Control: 12 */
                                    0x06,0x0a, /* Noise Freq: 10 */
                                    0x02,0xaa, /* Channel B Freq Low: 170: 735Hz */
                                    0x09,0x0c, /* Channel B Amplitude Control: 12 */
                                    0x82,0x01, /* wait 1 tick */

                                    0x00,0x82, /* Channel A Freq Low: 130: 962Hz */
                                    0x08,0x0b, /* Channel A Amplitude Control: 11 */
                                    0x06,0x0c, /* Noise Freq: 12 */
                                    0x02,0xb4, /* Channel B Freq Low: 180: 694Hz */
                                    0x09,0x0b, /* Channel B Amplitude Control: 11 */
                                    0x82,0x01, /* wait 1 tick */

                                    0x00,0x6e, /* Channel A Freq Low: 110: 1136Hz */
                                    0x08,0x0a, /* Channel A Amplitude Control: 10 */
                                    0x06,0x0e, /* Noise Freq: 14 */
                                    0x02,0xa0, /* Channel B Freq Low: 160: 781Hz */
                                    0x09,0x0a, /* Channel B Amplitude Control: 10 */
                                    0x82,0x01, /* wait 1 tick */

                                    0x00,0x78, /* Channel A Freq Low: 120: 1042Hz */
                                    0x08,0x09, /* Channel A Amplitude Control: 9 */
                                    0x06,0x10, /* Noise Freq: 16 */
                                    0x02,0xaa, /* Channel B Freq Low: 170: 735Hz */
                                    0x09,0x09, /* Channel B Amplitude Control: 9 */
                                    0x82,0x01, /* wait 1 tick */

                                    0x00,0x64, /* Channel A Freq Low: 100: 1250Hz */
                                    0x08,0x08, /* Channel A Amplitude Control: 8 */
                                    0x06,0x12, /* Noise Freq: 18 */
                                    0x02,0x96, /* Channel B Freq Low: 150: 833Hz */
                                    0x09,0x08, /* Channel B Amplitude Control: 8 */
                                    0x82,0x01, /* wait 1 tick */

                                    0x00,0x6e, /* Channel A Freq Low: 110: 1136Hz */
                                    0x08,0x07, /* Channel A Amplitude Control: 7 */
                                    0x06,0x14, /* Noise Freq: 20 */
                                    0x02,0xa0, /* Channel B Freq Low: 160: 781Hz */
                                    0x09,0x07, /* Channel B Amplitude Control: 7 */
                                    0x82,0x01, /* wait 1 tick */

                                    0x00,0x5a, /* Channel A Freq Low: 90: 1389Hz */
                                    0x08,0x06, /* Channel A Amplitude Control: 6 */
                                    0x06,0x16, /* Noise Freq: 22 */
                                    0x02,0x8c, /* Channel B Freq Low: 130: 962Hz */
                                    0x09,0x06, /* Channel B Amplitude Control: 6 */
                                    0x82,0x01, /* wait 1 tick */

                                    0x00,0x64, /* Channel A Freq Low: 100: 1250Hz */
                                    0x08,0x05, /* Channel A Amplitude Control: 5 */
                                    0x06,0x18, /* Noise Freq: 24 */
                                    0x02,0x96, /* Channel B Freq Low: 150: 833Hz */
                                    0x09,0x05, /* Channel B Amplitude Control: 5 */
                                    0x82,0x01, /* wait 1 tick */

                                    0x00,0x50, /* Channel A Freq Low: 80: 1563Hz */
                                    0x08,0x04, /* Channel A Amplitude Control: 4 */
                                    0x06,0x1a, /* Noise Freq: 26 */
                                    0x02,0x82, /* Channel B Freq Low: 130: 962Hz */
                                    0x09,0x04, /* Channel B Amplitude Control: 4 */
                                    0x82,0x01, /* wait 1 tick */

                                    0x00,0x5a, /* Channel A Freq Low: 90: 1389Hz */
                                    0x08,0x03, /* Channel A Amplitude Control: 3 */
                                    0x06,0x1c, /* Noise Freq: 28 */
                                    0x02,0x8c, /* Channel B Freq Low: 140: 893Hz */
                                    0x09,0x03, /* Channel B Amplitude Control: 3 */
                                    0x82,0x01, /* wait 1 tick */

                                    0x00,0x46, /* Channel A Freq Low: 70: 1786Hz */
                                    0x08,0x02, /* Channel A Amplitude Control: 2 */
                                    0x06,0x1e, /* Noise Freq: 30 */
                                    0x02,0x78, /* Channel B Freq Low: 120: 1042Hz */
                                    0x09,0x02, /* Channel B Amplitude Control: 2 */
                                    0x82,0x01, /* wait 1 tick */

                                    0x00,0x50, /* Channel A Freq Low: 80: 1563Hz */
                                    0x08,0x01, /* Channel A Amplitude Control: 1 */
                                    0x06,0x1f, /* Noise Freq: 31 */
                                    0x02,0x82, /* Channel B Freq Low: 130: 962Hz */
                                    0x09,0x01, /* Channel B Amplitude Control: 1 */
                                    0x82,0x01, /* wait 1 tick */

                                    0x08,0x00, /* Channel A Amplitude Control: 0 */
                                    0x09,0x00, /* Channel B Amplitude Control: 0 */
                                    0x82,0x00 /* end of sound */
                                    };
static const unsigned char sound_hit[] = {
                                    0x07,0x36, /* Mixer Control: Channel C Noise (1=Off), Channel B Noise, Channel C Tone (0=On), Channel B Tone */
                                    0x08,0x10, /* Channel A Amplitude Control: Variable */
                                    0x00,0xff, /* Channel A Freq Low: 255 */
                                    0x01,0x0f, /* Channel A Freq High: 15: 31Hz */
                                    0x06,0x1f, /* Noise Freq: 31 */
                                    0x0b,0x00, /* Envelope Period High: 0 */
                                    0x0c,0x10, /* Envelope Period Low: 16 */
                                    0x0d,0x00, /* Envelope Shape: \____________________________________ */
                                    0x82,0x00 /* end of sound */
                                    };
#define GI_REGS 14 /* number of saved registers */
static char saved_sound_regs[GI_REGS];
const char *sound_shot_ptr;
const char *sound_hit_ptr;

/************************************************************
 *** void init_sound(void)
 ************************************************************/
void init_sound(void) {
int i;

    for(i = 0; i < GI_REGS; i++)
        saved_sound_regs[i] = Giaccess(0, i);
    sound_shot_ptr = (const char *)sound_shot;
    sound_hit_ptr = (const char *)sound_hit;
}

/************************************************************
 *** void exit_sound(void)
 ************************************************************/
void exit_sound(void) {
int i;

    for(i = 0; i < GI_REGS; i++)
        (void)Giaccess(saved_sound_regs[i], i|128);
}
