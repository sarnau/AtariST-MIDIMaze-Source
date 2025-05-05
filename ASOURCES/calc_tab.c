#include "globals.h"
#include "calc_tab.h"

/*
* // Setup a *160 multiplication table (width of a color screen line in bytes)
* //  256 entries large, despite that the screen can only be 200 lines tall.
* //  But this allows any byte to be multiplicated by 160 (the table has to
* //  be unsigned, if any value larger than 204 is multiplicated)
.globl	calc_color_mult_tab
.text
calc_color_mult_tab:
_calc_color_tab:
lea     mult_color_tab,A0
clr     D0
move    #255,D1
L1:move D0,(A0)+
add     #SCREEN_COL_LINEOFFSET,D0
dbf     D1,L1
rts

* // Setup a *80 multiplication table (width of a monochrome screen line in bytes)
* //  400 entries large, because the screen can be 400 lines tall.
.globl	calc_bw_mult_tab
.text
calc_bw_mult_tab:
_calc_bw_tab:
lea     mult_bw_tab,A0
clr     D0
move    #399,D1
L2:move D0,(A0)+
add     #SCREEN_BW_LINEOFFSET,D0
dbf     D1,L2
rts
*/

int	mult160table[256];
int	mult80table[SCREEN_BW_HEIGHT];

const unsigned short bmask_allDown[16] = {   0xFFFF,0x7FFF,0x3FFF,0x1FFF,
                                             0x0FFF,0x07FF,0x03FF,0x01FF,
                                             0x00FF,0x007F,0x003F,0x001F,
                                             0x000F,0x0007,0x0003,0x0001 };
const unsigned short bmask_allUp[16] = {     0x8000,0xC000,0xE000,0xF000,
                                             0xF800,0xFC00,0xFE00,0xFF00,
                                             0xFF80,0xFFC0,0xFFE0,0xFFF0,
                                             0xFFF8,0xFFFC,0xFFFE,0xFFFF };
const unsigned short bmask_singlebit[16] = { 0x8000,0x4000,0x2000,0x1000,
                                             0x0800,0x0400,0x0200,0x0100,
                                             0x0080,0x0040,0x0020,0x0010,
                                             0x0008,0x0004,0x0002,0x0001 };


void calc_color_mult_tab(void)
{
	int val = 0;
	for(int i=0; i<256; ++i)
	{
		mult160table[i] = val;
		val += SCREEN_COL_LINEOFFSET;
	}
}

void calc_bw_mult_tab(void)
{
	int val = 0;
	for(int i=0; i<SCREEN_BW_HEIGHT; ++i)
	{
		mult80table[i] = val;
		val += SCREEN_BW_LINEOFFSET;
	}
}

