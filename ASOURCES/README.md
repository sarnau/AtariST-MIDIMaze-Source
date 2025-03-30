# ASOURCES

This directory contains all the code which was written in 68000 assembly code. This is mostly around line drawing and blitting shapes (implemented in 320x200x16 color and 640x400x2 black/white), but also the original copy protection, the decompression code for the title screen and a fast 16x16/16 bit signed multiplication/division. I've kept the original source code, but converted it by hand into ANSI-C code, so you can run it on todays machines. For many this is also easier to read, despite that the flow is identical to the assembly code.

These are the sources, converted to ANSI-C, however with the assembly code as a comment in front of it. They are in the same order as they are in the linked MIDImaze binary.

## `BLIT_DRAW_SHAPE_BW.C`
Draw a shape in black/white. Imporant: `imageMask` points _behind_ the actual image. `halfHeight` is the height in lines divided by 2. This makes the height identical to color. This the two lines are necessary for dithering. The ditherTable is the replacement for the color.

    void blit_draw_shape_bw(int x, int y, const unsigned short *imageMask, int widthInWords, int halfHeight, const unsigned short *ditherTable)

## `MULS_DIVS.C`
Multiply and divide 3 16-bit signed integer values with the temporary result being 32-bit wide.

## `CALC_TAB.C`/`CALC_TAB.H`

Sets up two multiplcation tables: one for color (* 160) and one for black/white (* 80)

I've also added the global structures with bitmasks to this source. These structures are shared between the color and the black/white code.

## `DRAW_BW.C`
Line drawing functions for black/white. Only horizontal and vertical ones are available. The `_double` are used for the 3D view, which is rendering in double-lines (for dithering, otherwise colors would be impossible in black/white). The `blit_clear_window_bw` fills the 3D view with the sky and floor pattern.

    void blit_draw_hline_bw(int x1, int x2, int y, int col)
    void blit_draw_hline_bw_double(int x1, int x2, int y, int col)
    void blit_draw_vline_bw(int y1, int y2, int x, int col)
    void blit_fill_box_bw(int x1, int y1, int x2, int y2, int col)
    void blit_fill_box_bw_double(int x1, int y1, int x2, int y2, int col)
    void blit_clear_window_bw()

The source also contains the `bw_fillpattern_table[16+3]` table with the dithering patterns. The table is 3 larger than the available colors, because dithering requires 4 entries.

## `CHECK_COPY_PROT.C`

## `BLIT_DRAW_SHAPE_COL.C`
Draw a shape in color. Imporant: `imageMask` points _behind_ the actual image. colorFuncPtr comes from `col_setcolor_jumptable[]`, which is taking the color as an index and returns a pointer to a small assembler function for setting/clearing the correct bits in the 4 planes.

    void blit_draw_shape_color(int x, int y, const unsigned short *imageMask, int widthInWords, int height, void (*colorFuncPtr)(unsigned short *,int,int,int))

## `DRAW_COL.C`
Line drawing functions for color. Only horizontal and vertical ones are available. The `blit_clear_window_color` fills the 3D view with the sky (blue) and floor (steel) colors.

    void blit_draw_hline_color(int x1, int x2, int y, int col)
    void blit_draw_vline_color(int y1, int y2, int x, int col)
    void blit_fill_box_color(int x1, int y1, int x2, int y2, int col)
    void blit_clear_window_color()

## `DECOMPRESSTITLE.C`
Decompress the title screen. More details in [File format of the compressed title screens](../README/CompressedTitle.md)

## `DELAY.S`
A short delay by maybe 100 cycles. Delays like this were typically used when programming registers directly, but it is not used anywhere.
