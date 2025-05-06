# File format of the compressed title screens

The Atari ST 320x200 16 color screen format is organized in 4 planes which
allow 2^4 = 16 colors. A plane is 16 pixels (= 16 bits = 1 word) wide. With
20 packages per line we have 20 * 4 Planes * 16 Pixel = 160 bytes per lines.
200 lines * 160 = 32000 bytes for a full screen image.

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

This algorithm assumes that changes in the color plane are more rare when going
vertically through the picture and it allows skipping an empty run (it is assumed
that the screen is fully erased before decompression).

In simple words: it copies n vertical words to the screen, then skips m vertical
words. If the bottom of the screen (line 199) is reached, it jumps back up
line 0 of the next plane and continues.


Binary file format (every word is in big-endian):

    WORD header1 number of words of image data (not used for decompression)
    WORD header2 number of bytes in the encode table. Following the encode table starts the image data.

    encodeTable
      BYTE number of lines of WORDs to copy
      BYTE number of skipped lines of WORDs to copy
      ...

    imageData
      WORD image data 1
      WORD image data 2
      WORD ...
      ...

The file size can be calculated as 4 (2 WORD header) + header2 + header1 * 2
