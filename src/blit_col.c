#include "globals.h"
#include "calc_tab.h"

/*
; L0A16
; void blit_draw_shape_color(int x,int y,const unsigned int *imageMask,int widthInWords,int height,void *colorFuncPtr);
blit_draw_shape_color:
      LINK      A6,#0
      MOVEM.L   D3-D7,-(A7)
      MOVEA.L   screen_offs_adr,A0
      MOVE.W    10(A6),D2 ; y
      ADD.W     D2,D2
      LEA       mult_color_tab,A2
      MOVE.W    0(A2,D2.W),D2
      LEA       0(A0,D2.W),A1
      MOVEA.L   20(A6),A2 ; colorFuncPtr
      MOVEA.L   12(A6),A0 ; imageMask
      MOVE.W    18(A6),D5 ; height
      SUBQ.W    #1,D5
      MOVE.W    8(A6),D2 ; x
      MOVE.W    D2,D4
      ANDI.W    #$F,D2
      ASR.W     #4,D4
      ADD.W     16(A6),D4 ; widthInWords
      ASL.W     #3,D4
      MOVE.W    D4,8(A6)
      SUBQ.W    #1,16(A6)
      CMP.W     #8,D2
      BGT       L0A19
L0A17:MOVE.W    8(A6),D4
      MOVE.W    16(A6),D3
      CLR.W     D1
L0A18:MOVE.W    -(A0),D0
      SWAP      D0
      MOVE.W    D1,D0
      LSR.L     D2,D0
      BSR       L0A1C
      SUBQ.W    #8,D4
      MOVE.W    (A0),D1
      DBF       D3,L0A18
      LSR.W     D2,D1
      MOVE.W    D1,D0
      BSR       L0A1C
      SUBA.W    #SCREEN_COL_LINEOFFSET,A1
      DBF       D5,L0A17
      MOVEM.L   (A7)+,D3-D7
      UNLK      A6
      RTS
L0A19:NEG.W     D2
      ADD.W     #16,D2
L0A1A:MOVE.W    8(A6),D4
      MOVE.W    16(A6),D3
      CLR.L     D0
      CLR.W     D1
L0A1B:MOVE.W    -(A0),D0
      ASL.L     D2,D0
      OR.W      D1,D0
      BSR       L0A1C
      CLR.W     D0
      SWAP      D0
      MOVE.W    D0,D1
      SUBQ.W    #8,D4
      DBF       D3,L0A1B
      MOVE.W    D1,D0
      BSR       L0A1C
      SUBA.W    #SCREEN_COL_LINEOFFSET,A1
      DBF       D5,L0A1A
      MOVEM.L   (A7)+,D3-D7
      UNLK      A6
      RTS
L0A1C:TST.W     D4
      BLT       L0A1D
      CMP.W     #SCREEN_BW_LINEOFFSET,D4
      BGE       L0A1D
      MOVE.W    D0,D6
      NOT.W     D6
      JMP       (A2)
L0A1D:RTS

L0A1E:AND.W     D6,6(A1,D4.W)
      AND.W     D6,4(A1,D4.W)
      AND.W     D6,2(A1,D4.W)
      AND.W     D6,0(A1,D4.W)
      RTS
L0A1F:AND.W     D6,6(A1,D4.W)
      AND.W     D6,4(A1,D4.W)
      AND.W     D6,2(A1,D4.W)
      OR.W      D0,0(A1,D4.W)
      RTS
L0A20:AND.W     D6,6(A1,D4.W)
      AND.W     D6,4(A1,D4.W)
      OR.W      D0,2(A1,D4.W)
      AND.W     D6,0(A1,D4.W)
      RTS
L0A21:AND.W     D6,6(A1,D4.W)
      AND.W     D6,4(A1,D4.W)
      OR.W      D0,2(A1,D4.W)
      OR.W      D0,0(A1,D4.W)
      RTS
L0A22:AND.W     D6,6(A1,D4.W)
      OR.W      D0,4(A1,D4.W)
      AND.W     D6,2(A1,D4.W)
      AND.W     D6,0(A1,D4.W)
      RTS
L0A23:AND.W     D6,6(A1,D4.W)
      OR.W      D0,4(A1,D4.W)
      AND.W     D6,2(A1,D4.W)
      OR.W      D0,0(A1,D4.W)
      RTS
L0A24:AND.W     D6,6(A1,D4.W)
      OR.W      D0,4(A1,D4.W)
      OR.W      D0,2(A1,D4.W)
      AND.W     D6,0(A1,D4.W)
      RTS
L0A25:AND.W     D6,6(A1,D4.W)
      OR.W      D0,4(A1,D4.W)
      OR.W      D0,2(A1,D4.W)
      OR.W      D0,0(A1,D4.W)
      RTS
L0A26:OR.W      D0,6(A1,D4.W)
      AND.W     D6,4(A1,D4.W)
      AND.W     D6,2(A1,D4.W)
      AND.W     D6,0(A1,D4.W)
      RTS
L0A27:OR.W      D0,6(A1,D4.W)
      AND.W     D6,4(A1,D4.W)
      AND.W     D6,2(A1,D4.W)
      OR.W      D0,0(A1,D4.W)
      RTS
L0A28:OR.W      D0,6(A1,D4.W)
      AND.W     D6,4(A1,D4.W)
      OR.W      D0,2(A1,D4.W)
      AND.W     D6,0(A1,D4.W)
      RTS
L0A29:OR.W      D0,6(A1,D4.W)
      AND.W     D6,4(A1,D4.W)
      OR.W      D0,2(A1,D4.W)
      OR.W      D0,0(A1,D4.W)
      RTS
L0A2A:OR.W      D0,6(A1,D4.W)
      OR.W      D0,4(A1,D4.W)
      AND.W     D6,2(A1,D4.W)
      AND.W     D6,0(A1,D4.W)
      RTS
L0A2B:OR.W      D0,6(A1,D4.W)
      OR.W      D0,4(A1,D4.W)
      AND.W     D6,2(A1,D4.W)
      OR.W      D0,0(A1,D4.W)
      RTS
L0A2C:OR.W      D0,6(A1,D4.W)
      OR.W      D0,4(A1,D4.W)
      OR.W      D0,2(A1,D4.W)
      AND.W     D6,0(A1,D4.W)
      RTS
L0A2D:OR.W      D0,6(A1,D4.W)
      OR.W      D0,4(A1,D4.W)
      OR.W      D0,2(A1,D4.W)
      OR.W      D0,0(A1,D4.W)
      RTS
*/

static void blit_draw_shape_color_0(unsigned short *scrPtr, int xoffs, int orMask, int andMask)
{
    scrPtr[xoffs+3] &= andMask;
    scrPtr[xoffs+2] &= andMask;
    scrPtr[xoffs+1] &= andMask;
    scrPtr[xoffs+0] &= andMask;
}

static void blit_draw_shape_color_1(unsigned short *scrPtr, int xoffs, int orMask, int andMask)
{
    scrPtr[xoffs+3] &= andMask;
    scrPtr[xoffs+2] &= andMask;
    scrPtr[xoffs+1] &= andMask;
    scrPtr[xoffs+0] |= orMask;
}

static void blit_draw_shape_color_2(unsigned short *scrPtr, int xoffs, int orMask, int andMask)
{
    scrPtr[xoffs+3] &= andMask;
    scrPtr[xoffs+2] &= andMask;
    scrPtr[xoffs+1] |= orMask;
    scrPtr[xoffs+0] &= andMask;
}

static void blit_draw_shape_color_3(unsigned short *scrPtr, int xoffs, int orMask, int andMask)
{
    scrPtr[xoffs+3] &= andMask;
    scrPtr[xoffs+2] &= andMask;
    scrPtr[xoffs+1] |= orMask;
    scrPtr[xoffs+0] |= orMask;
}

static void blit_draw_shape_color_4(unsigned short *scrPtr, int xoffs, int orMask, int andMask)
{
    scrPtr[xoffs+3] &= andMask;
    scrPtr[xoffs+2] |= orMask;
    scrPtr[xoffs+1] &= andMask;
    scrPtr[xoffs+0] &= andMask;
}

static void blit_draw_shape_color_5(unsigned short *scrPtr, int xoffs, int orMask, int andMask)
{
    scrPtr[xoffs+3] &= andMask;
    scrPtr[xoffs+2] |= orMask;
    scrPtr[xoffs+1] &= andMask;
    scrPtr[xoffs+0] |= orMask;
}

static void blit_draw_shape_color_6(unsigned short *scrPtr, int xoffs, int orMask, int andMask)
{
    scrPtr[xoffs+3] &= andMask;
    scrPtr[xoffs+2] |= orMask;
    scrPtr[xoffs+1] |= orMask;
    scrPtr[xoffs+0] &= andMask;
}

static void blit_draw_shape_color_7(unsigned short *scrPtr, int xoffs, int orMask, int andMask)
{
    scrPtr[xoffs+3] &= andMask;
    scrPtr[xoffs+2] |= orMask;
    scrPtr[xoffs+1] |= orMask;
    scrPtr[xoffs+0] |= orMask;
}

static void blit_draw_shape_color_8(unsigned short *scrPtr, int xoffs, int orMask, int andMask)
{
    scrPtr[xoffs+3] |= orMask;
    scrPtr[xoffs+2] &= andMask;
    scrPtr[xoffs+1] &= andMask;
    scrPtr[xoffs+0] &= andMask;
}

static void blit_draw_shape_color_9(unsigned short *scrPtr, int xoffs, int orMask, int andMask)
{
    scrPtr[xoffs+3] |= orMask;
    scrPtr[xoffs+2] &= andMask;
    scrPtr[xoffs+1] &= andMask;
    scrPtr[xoffs+0] |= orMask;
}

static void blit_draw_shape_color_10(unsigned short *scrPtr, int xoffs, int orMask, int andMask)
{
    scrPtr[xoffs+3] |= orMask;
    scrPtr[xoffs+2] &= andMask;
    scrPtr[xoffs+1] |= orMask;
    scrPtr[xoffs+0] &= andMask;
}

static void blit_draw_shape_color_11(unsigned short *scrPtr, int xoffs, int orMask, int andMask)
{
    scrPtr[xoffs+3] |= orMask;
    scrPtr[xoffs+2] &= andMask;
    scrPtr[xoffs+1] |= orMask;
    scrPtr[xoffs+0] |= orMask;
}

static void blit_draw_shape_color_12(unsigned short *scrPtr, int xoffs, int orMask, int andMask)
{
    scrPtr[xoffs+3] |= orMask;
    scrPtr[xoffs+2] |= orMask;
    scrPtr[xoffs+1] &= andMask;
    scrPtr[xoffs+0] &= andMask;
}

static void blit_draw_shape_color_13(unsigned short *scrPtr, int xoffs, int orMask, int andMask)
{
    scrPtr[xoffs+3] |= orMask;
    scrPtr[xoffs+2] |= orMask;
    scrPtr[xoffs+1] &= andMask;
    scrPtr[xoffs+0] |= orMask;
}

static void blit_draw_shape_color_14(unsigned short *scrPtr, int xoffs, int orMask, int andMask)
{
    scrPtr[xoffs+3] |= orMask;
    scrPtr[xoffs+2] |= orMask;
    scrPtr[xoffs+1] |= orMask;
    scrPtr[xoffs+0] &= andMask;
}

static void blit_draw_shape_color_15(unsigned short *scrPtr, int xoffs, int orMask, int andMask)
{
    scrPtr[xoffs+3] |= orMask;
    scrPtr[xoffs+2] |= orMask;
    scrPtr[xoffs+1] |= orMask;
    scrPtr[xoffs+0] |= orMask;
}

void (*col_setcolor_jumptable[16])(unsigned short *scrPtr, int xoffs, int orMask, int andMask) = { blit_draw_shape_color_0,blit_draw_shape_color_1,blit_draw_shape_color_2,blit_draw_shape_color_3,blit_draw_shape_color_4,blit_draw_shape_color_5,blit_draw_shape_color_6,blit_draw_shape_color_7,blit_draw_shape_color_8,blit_draw_shape_color_9,blit_draw_shape_color_10,blit_draw_shape_color_11,blit_draw_shape_color_12,blit_draw_shape_color_13,blit_draw_shape_color_14,blit_draw_shape_color_15 };

// validate that the xoffs is in the current line and set 16 pixels
#define blit_draw_shape_color_blit2screen() \
    if(xoffs >= 0 && xoffs < SCREEN_COL_LINEOFFSET/4) { \
        (*colorFuncPtr)(scrPtr, xoffs, INTELSWAP16(imageMaskBits), INTELSWAP16(~imageMaskBits)); \
    }

/***
 *  The imageMask is pointing _behind_ the last line, because shapes are drawn from the last line _up_
 ***/
void blit_draw_shape_color(int x, int y, const unsigned short *imageMask, int widthInWords, int height, void (*colorFuncPtr)(unsigned short *,int,int,int))
{
    // destination address of the first line
    unsigned short *scrPtr = screen_offs_adr + mult160table[y]/sizeof(unsigned short);
    // pixel within the planes
    int pixelBitOffset = x & 0x0F;
    // word offset to the x + width coordinate into the correct plane
    int xWordOffset = (((x >> 4) + widthInWords) * 8) / sizeof(unsigned short);

    if(pixelBitOffset <= 8)
    {
        do {
            int xoffs = xWordOffset;
            int width = widthInWords;
            unsigned int imageMaskBits;
            // leftover bits from the previous word
            unsigned short imageMaskLeftoverBits = 0;
            do {
                // get the last word from the image and shift it into position
                imageMaskBits = ((*--imageMask << 16) | imageMaskLeftoverBits) >> pixelBitOffset;
                // draw 16 pixels
                blit_draw_shape_color_blit2screen();
                // next word to the left
                xoffs -= 8/sizeof(unsigned short);
                imageMaskLeftoverBits = *imageMask;
            } while(--width > 0); // all words done in this line?
            // draw the last remaining leftover bits
            imageMaskBits = imageMaskLeftoverBits >> pixelBitOffset;
            blit_draw_shape_color_blit2screen();
            // and jump up to the next line
            scrPtr -= SCREEN_COL_LINEOFFSET/sizeof(unsigned short);
        } while(--height > 0);
    } else {
        pixelBitOffset = 16 - pixelBitOffset;
        do {
            int xoffs = xWordOffset;
            int width = widthInWords;
            unsigned int imageMaskBits = 0;
            // leftover bits from the previous word
            unsigned short imageMaskLeftoverBits = 0;
            do {
                // get the last word from the image and shift it into position
                imageMaskBits &= 0xFFFF0000L;
                imageMaskBits |= *--imageMask;
                imageMaskBits <<= pixelBitOffset;
                imageMaskBits |= imageMaskLeftoverBits;
                // draw 16 pixels
                blit_draw_shape_color_blit2screen();
                // 
                imageMaskBits >>= 16;
                imageMaskLeftoverBits = imageMaskBits;
                // next word to the left
                xoffs -= 8/sizeof(unsigned short);
            } while(--width > 0);
            // draw the last remaining leftover bits
            imageMaskBits = imageMaskLeftoverBits;
            blit_draw_shape_color_blit2screen();
            // and jump up to the next line
            scrPtr -= SCREEN_COL_LINEOFFSET/sizeof(unsigned short);
        } while(--height > 0);
    }
}
