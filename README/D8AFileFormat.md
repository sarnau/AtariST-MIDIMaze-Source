# File Format for the MIDIMAZE.D8A File

The extension D8A stands for 'd-eight-a: data' as mentioned by Xanth Park in the article [FUJIBOINK! – BEHIND THE BIT PLANES](https://www.atarimagazines.com/startv1n2/Fujiboink.html)

The file is just an unstructured data file, which combines 5 files and is 0x1AF8E bytes long. Because MIDI Maze is an Atari ST game with a 68k CPU, all 16 bit values are encoded in big-endian.


| Offset | Length | Content |
|--------|--------|---------|
| 0x00000| 0x5076 | Color graphics of the main screen
| 0x05076| 0x5A62 | Monochrome graphics of the main screen
| 0x0AAD8| 0x0082 | First quarter of the sine table (65 words from 0…256 (0.0…1.0)
| 0x0AB5A| 0x0C64 | Ball images for Player and Shots (1586 words, compressed by height)
| 0x0B7BE| 0xF7D0 | Faces (20 * 1586 words for 20 different rotation image)

## Main Screens
The main screens are compressed and are decompressed via the decompress_image_to_screen() function. While being optimized for color, the same compression is applied to the monochrome image as well.


## Sine table
 Because MIDI Maze does not have any floating point math, a sine table was precalculated and stored in this file.  The sine table (used for rotations) only encodes the first 65 out of 256 values, which is the first quarter of the sine wave. All values are 16 bit integer numbers. You can calculated them like this: sin(value) * 256, resulting a 1.0 being stored as 256.

The rest of the values are then calculated from this data: the second quarter is qual to the first one, but in reverse order of values and the second half is equal to the first, except all values are negative.


## Images

The image for the ball is encoded without faces, which allows the ball to be the same for players and shots.

### The ball
24 different shapes are stored in the file, which are scaled into a size between 1 and 32:

```Python
    size = 24 - shapeIndex
    if(size > 16) size = (size - 16) * 2 + 16
```

Based on the size the shape is between 2…53 lines high and requires 1…4 words per line:

```Python
    height = ((size * 40) / 12 + 1) / 2
    shapeWidthInWords = (size  - 1) / 8 + 1
```

By putting this all together we can see that 1586 words are required to encode all 24 shapes:

Shape | Size | Height | Width | Words | Offset
-----:|-----:|-------:|------:|------:|------:
 0 | 32 | 53 | 4 |  212 | 0
 1 | 30 | 50 | 4 |  200 | 212
 2 | 28 | 47 | 4 |  188 | 412
 3 | 26 | 43 | 4 |  172 | 600
 4 | 24 | 40 | 3 |  120 | 772
 5 | 22 | 37 | 3 |  111 | 892
 6 | 20 | 33 | 3 |   99 | 1003
 7 | 18 | 30 | 3 |   90 | 1102
 8 | 16 | 27 | 2 |   54 | 1192
 9 | 15 | 25 | 2 |   50 | 1246
10 | 14 | 23 | 2 |   46 | 1296
11 | 13 | 22 | 2 |   44 | 1342
12 | 12 | 20 | 2 |   40 | 1386
13 | 11 | 18 | 2 |   36 | 1426
14 | 10 | 17 | 2 |   34 | 1462
15 |  9 | 15 | 2 |   30 | 1496
16 |  8 | 13 | 1 |   13 | 1526
17 |  7 | 12 | 1 |   12 | 1539
18 |  6 | 10 | 1 |   10 | 1551
19 |  5 |  8 | 1 |    8 | 1561
20 |  4 |  7 | 1 |    7 | 1569
21 |  3 |  5 | 1 |    5 | 1576
22 |  2 |  3 | 1 |    3 | 1581
23 |  1 |  2 | 1 |    2 | 1584

The shape doesn't contain any color information, because it is colored during drawing to match the players color. The shape is just a circle, so you could calculate it or just draw it, but this would require floating point math, which MIDI Maze lacks.

#### The shadow

You might have noticed, that the shadow under the shape is missing. That's because it is generated from the shape and not stored in the file.

The shadow height is 1/4 of the height of the shape, but at least one pixel high and it has the same width as the shape.

To scale the shape down, MIDI Maze does this very well and puts some effort in. If you would just take every 4th line of a shape to use as the shadow, you would end up with several shadows looking identical (because of it's 1/4 resolution), despite that the shape above would wary in size. To avoid this, MIDI Maze prefers the lines closer to the middle if a shape is larger, this results in a slightly larger shadow for a larger shape.

```Python
    shadowHeight = height / 4
    if shadowHeight == 0:
        shadowHeight = 1
    shadowHeightScale = shadowHeight-1
    if shadowHeightScale == 0:
        shadowHeightScale = 1
    for shadowLine in range(0,shadowHeight):
        shapeLine = ((((shapeHeight-1) * (shadowLine - (shadowHeight/2))) * 2) / shadowHeightScale + shapeHeight-1) / 2
        if shapeLine < 0:
            shapeLine = 0
```

Shape | Height | Shadow Height | Lines copied from Shape into Shadow
-----:|-----:|-------:|:-----
 0 | 53 | 13 |  0,4,8,13,17,21,26,30,34,39,43,47,52
 1 | 50 | 12 |  0,2,6,11,15,20,24,28,33,37,42,46
 2 | 47 | 11 |  0,4,9,13,18,23,27,32,36,41,46
 3 | 43 | 10 |  0,2,7,11,16,21,25,30,35,39
 4 | 40 | 10 |  0,2,6,10,15,19,23,28,32,36
 5 | 37 |  9 |  0,4,9,13,18,22,27,31,36
 6 | 33 |  8 |  0,2,6,11,16,20,25,29
 7 | 30 |  7 |  0,4,9,14,19,24,29
 8 | 27 |  6 |  0,2,7,13,18,23
 9 | 25 |  6 |  0,2,7,12,16,21
10 | 23 |  5 |  0,5,11,16,22
11 | 22 |  5 |  0,5,10,15,21
12 | 20 |  5 |  0,4,9,14,19
13 | 18 |  4 |  0,2,8,14
14 | 17 |  4 |  0,2,8,13
15 | 15 |  3 |  0,7,14
16 | 13 |  3 |  0,6,12
17 | 12 |  3 |  0,5,11
18 | 10 |  2 |  0,4
19 |  8 |  2 |  0,3
20 |  7 |  1 |  3
21 |  5 |  1 |  2
22 |  3 |  1 |  1
23 |  2 |  1 |  0


### The face and frame
The face and frame is drawn on top of the player ball.

There are 20 possible different faces, resulting in one image for every 18 degrees (360deg/20 = 18deg).

Face 0 is the frontal view, also used for several other views showing a smily face. 1…9 are looking to the right, 10 is no face (the back of the player), 11…19 are looking to the left with 19 being close to 0 again.


### Other images

All other images used within MIDI Maze (the notes, the crossed through marker) are encoded directly within the application and are not part of this file.
