#include "GLOBALS.H"

// The compressed title is always encoded as a color image (160 bytes per line, 200 lines)
// It is expected that the destination buffer has the same size!
#define SCR_LINEOFFSET 160
#define SCR_HEIGHT 200

/*
File format of the compressed title screens
-------------------------------------------

The Atari ST 320x200 16 color screen format is organized in 4 planes which
allow 2^4 = 16 colors. A plane is 16 pixels (= 16 bits = 1 word) wide. With
20 packages per line we have 20 * 4 Planes * 16 Pixel = 160 bytes per lines.
200 lines * 160 = 32000 bytes for a full screen image.

In black/white we only have one plane and 80 bytes per line, resulting in 640
horizontal pixel and 400 vertical, which also results in 32000 bytes for a full
screen black/white image.

Screen layout of the 320x200 16 color format:

              Pixel 0..15    |   Pixel 16..31    | ...
         | 4 planes=16 colors| 4 planes=16 colors|
         +----+----+----+----+----+----+----+----+-
Line   0 | P0 | P1 | P2 | P3 | P0 | P1 | P2 | P3 |
         +----+----+----+----+----+----+----+----+-
Line   1 |    |    |    |    |    |    |    |    | 
         +----+----+----+----+----+----+----+----+-
                             ...
         +----+----+----+----+----+----+----+----+-
Line 198 |    |    |    |    |    |    |    |    | 
         +----+----+----+----+----+----+----+----+-
Line 199 |    |    |    |    |    |    |    |    | 
         +----+----+----+----+----+----+----+----+-

This algorithm assumes that changes in the color plane are more rare when going
vertically through the picture and it allows skipping an empty run (it is assumed
that the screen is fully erased before decompression).

In simple words: it copies n vertical words to the screen, then skips m vertical
words. If the bottom of the screen (line 199) is reached, it jumps back up
line 0 of the next plane and continues.


Binary file format (every word is in big-endian):

WORD header1 number of words of image data (not used for decompression)
WORD header2 number of bytes in the encode table. Following the
             encode table starts the image data.

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

*/

/*
decompress_image_to_screen:
      MOVEA.L   4(A7),A0 ; unsigned short *bytesW
      MOVEA.L   8(A7),A2 ; unsigned short *screenPtrW
      MOVE.L    A3,-(A7)
      LEA       mult_color_tab,A3 ; 16-bit 160* multiplication table with 256 entries
      MOVE.W    #SCR_HEIGHT,D2  ; start with 200 lines
      ADDQ.L    #2,A0    ; we do not need the number of image words
      MOVE.W    (A0)+,D0 ; byteCount
      LEA       0(A0,D0.W),A1 ; start of image data
      ASR.W     #1,D0    ; convert into a word count
      BRA       decompLoopStart

decompLoop:
      CLR.W     D1
      MOVE.B    (A0)+,D1 ; number of lines
      SUB.W     D1,D2    ; does the data fit into the remaining lines?
      BGT       L0A4B    ; => yes
      BRA       decompOverflowLine
L0A4A:MOVE.W    (A1)+,(A2)
      LEA       SCR_LINEOFFSET(A2),A2 ; the screen is oriented in 160 bytes per line
L0A4B:DBF       D1,L0A4A
      BRA       decompSkipLines

decompOverflowLine:
      ADD.W     D1,D2
      SUB.W     D2,D1
      BRA       L0A4E
L0A4D:MOVE.W    (A1)+,(A2) ; copy the remaining lines
      LEA       SCR_LINEOFFSET(A2),A2
L0A4E:DBF       D2,L0A4D

      ; overflow into the next plane
      MOVE.W    #SCR_HEIGHT,D2
      SUB.W     D1,D2
      LEA       -SCR_LINEOFFSET*SCR_HEIGHT+2(A2),A2 ; skip to the next pixel row
      BRA       L0A50
L0A4F:MOVE.W    (A1)+,(A2) ; copy the leftover lines
      LEA       SCR_LINEOFFSET(A2),A2
L0A50:DBF       D1,L0A4F

decompSkipLines:
      CLR.W     D1
      MOVE.B    (A0)+,D1 ; number of skipped lines
      SUB.W     D1,D2 ; any lines left?
      BLS       decompSkipOverflowLines
decompSkipLines2:
      ADD.W     D1,D1
      MOVE.W    0(A3,D1.W),D1 ; offset += 160 * lines
      LEA       0(A2,D1.W),A2

decompLoopStart:
      DBRA       D0,decompLoop
      MOVEA.L   (A7)+,A3
      RTS

decompSkipOverflowLines:
      ADD.W     #SCR_HEIGHT,D2 ; skip to the next column
      LEA       -SCR_LINEOFFSET*SCR_HEIGHT+2(A2),A2
      BRA.S     decompSkipLines2
*/


void decompress_image_to_screen(unsigned short *bytesW, unsigned short *screenPtrW)
{
	unsigned char *bytes = (unsigned char*)bytesW;
	unsigned char *screenPtr = (unsigned char*)screenPtrW;
    int byteCount = (bytes[2] << 8) + bytes[3];
    const unsigned char	*offsetDataPtr = bytes + sizeof(short) + sizeof(short);
    const unsigned char	*imageDataPtr = offsetDataPtr + byteCount;
    byteCount /= 2;
    int linesLeft = SCR_HEIGHT;
    while(byteCount-- > 0)
    {
        int	numberOflines = *offsetDataPtr++;
        if(linesLeft > numberOflines)
        {
            linesLeft -= numberOflines;
            // copy numberOflines into the screen
            while(numberOflines-- > 0)
            {
                screenPtr[0] = *imageDataPtr++;
                screenPtr[1] = *imageDataPtr++;
                screenPtr += SCR_LINEOFFSET;
            }
        } else {
            numberOflines -= linesLeft;
            // copy leftover lines in the current column into the screen
            while(linesLeft-- > 0)
            {
                screenPtr[0] = *imageDataPtr++;
                screenPtr[1] = *imageDataPtr++;
                screenPtr += SCR_LINEOFFSET;
            }
            linesLeft = SCR_HEIGHT - numberOflines;
            screenPtr -= SCR_LINEOFFSET*SCR_HEIGHT-sizeof(short);
            // copy numberOflines in the next column into the screen
            while(numberOflines-- > 0)
            {
                screenPtr[0] = *imageDataPtr++;
                screenPtr[1] = *imageDataPtr++;
                screenPtr += SCR_LINEOFFSET;
            }
        }
        int numberOfSkippedlines = *offsetDataPtr++;
        linesLeft -= numberOfSkippedlines;
        if(linesLeft <= 0)
        {
            linesLeft += SCR_HEIGHT;
            screenPtr -= SCR_LINEOFFSET*SCR_HEIGHT-sizeof(short);
        }
        screenPtr += SCR_LINEOFFSET * numberOfSkippedlines;
    }
}
