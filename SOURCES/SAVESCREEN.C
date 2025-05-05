/************************************************************
 *** Restore/Save the screen/areas of the screen. Used in the
 *** context of AES, which doesn't support double buffering.
 ************************************************************/
#include "globals.h"

/************************************************************
 *** Copy currently visible screen onto the double-buffer
 *** and switch both to the same. This is used outside of
 *** the game loop to be able to use AES dialogs.
 ************************************************************/
void copy_screen(void) {
register unsigned short *dest;
register unsigned short *src;
register int i;

    src = screen_ptr[screen_flag^1]; /* copy from current Physbase */
    dest = screen_ptr[screen_flag]; /* to current logbase */
    for(i = 0; i < 16000; i++)
        *dest++ = *src++;
    Setscreen(screen_ptr[1], screen_ptr[1], -1); /* use original Physbase */
    if(!screen_flag)
        switch_logbase();   /* this is only called to update the screen_offs_adr variable */
}

/************************************************************
 *** save/restore two screen areas: the viewscreen and the pop chart
 *** This is used when AES dialogs are show to be able to restore
 *** it. Especially the pop chart can't be restored otherwise, because
 *** The kills are not saved anyway.
 *** saveFlag = NO => store the areas
 *** saveFlag = YES = restore the areas
 ************************************************************/
void save_part_of_screen_for_dialog(int saveFlag) {
register long bufferIndex;
register long wordIndex;
register long lineIndex;
register long areaIndex;
register unsigned short *scrPtr;
struct {
    int offset;
    int lines;
    int words;
} saveAreas[2];
static unsigned short save_part_of_screen_for_dialog_buffer[202*20 + 76*16];

    /* area for the viewscreen */
    saveAreas[0].offset = viewscreen_wind_offset;
    saveAreas[0].lines = screen_rez ? 202 : 101;
    saveAreas[0].words = screen_rez ? 20 : 40;

    /* area for the pop chart */
    saveAreas[1].offset = popchart_wind_offset;
    saveAreas[1].lines = screen_rez ? 76 : 38;
    saveAreas[1].words = screen_rez ? 16 : 32;

    bufferIndex = 0;
    for(areaIndex = 0; areaIndex < sizeof(saveAreas)/sizeof(saveAreas[0]); areaIndex++) {
        scrPtr = (unsigned short*)(saveAreas[areaIndex].offset + (char*)screen_ptr[0]);
        for(lineIndex = 0; lineIndex < saveAreas[areaIndex].lines; lineIndex++) {
            for(wordIndex = 0; wordIndex < saveAreas[areaIndex].words; wordIndex++) {
                if(!saveFlag)
                    save_part_of_screen_for_dialog_buffer[bufferIndex++] = scrPtr[wordIndex];
                else
                    scrPtr[wordIndex] = save_part_of_screen_for_dialog_buffer[bufferIndex++];
            }
            scrPtr += screen_rez ? (SCREEN_BW_LINEOFFSET/2) : (SCREEN_COL_LINEOFFSET/2); /* line width in words */
        }
    }
}
