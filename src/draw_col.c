#include "globals.h"
#include "calc_tab.h"

/*
; L0A2E
; void blit_draw_hline_color(int x1,int x2,int y,int col);
blit_draw_hline_color:
      LINK      A6,#0
      MOVEM.L   D3-D7,-(A7)
      MOVE.W    8(A6),D0 ; x1
      MOVE.W    10(A6),D1 ; x2
      MOVE.W    12(A6),D2 ; y
      MOVE.W    14(A6),D3 ; color
      MOVEA.L   screen_offs_adr,A0
      BSR       blit_draw_hline_color_asm
      MOVEM.L   (A7)+,D3-D7
      UNLK      A6
      RTS

; L0A2F
; void blit_draw_vline_color(int y1,int y2,int x,int col);
blit_draw_vline_color:
      LINK      A6,#0
      MOVEM.L   D3-D7,-(A7)
      MOVE.W    8(A6),D0 ; y1
      MOVE.W    10(A6),D1 ; y2
      MOVE.W    12(A6),D2 ; x
      MOVE.W    14(A6),D3 ; color
      MOVEA.L   screen_offs_adr,A0
      BSR       blit_draw_vline_color_asm
      MOVEM.L   (A7)+,D3-D7
      UNLK      A6
      RTS

; L0A30
; void blit_fill_box_color(int x1,int y1,int x2,int y2,int col);
blit_fill_box_color:
      LINK      A6,#0
      MOVEM.L   D3-D7,-(A7)
      MOVE.W    8(A6),D0 ; x1
      MOVE.W    12(A6),D1 ; x2
      MOVE.W    16(A6),D3 ; color
      MOVEA.L   screen_offs_adr,A0
      MOVE.W    10(A6),D2 ; y1
L0A31:BSR       blit_draw_hline_color_asm
      ADDQ.W    #1,10(A6) ; y1++
      MOVE.W    10(A6),D2 ; y1
      CMP.W     14(A6),D2 ; y2
      BLE.S     L0A31
      MOVEM.L   (A7)+,D3-D7
      UNLK      A6
      RTS

; L0A32
; void blit_clear_window_color();
blit_clear_window_color:
      MOVEA.L   screen_offs_adr,A0
      MOVEQ     #$FF,D2
      MOVEQ     #49,D0
L0A33:MOVEQ     #4,D1
L0A34:MOVE.L    D2,(A0)+
      CLR.L     (A0)+
      MOVE.L    D2,(A0)+
      CLR.L     (A0)+
      DBF       D1,L0A34
      ADDA.W    #SCREEN_BW_LINEOFFSET,A0
      DBF       D0,L0A33
      MOVEQ     #50,D0
L0A35:MOVEQ     #9,D1
L0A36:MOVE.W    D2,(A0)+
      MOVE.L    D2,(A0)+
      CLR.W     (A0)+
      DBF       D1,L0A36
      ADDA.W    #SCREEN_BW_LINEOFFSET,A0
      DBF       D0,L0A35
      RTS

blit_draw_hline_color_asm:
      CMP.W     D0,D1 ; x1,x2
      BGE       L0A38
      EXG       D1,D0
L0A38:ADD.W     D2,D2 ; y1
      LEA       mult_color_tab,A2
      MOVE.W    0(A2,D2.W),D2
      MOVE.W    D0,D4
      LSR.W     #4,D4
      ASL.W     #3,D4
      ADD.W     D4,D2
      LEA       0(A0,D2.W),A1
      MOVE.W    D0,D4
      LSR.W     #4,D4
      MOVE.W    D1,D2
      LSR.W     #4,D2
      SUB.W     D4,D2
      SUBQ.W    #1,D2
      BGE       L0A39
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
      JMP       L0A3C
L0A39:MOVE.W    D0,D4
      ANDI.W    #$F,D4
      ADD.W     D4,D4
      LEA       bmask_allDown,A2
      MOVE.W    0(A2,D4.W),D5
      BSR       L0A3C
      SUBQ.W    #1,D2
      BMI       L0A3B
      LEA       bmask_d,A2
      MOVE.W    D3,D4 ; color
      ANDI.W    #3,D4
      ASL.W     #2,D4
      MOVE.L    0(A2,D4.W),D6
      MOVE.W    D3,D4 ; color
      ANDI.W    #$C,D4
      MOVE.L    0(A2,D4.W),D7
L0A3A:MOVE.L    D6,(A1)+
      MOVE.L    D7,(A1)+
      DBF       D2,L0A3A
L0A3B:MOVE.W    D1,D4
      ANDI.W    #$F,D4
      ADD.W     D4,D4
      LEA       bmask_allUp,A2
      MOVE.W    0(A2,D4.W),D5
L0A3C:MOVE.W    D5,D6
      NOT.W     D6

L0A3D:BTST      #0,D3 ; color
      BEQ       L0A3E
      OR.W      D5,(A1)+
      BRA       L0A3F
L0A3E:AND.W     D6,(A1)+
L0A3F:BTST      #1,D3 ; color
      BEQ       L0A40
      OR.W      D5,(A1)+
      BRA       L0A41
L0A40:AND.W     D6,(A1)+
L0A41:BTST      #2,D3 ; color
      BEQ       L0A42
      OR.W      D5,(A1)+
      BRA       L0A43
L0A42:AND.W     D6,(A1)+
L0A43:BTST      #3,D3 ; color
      BEQ       L0A44
      OR.W      D5,(A1)+
      RTS
L0A44:AND.W     D6,(A1)+
      RTS

blit_draw_vline_color_asm:
      CMP.W     D0,D1
      BGE       L0A46
      EXG       D1,D0
L0A46:SUB.W     D0,D1
      ADD.W     D0,D0
      LEA       mult_color_tab,A2
      MOVE.W    0(A2,D0.W),D0
      MOVE.W    D2,D4
      LSR.W     #1,D4
      ANDI.W    #$FFF8,D4
      ADD.W     D0,D4
      LEA       0(A0,D4.W),A1
      ANDI.W    #$F,D2
      ADD.W     D2,D2
      LEA       bmask_singlebit,A2
      MOVE.W    0(A2,D2.W),D5
      MOVE.W    D5,D6
      NOT.W     D6
L0A47:BSR.S     L0A3D
      ADDA.W    #SCREEN_COL_LINEOFFSET-8,A1
      DBF       D1,L0A47
      RTS
*/

#define c_push_pixel() \
    { \
        if(col & 1) *scrPtr++ |= orMask; \
        else *scrPtr++ &= andMask; \
        if(col & 2) *scrPtr++ |= orMask; \
        else *scrPtr++ &= andMask; \
        if(col & 4) *scrPtr++ |= orMask; \
        else *scrPtr++ &= andMask; \
        if(col & 8) *scrPtr++ |= orMask; \
        else *scrPtr++ &= andMask; \
    }

static void fill_vline(int y1, int y2, int x, int col, unsigned short *a0)
{
    unsigned short *scrPtr;
    unsigned short orMask;
    unsigned short andMask;

    if(y1 > y2)
    {
        int temp = y1; y1 = y2; y2 = temp;
    }
    y2 -= y1;
    scrPtr = (unsigned short*)((unsigned char*)a0 + ((x >> 1) & 0xFFF8) + mult160table[y1]);
    orMask = INTELSWAP16(bmask_singlebit[x & 0x0F]);
    andMask = ~orMask;
    do {
        c_push_pixel();
        scrPtr += (SCREEN_COL_LINEOFFSET-8)/2;
    } while(y2-- > 0);
}

#define c_push_pixelm() \
    { \
        andMask = ~orMask; \
        c_push_pixel(); \
    }

static void fill_cline(int x1, int x2, int y, int col, unsigned short *scr)
{
    unsigned short *scrPtr;
    unsigned short orMask;
    unsigned short andMask;
    int countWordCount;
    static const uint32_t bmask_d[4]  = { 0x00000000L, 0xFFFF0000L, 0x0000FFFFL, 0xFFFFFFFFL };

    if(x1 > x2)
    {
        int temp = x1; x1 = x2; x2 = temp;
    }
    scrPtr = (unsigned short*)((unsigned char*)scr + ((x1 >> 4) * 8) + mult160table[y]);
    
    /* number of full words to be copied */
    countWordCount = (x2 >> 4) - (x1 >> 4) - 1;

    if(countWordCount < 0)
    {
        orMask = INTELSWAP16(bmask_allDown[x1 & 0x0F]) & INTELSWAP16(bmask_allUp[x2 & 0x0F]);
        c_push_pixelm();
        return;
    }

    orMask = INTELSWAP16(bmask_allDown[x1 & 0x0F]);
    c_push_pixelm();
    if(--countWordCount >= 0)
    {
        uint32_t plane12 = INTELSWAP32(bmask_d[col & 0x03]);
        uint32_t plane34 = INTELSWAP32(bmask_d[col >> 2]);
        do {
            *scrPtr++ = plane12;
            *scrPtr++ = plane12 >> 16;
            *scrPtr++ = plane34;
            *scrPtr++ = plane34 >> 16;
        } while(countWordCount-- > 0);
    }
    orMask = INTELSWAP16(bmask_allUp[x2 & 0x0F]);
    c_push_pixelm();
}

void blit_draw_hline_color(int x1, int x2, int y, int col)
{
    fill_cline(x1, x2, y, col, screen_offs_adr);
}

void blit_draw_vline_color(int y1, int y2, int x, int col)
{
    fill_vline(y1, y2, x, col, screen_offs_adr);
}

void blit_fill_box_color(int x1, int y1, int x2, int y2, int col)
{
    do {
        fill_cline(x1, x2, y1, col, screen_offs_adr);
    } while(++y1 <= y2);
}

void blit_clear_window_color(void)
{
    const int width = screen_rez != 0 ? SCREEN_BW_WIDTH : SCREEN_COL_WIDTH;
    const int skyColor = COLOR_BLUE_INDEX;
    const int floorColor = COLOR_STEEL_INDEX;
    unsigned short *scrPtr = screen_offs_adr;
	int lines;
	int i;

    for(lines=0; lines<viewscreen_sky_height; ++lines)        /* 50 lines */
    {
        for(i=0; i<(viewscreen_hcenter+viewscreen_halfwidth)/16; ++i)  /* 160 pixels of blue */
        {
            *scrPtr++ = (skyColor & 1) ? 0xFFFF : 0x0000;
            *scrPtr++ = (skyColor & 2) ? 0xFFFF : 0x0000;
            *scrPtr++ = (skyColor & 4) ? 0xFFFF : 0x0000;
            *scrPtr++ = (skyColor & 8) ? 0xFFFF : 0x0000;
        }
        scrPtr += (width-(viewscreen_hcenter+viewscreen_halfwidth))/8*sizeof(unsigned short);    /* skip 160 pixels */
    }
    for(lines=0; lines<=viewscreen_floor_height; ++lines)        /* 51 lines */
    {
        for(i=0; i<(viewscreen_hcenter+viewscreen_halfwidth)/16; ++i)  /* 160 pixels of dark grey */
        {
            *scrPtr++ = (floorColor & 1) ? 0xFFFF : 0x0000;
            *scrPtr++ = (floorColor & 2) ? 0xFFFF : 0x0000;
            *scrPtr++ = (floorColor & 4) ? 0xFFFF : 0x0000;
            *scrPtr++ = (floorColor & 8) ? 0xFFFF : 0x0000;
        }
        scrPtr += (width-(viewscreen_hcenter+viewscreen_halfwidth))/8*sizeof(unsigned short);    /* skip 160 pixels */
    }
}
