#include "globals.h"
#include "calc_tab.h"

/*
; L09C7
; void blit_draw_hline_bw(int x1,int x2,int y,int col);
blit_draw_hline_bw:
      LINK      A6,#0
      MOVEM.L   D3-D7,-(A7)
      MOVE.W    8(A6),D0 ; x1
      MOVE.W    10(A6),D1 ; x2
      MOVE.W    12(A6),D2 ; y
      MOVE.W    14(A6),D3 ; color
      MOVEA.L   screen_offs_adr,A0
      BSR       blit_draw_hline_bw_asm
      MOVEM.L   (A7)+,D3-D7
      UNLK      A6
      RTS

; L09C8
; void blit_draw_hline_bw_double(int x1,int x2,int y,int col);
blit_draw_hline_bw_double:
      LINK      A6,#0
      MOVEM.L   D3-D7,-(A7)
      MOVE.W    8(A6),D0 ; x1
      MOVE.W    10(A6),D1 ; x2
      MOVE.W    12(A6),D2 ; y
      MOVE.W    14(A6),D3 ; color
      MOVEA.L   screen_offs_adr,A0
      BSR       blit_draw_hline_bw_double_asm
      MOVEM.L   (A7)+,D3-D7
      UNLK      A6
      RTS

; L09C9
; void blit_draw_vline_bw(int y1,int y2,int x,int col);
blit_draw_vline_bw:LINK      A6,#0
      MOVEM.L   D3-D7,-(A7)
      MOVE.W    8(A6),D0 ; y1
      MOVE.W    10(A6),D1 ; y2
      MOVE.W    12(A6),D2 ; x
      MOVE.W    14(A6),D3 ; color
      MOVEA.L   screen_offs_adr,A0
      BSR       blit_draw_vline_bw_asm
      MOVEM.L   (A7)+,D3-D7
      UNLK      A6
      RTS

; L09CA
; void blit_fill_box_bw(int x1,int y1,int x2,int y2,int col);
blit_fill_box_bw:
      LINK      A6,#0
      MOVEM.L   D3-D7,-(A7)
      MOVE.W    8(A6),D0 ; x1
      MOVE.W    12(A6),D1 ; x2
      MOVE.W    16(A6),D3 ; color
      MOVEA.L   screen_offs_adr,A0
      MOVE.W    10(A6),D2 ; y1
L09CB:BSR       blit_draw_hline_bw_asm
      ADDQ.W    #1,10(A6) ; y1++
      MOVE.W    10(A6),D2 ; y1
      CMP.W     14(A6),D2 ; y2
      BLE.S     L09CB
      MOVEM.L   (A7)+,D3-D7
      UNLK      A6
      RTS

; L09CC
; void blit_fill_box_bw_double(int x1,int y1,int x2,int y2,int col);
blit_fill_box_bw_double:
      LINK      A6,#0
      MOVEM.L   D3-D7,-(A7)
      MOVE.W    8(A6),D0 ; x1
      MOVE.W    12(A6),D1 ; x2
      MOVEA.L   screen_offs_adr,A0
      MOVE.W    10(A6),D2 ; y1
L09CD:MOVE.W    16(A6),D3 ; color
      BSR       blit_draw_hline_bw_double_asm
      ADDQ.W    #1,10(A6) ; y1++
      MOVE.W    10(A6),D2 ; y1
      CMP.W     14(A6),D2 ; y2
      BLE.S     L09CD
      MOVEM.L   (A7)+,D3-D7
      UNLK      A6
      RTS

; L09CE
; void blit_clear_window_bw();
blit_clear_window_bw:
      MOVEA.L   screen_offs_adr,A0
      MOVEQ     #99,D0
      MOVEQ     #$FF,D2
L09CF:MOVEQ     #9,D1
L09D0:MOVE.L    D2,(A0)+
      DBF       D1,L09D0
      ADDA.W    #SCREEN_BW_LINEOFFSET/2,A0
      DBF       D0,L09CF
      MOVEQ     #50,D0
L09D1:MOVEQ     #9,D1
L09D2:MOVE.L    D2,(A0)+
      DBF       D1,L09D2
      ADDA.W    #SCREEN_BW_LINEOFFSET/2,A0
      EORI.L    #$AAAAAAAA,D2
      MOVEQ     #9,D1
L09D3:MOVE.L    D2,(A0)+
      DBF       D1,L09D3
      ADDA.W    #SCREEN_BW_LINEOFFSET/2,A0
      EORI.L    #$AAAAAAAA,D2
      DBF       D0,L09D1
      RTS

blit_draw_hline_bw_asm:
      LEA       sw_colorfill_functable,A2
      MOVE.W    D3,D7 ; color
      ASL.W     #2,D7
      MOVEA.L   0(A2,D7.W),A2
      MOVE.W    D2,D7
      ADD.W     D7,D7
      ANDI.W    #6,D7
      MOVE.W    2(A2,D7.W),D7
      CMP.W     D0,D1 ; x1,x2
      BGE       L09D5
      EXG       D1,D0
L09D5:ADD.W     D2,D2 ; y1
      LEA       mult_bw_tab,A2
      MOVE.W    0(A2,D2.W),D2
      MOVE.W    D0,D4
      LSR.W     #4,D4
      ASL.W     #1,D4
      ADD.W     D4,D2
      LEA       0(A0,D2.W),A1
      MOVE.W    D0,D4
      LSR.W     #4,D4
      MOVE.W    D1,D2
      LSR.W     #4,D2
      SUB.W     D4,D2
      SUBQ.W    #1,D2
      BGE       L09D6
      MOVE.W    D0,D4
      ANDI.W    #$F,D4
      ADD.W     D4,D4
      LEA       bmask_allDown,A2
      MOVE.W    0(A2,D4.W),D5
      MOVE.W    D1,D4
      ANDI.W    #$F,D4
      ASL.W     #1,D4
      LEA       bmask_allUp,A2
      AND.W     0(A2,D4.W),D5
      BRA       L09D9
L09D6:MOVE.W    D0,D4
      ANDI.W    #$F,D4
      ADD.W     D4,D4
      LEA       bmask_allDown,A2
      MOVE.W    0(A2,D4.W),D5
      BSR       L09D9
      BRA       L09D8
L09D7:MOVE.W    D7,(A1)+
L09D8:DBF       D2,L09D7
      MOVE.W    D1,D4
      ANDI.W    #$F,D4
      ADD.W     D4,D4
      LEA       bmask_allUp,A2
      MOVE.W    0(A2,D4.W),D5
L09D9:MOVE.W    (A1),D6
      EOR.W     D7,(A1)
      AND.W     D5,(A1)
      EOR.W     D6,(A1)+
      RTS

blit_draw_hline_bw_double_asm:
      LEA       sw_colorfill_functable,A2
      MOVE.W    D3,D7 ; color
      ASL.W     #2,D7
      MOVEA.L   0(A2,D7.W),A2
      ADD.W     D2,D2 ; y1
      MOVE.W    D2,D7
      ADDQ.W    #1,D7
      ADD.W     D7,D7
      ANDI.W    #6,D7
      MOVE.W    0(A2,D7.W),D3
      MOVE.W    2(A2,D7.W),D7
      CMP.W     D0,D1 ; x1,x2
      BGE       L09DB
      EXG       D1,D0
L09DB:LEA       mult_color_tab,A2
      MOVE.W    0(A2,D2.W),D2
      MOVE.W    D0,D4
      LSR.W     #4,D4
      ASL.W     #1,D4
      ADD.W     D4,D2
      LEA       0(A0,D2.W),A1
      MOVE.W    D0,D4
      LSR.W     #4,D4
      MOVE.W    D1,D2
      LSR.W     #4,D2
      SUB.W     D4,D2
      SUBQ.W    #1,D2
      BGE       L09DC
      MOVE.W    D0,D4
      ANDI.W    #$F,D4
      ADD.W     D4,D4
      LEA       bmask_allDown,A2
      MOVE.W    0(A2,D4.W),D5
      MOVE.W    D1,D4
      ANDI.W    #$F,D4
      ASL.W     #1,D4
      LEA       bmask_allUp,A2
      AND.W     0(A2,D4.W),D5
      BRA       L09DF
L09DC:MOVE.W    D0,D4
      ANDI.W    #$F,D4
      ADD.W     D4,D4
      LEA       bmask_allDown,A2
      MOVE.W    0(A2,D4.W),D5
      BSR       L09DF
      BRA       L09DE
L09DD:MOVE.W    D7,80(A1)
      MOVE.W    D3,(A1)+
L09DE:DBF       D2,L09DD
      MOVE.W    D1,D4
      ANDI.W    #$F,D4
      ADD.W     D4,D4
      LEA       bmask_allUp,A2
      MOVE.W    0(A2,D4.W),D5
L09DF:MOVE.W    (A1),D6
      EOR.W     D3,(A1)
      AND.W     D5,(A1)
      EOR.W     D6,(A1)
      ADDA.W    #SCREEN_BW_LINEOFFSET,A1
      MOVE.W    (A1),D6
      EOR.W     D7,(A1)
      AND.W     D5,(A1)
      EOR.W     D6,(A1)+
      SUBA.W    #SCREEN_BW_LINEOFFSET,A1
      RTS

blit_draw_vline_bw_asm:
      CMP.W     D0,D1
      BGE       L09E1
      EXG       D1,D0
L09E1:SUB.W     D0,D1
      ADD.W     D0,D0
      LEA       mult_bw_tab,A2
      MOVE.W    0(A2,D0.W),D0
      MOVE.W    D2,D4
      LSR.W     #3,D4
      ANDI.W    #$FFFE,D4
      ADD.W     D0,D4
      LEA       0(A0,D4.W),A1
      ANDI.W    #$F,D2
      ADD.W     D2,D2
      LEA       bmask_singlebit,A2
      MOVE.W    0(A2,D2.W),D5
      MOVE.W    D5,D6
      NOT.W     D6
L09E2:BTST      #0,D3 ; color
      BEQ       L09E3
      OR.W      D5,(A1)
      BRA       L09E4
L09E3:AND.W     D6,(A1)
L09E4:ADDA.W    #SCREEN_BW_LINEOFFSET,A1
      DBF       D1,L09E2
      RTS
*/

const unsigned short fm_bw_muster0[5] = { 0x0000,0x0000,0x0000,0x0000,0x0000 };
const unsigned short fm_bw_muster1[5] = { 0xffff,0xffff,0xffff,0xffff,0xffff };
const unsigned short fm_bw_muster2[5] = { 0xaaaa,0x5555,0xaaaa,0x5555,0xaaaa };
const unsigned short fm_bw_muster3[5] = { 0x0000,0xffff,0x0000,0xffff,0x0000 };
const unsigned short fm_bw_muster4[5] = { 0xaaaa,0xaaaa,0xaaaa,0xaaaa,0xaaaa };
const unsigned short fm_bw_muster5[5] = { 0x1111,0x8888,0x4444,0x2222,0x1111 };
const unsigned short fm_bw_muster6[5] = { 0x8888,0x1111,0x2222,0x4444,0x8888 };
const unsigned short fm_bw_muster7[5] = { 0x4444,0x8888,0x4444,0x2222,0x4444 };
const unsigned short fm_bw_muster8[5] = { 0x0000,0x8888,0x5555,0x2222,0x0000 };
const unsigned short fm_bw_muster9[5] = { 0x4444,0x2222,0x5555,0xaaaa,0x4444 };
const unsigned short fm_bw_muster10[5] = { 0x0000,0xcccc,0xcccc,0x0000,0x0000 };
const unsigned short fm_bw_muster11[5] = { 0x9249,0xcccc,0x6666,0x3333,0x9249 };
const unsigned short fm_bw_muster12[5] = { 0x6666,0xcccc,0x9999,0x3333,0x6666 };
const unsigned short fm_bw_muster13[5] = { 0x8888,0xffff,0x8888,0x8888,0x8888 };
const unsigned short fm_bw_muster14[5] = { 0x0000,0xeeee,0x0000,0x7777,0x0000 };
const unsigned short fm_bw_muster15[5] = { 0x0000,0x4444,0xeeee,0x4444,0x0000 };
const unsigned short fm_bw_muster16[5] = { 0x2222,0x8888,0xaaaa,0xaaaa,0x2222 };
const unsigned short fm_bw_muster17[5] = { 0x0000,0x4444,0xaaaa,0x4444,0x0000 };
const unsigned short fm_bw_muster18[5] = { 0x3333,0xcccc,0xcccc,0x3333,0x3333 };
const unsigned short *bw_fillpattern_table[16+3] = { fm_bw_muster0,fm_bw_muster1,fm_bw_muster2,fm_bw_muster3,fm_bw_muster4,fm_bw_muster5,fm_bw_muster6,fm_bw_muster7,fm_bw_muster8,fm_bw_muster9,fm_bw_muster10,fm_bw_muster11,fm_bw_muster12,fm_bw_muster13,fm_bw_muster14,fm_bw_muster15,fm_bw_muster16,fm_bw_muster17,fm_bw_muster18 };

#define sw_push_pixel() \
    { \
        unsigned short scrWord = *scrPtr; \
        *scrPtr ^= INTELSWAP16(patternMask); \
        *scrPtr &= INTELSWAP16(pixelMask); \
        *scrPtr++ ^= scrWord; \
    }

static void sw_fill_line(int x1, int x2, int y, int col, unsigned short *scr)
{
    unsigned short patternMask = bw_fillpattern_table[col][1 + (y & 3)];
    if(x1 > x2)
    {
        int temp = x1; x1 = x2; x2 = temp;
    }
    // offset to the first word
    unsigned short *scrPtr = (unsigned short*)((unsigned char*)scr + ((x1 >> 4) << 1) + mult80table[y]);

    // number of full words to be copied
    int countWordCount = (x2 >> 4) - (x1 >> 4) - 1;

    unsigned short pixelMask;
    if(countWordCount < 0)
    {
        pixelMask = bmask_allDown[x1 & 0x0F] & bmask_allUp[x2 & 0x0F];
        sw_push_pixel();
        return;
    }

    pixelMask = bmask_allDown[x1 & 0x0F];
    sw_push_pixel();

    while(--countWordCount >= 0)
        *scrPtr++ = patternMask;

    pixelMask = bmask_allUp[x2 & 0x0F];
    sw_push_pixel();
}

#define sw_push_double_pixel() \
    { \
        unsigned short scrWord = *scrPtr; \
        *scrPtr ^= INTELSWAP16(patternMask1); \
        *scrPtr &= INTELSWAP16(pixelMask); \
        *scrPtr ^= scrWord; \
        scrPtr += SCREEN_BW_LINEOFFSET/2; \
        scrWord = *scrPtr; \
        *scrPtr ^= INTELSWAP16(patternMask2); \
        *scrPtr &= INTELSWAP16(pixelMask); \
        *scrPtr++ ^= scrWord; \
        scrPtr -= SCREEN_BW_LINEOFFSET/2; \
    }

static void sw_fill_line_double(int x1, int x2, int y, int col, unsigned short *scr)
{
    unsigned short patternMask1 = bw_fillpattern_table[col][0 + (y & 3)];
    unsigned short patternMask2 = bw_fillpattern_table[col][1 + (y & 3)];
    if(x1 > x2)
    {
        int temp = x1; x1 = x2; x2 = temp;
    }
    // offset to the first word
    unsigned short *scrPtr = (unsigned short*)((unsigned char*)scr + ((x1 >> 4) << 1) + mult160table[y]);

    // number of full words to be copied
    int countWordCount = (x2 >> 4) - (x1 >> 4) - 1;

    unsigned short pixelMask;
    if(countWordCount < 0)
    {
        pixelMask = bmask_allDown[x1 & 0x0F] & bmask_allUp[x2 & 0x0F];
        sw_push_double_pixel();
        return;
    }

    pixelMask = bmask_allDown[x1 & 0x0F];
    sw_push_double_pixel();
    while(--countWordCount >= 0)
    {
        scrPtr[SCREEN_BW_LINEOFFSET/2] = patternMask2;
        *scrPtr++ = patternMask1;
    }
    pixelMask = bmask_allUp[x2 & 0x0F];
    sw_push_double_pixel();
}

static void sw_fill_vline(int y1, int y2, int x, int col, unsigned short *scr)
{
    if(y1 > y2)
    {
        int temp = y1; y1 = y2; y2 = temp;
    }
    y2 -= y1;

    unsigned short *scrWordPtr = (unsigned short*)((unsigned char*)scr + (((x >> 3) & 0xFFFE) + mult80table[y1]));
    int orMask = INTELSWAP16(bmask_singlebit[x & 0x0F]);
    int andMask = ~orMask;
    do {
        if(col & 1)
            *scrWordPtr |= orMask;
        else
            *scrWordPtr &= andMask;
        scrWordPtr += SCREEN_BW_LINEOFFSET/2;
    } while(--y2 > 0);
}

void blit_draw_hline_bw(int x1, int x2, int y, int col)
{
    sw_fill_line(x1, x2, y, col, screen_offs_adr);
}

void blit_draw_hline_bw_double(int x1, int x2, int y, int col)
{
    sw_fill_line_double(x1, x2, y, col, screen_offs_adr);
}

void blit_draw_vline_bw(int y1, int y2, int x, int col)
{
    sw_fill_vline(y1, y2, x, col, screen_offs_adr);
}

void blit_fill_box_bw(int x1, int y1, int x2, int y2, int col)
{
    do {
        sw_fill_line(x1, x2, y1, col, screen_offs_adr);
    } while(++y1 <= y2);
}

void blit_fill_box_bw_double(int x1, int y1, int x2, int y2, int col)
{
    do {
        sw_fill_line_double(x1, x2, y1, col, screen_offs_adr);
    } while(++y1 <= y2);
}

void blit_clear_window_bw()
{
    const int width = screen_rez != 0 ? SCREEN_BW_WIDTH : SCREEN_COL_WIDTH;
    unsigned int colorBits = 0xFFFFFFFF; // The sky color is a plain white
    unsigned int *scrPtr = (unsigned int *)screen_offs_adr;
    for(int lines=0; lines<viewscreen_sky_height; ++lines)    // 100 lines
    {
        for(int i=0; i<(viewscreen_hcenter+viewscreen_halfwidth)/32; ++i)  // 320 pixels of white
            *scrPtr++ = colorBits;
        scrPtr += (width-(viewscreen_hcenter+viewscreen_halfwidth))/32;  // skip 320 pixels
    }
    // The floor is a 010101 grey pattern
    for(int lines=0; lines<=viewscreen_floor_height/2; ++lines)    // 51 double-lines
    {
        for(int i=0; i<(viewscreen_hcenter+viewscreen_halfwidth)/32; ++i)  // 320 pixels of grey
            *scrPtr++ = colorBits;
        scrPtr += (width-(viewscreen_hcenter+viewscreen_halfwidth))/32;  // skip 320 pixels
        colorBits ^= 0xAAAAAAAA;

        for(int i=0; i<(viewscreen_hcenter+viewscreen_halfwidth)/32; ++i)  // 320 pixels of shifted grey
            *scrPtr++ = colorBits;
        scrPtr += (width-(viewscreen_hcenter+viewscreen_halfwidth))/32;  // skip 320 pixels
        colorBits ^= 0xAAAAAAAA;
    }
}
