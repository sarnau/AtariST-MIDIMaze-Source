# Atari ST Screen Bitmap

The Atari ST 320x200 16 color screen format is organized in 4 planes which
allows 2^4 = 16 colors. A plane is 16 pixels (= 16 bits = 1 word) wide. With
20 groups per line we have 20 * 4 planes * 16 pixel = 160 bytes per lines.
200 lines * 160 = 32000 bytes for a full screen image.

Not used by MIDImaze is the 640x200 screen format with 2 planes and 4 colors.

In black/white we only have one plane and 80 bytes per line, resulting in 640
horizontal pixel and 400 vertical, which also results in 32000 bytes for a full
screen black/white image.

Screen layout of the 320x200 16 color format:

    |       |   Pixel 0..15   ||||   Pixel 16..31  ||||
    |       | 4 planes=16 col |||| 4 planes=16 col ||||
    --------|----|----|----|----|----|----|----|----|
    |Line 0 | P0 | P1 | P2 | P3 | P0 | P1 | P2 | P3 |
    |Line 1 | P0 | P1 | P2 | P3 | P0 | P1 | P2 | P3 |
    |Line 2 | P0 | P1 | P2 | P3 | P0 | P1 | P2 | P3 |
    |…      |    |    |    |    |    |    |    |    |
