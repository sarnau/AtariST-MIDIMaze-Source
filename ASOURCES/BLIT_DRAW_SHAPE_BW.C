#include "GLOBALS.H"
#include "CALC_TAB.H"

/*
; L09BA
; void blit_draw_shape_bw(int x,int y,const unsigned int *imageMask,int widthInWords,int halfHeight,const unsigned int *ditherTable);
blit_draw_shape_bw:
      LINK      A6,#-2
      MOVEM.L   D3-D7,-(A7)
      MOVEA.L   screen_offs_adr,A0
      MOVE.W    10(A6),D2
      ADD.W     D2,D2
      MOVE.W    D2,-2(A6)
      LEA       mult_bw_tab,A2
      MOVE.W    0(A2,D2.W),D2
      LEA       0(A0,D2.W),A1
      MOVEA.L   20(A6),A2
      MOVEA.L   12(A6),A0
      MOVE.W    18(A6),D5
      SUBQ.W    #1,D5
      MOVE.W    8(A6),D2
      MOVE.W    D2,D4
      ANDI.W    #$F,D2
      ASR.W     #4,D4
      ADD.W     16(A6),D4
      ADD.W     D4,D4
      MOVE.W    D4,8(A6)
      SUBQ.W    #1,16(A6)
      CMP.W     #8,D2
      BGT       L09BD
L09BB:MOVE.W    8(A6),D4
      MOVE.W    16(A6),D3
      MOVE.W    -2(A6),D6
      ANDI.W    #6,D6
      MOVE.W    2(A2,D6.W),D7
      MOVE.W    0(A2,D6.W),D6
      MOVE.W    D5,-(A7)
      CLR.W     D1
L09BC:MOVE.W    -(A0),D0
      SWAP      D0
      MOVE.W    D1,D0
      LSR.L     D2,D0
      BSR       L09C0
      SUBQ.W    #2,D4
      MOVE.W    (A0),D1
      DBF       D3,L09BC
      LSR.W     D2,D1
      MOVE.W    D1,D0
      BSR       L09C0
      SUBQ.W    #2,D4
      SUBA.W    #SCREEN_COL_LINEOFFSET,A1
      SUBQ.W    #4,-2(A6)
      MOVE.W    (A7)+,D5
      DBF       D5,L09BB
      MOVEM.L   (A7)+,D3-D7
      UNLK      A6
      RTS
L09BD:NEG.W     D2
      ADD.W     #16,D2
L09BE:MOVE.W    8(A6),D4
      MOVE.W    16(A6),D3
      MOVE.W    -2(A6),D6
      ANDI.W    #6,D6
      MOVE.W    2(A2,D6.W),D7
      MOVE.W    0(A2,D6.W),D6
      MOVE.W    D5,-(A7)
      CLR.L     D0
      CLR.W     D1
L09BF:MOVE.W    -(A0),D0
      ASL.L     D2,D0
      OR.W      D1,D0
      BSR       L09C0
      CLR.W     D0
      SWAP      D0
      MOVE.W    D0,D1
      SUBQ.W    #2,D4
      DBF       D3,L09BF
      MOVE.W    D1,D0
      BSR       L09C0
      SUBA.W    #SCREEN_COL_LINEOFFSET,A1
      SUBQ.W    #4,-2(A6)
      MOVE.W    (A7)+,D5
      DBF       D5,L09BE
      MOVEM.L   (A7)+,D3-D7
      UNLK      A6
      RTS

// this routine blits one word in two scanlines. It works in an interesting way:
//
// Example:
// Screen data:    11111111 11111111
// Color mask:     10101010 10101010
// EOR.W D7,D5 =>  01010101 01010101
// Shape mask:     00001111 11110000
// AND.W D0,D5 =>  00000101 01010000
// Endresult:      11111010 10101111

L09C0:TST.W     D4
      BLT       L09C1
      CMP.W     #40,D4
      BGE       L09C1
      MOVE.W    0(A1,D4.W),D5
      EOR.W     D7,D5
      AND.W     D0,D5
      EOR.W     D5,0(A1,D4.W)
      MOVE.W    -SCREEN_BW_LINEOFFSET(A1,D4.W),D5
      EOR.W     D6,D5
      AND.W     D0,D5
      EOR.W     D5,-SCREEN_BW_LINEOFFSET(A1,D4.W)
L09C1:RTS
*/

// validate that the xoffs is in the current line and set 16 pixels in 2 lines
#define blit_draw_shape_bw_blit2screen() \
    if(xoffs >= 0 && xoffs < SCREEN_BW_LINEOFFSET/4) { \
        scrPtr[xoffs] ^= (scrPtr[xoffs] ^ INTELSWAP16(ditherOne)) & INTELSWAP16(imageMaskBits); \
        scrPtr[xoffs-SCREEN_BW_LINEOFFSET/2] ^= (scrPtr[xoffs-SCREEN_BW_LINEOFFSET/2] ^ INTELSWAP16(ditherTwo)) & INTELSWAP16(imageMaskBits); \
    }

/***
 *  The imageMask is pointing _behind_ the last line, because shapes are drawn from the last line _up_
 *  A b/w image has twice the horizontal pixel, but the same vertical ones. Each pixel is dithered and drawn
 *  in two lines at once, resulting in the correct proportions.
 ***/
void blit_draw_shape_bw(int x, int y, const unsigned short *imageMask, int widthInWords, int halfHeight, const unsigned short *ditherTable)
{
    // destination address of the first line
    unsigned short *scrPtr = screen_offs_adr + mult80table[y]/sizeof(unsigned short);
    // pixel within the word
    int pixelBitOffset = x & 0x0F;
    // word offset to the x + width coordinate
    int xWordOffset = (x >> 4) + widthInWords;
    if(pixelBitOffset <= 8)
    {
        do {
            int xoffs = xWordOffset;
            int width = widthInWords;
            unsigned short ditherOne = ditherTable[(y & 3) + 1];
            unsigned short ditherTwo = ditherTable[(y & 3) + 0];
            unsigned int   imageMaskBits;
            // leftover bits from the previous word
            unsigned short imageMaskLeftoverBits = 0;
            do {
                // get the last word from the image and shift it into position
                imageMaskBits = ((*--imageMask << 16) | imageMaskLeftoverBits) >> pixelBitOffset;
                // draw 16 pixels in two lines
                blit_draw_shape_bw_blit2screen();
                // next word to the left
                xoffs -= 2/sizeof(unsigned short);
                imageMaskLeftoverBits = *imageMask;
            } while(--width > 0);
            // draw the last remaining leftover bits
            imageMaskBits = imageMaskLeftoverBits >> pixelBitOffset;
            blit_draw_shape_bw_blit2screen();
            // and jump up by two lines
            xoffs -= 2/sizeof(unsigned short);
            scrPtr -= 2*SCREEN_BW_LINEOFFSET/sizeof(unsigned short);
            y -= 2;
        } while(--halfHeight > 0);
    } else {
        pixelBitOffset = 16 - pixelBitOffset;
        do {
            int xoffs = xWordOffset;
            int width = widthInWords;
            unsigned short ditherOne = ditherTable[(y & 3) + 1];
            unsigned short ditherTwo = ditherTable[(y & 3) + 0];
            unsigned int    imageMaskBits = 0;
            // leftover bits from the previous word
            unsigned short imageMaskLeftoverBits = 0;
            do {
                // get the last word from the image and shift it into position
                imageMaskBits &= 0xFFFF0000L;
                imageMaskBits |= *--imageMask;
                imageMaskBits <<= pixelBitOffset;
                imageMaskBits |= imageMaskLeftoverBits;
                // draw 16 pixels in two lines
                blit_draw_shape_bw_blit2screen();
                imageMaskBits >>= 16;
                imageMaskLeftoverBits = imageMaskBits;
                // next word to the left
                xoffs -= 2/sizeof(unsigned short);
            } while(--width > 0);
            // draw the last remaining leftover bits
            imageMaskBits = imageMaskLeftoverBits;
            blit_draw_shape_bw_blit2screen();
            // and jump up by two lines
            scrPtr -= 2*SCREEN_BW_LINEOFFSET/sizeof(unsigned short);
            y -= 2;
        } while(--halfHeight > 0);
    }
}
