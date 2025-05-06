/************************************************************
 *** DRAWWALLS.C
 ************************************************************/
#include "globals.h"

static int objecttable_set_wall(int x1,int h1,int x2,int h2,int color,int leftRightFlag);
static int draw_mazes_clip_wall(int *py1,int *px1,int *py2,int *px2);

static XY_SPEED_TABLE viewmatrix_delta[9][17]; /* [depth][width]; home is (7.7). The table contains the distance from home in cell units (MAZE_CELL_SIZE * field), negative values are in front and left of home */

/************************************************************
 *** Calculate a viewmatrix in cell units based on the sub-positioning
 *** of a viewpoint inside a field 
 ************************************************************/
void draw_maze_calc_viewmatrix(int microY,int microX,int minYOffset,int minXOffset,int maxYOffset,int maxXOffset,int isFlipped,int dir) {
register int i;
register int j;
register int j2;
register int k;
int maxXDelta;
int maxYDelta;
int minXDelta;
int minYDelta;

    /* set the 4 corners of our view field */
    viewmatrix_delta[0][0].deltaY = minYDelta = minYOffset-microY;
    viewmatrix_delta[0][0].deltaX = minXDelta = minXOffset-microX;
    viewmatrix_delta[8][16].deltaY = maxYDelta = maxYOffset-microY;
    viewmatrix_delta[8][16].deltaX = maxXDelta = maxXOffset-microX;
    if(!isFlipped) {
        viewmatrix_delta[8][0].deltaY = maxYDelta;
        viewmatrix_delta[8][0].deltaX = minXDelta;
        viewmatrix_delta[0][16].deltaY = minYDelta;
        viewmatrix_delta[0][16].deltaX = maxXDelta;
    } else {
        viewmatrix_delta[8][0].deltaY = minYDelta;
        viewmatrix_delta[8][0].deltaX = maxXDelta;
        viewmatrix_delta[0][16].deltaY = maxYDelta;
        viewmatrix_delta[0][16].deltaX = minXDelta;
    }

    /* now rotate the corners by our viewing direction  */
    rotate2d(&viewmatrix_delta[0][0].deltaY, &viewmatrix_delta[0][0].deltaX, dir);
    rotate2d(&viewmatrix_delta[8][0].deltaY, &viewmatrix_delta[8][0].deltaX, dir);
    rotate2d(&viewmatrix_delta[0][16].deltaY, &viewmatrix_delta[0][16].deltaX, dir);
    rotate2d(&viewmatrix_delta[8][16].deltaY, &viewmatrix_delta[8][16].deltaX, dir);

    /* now interpolate the columns in the first and last row */
    for(i = 8; i; i >>= 1) {
        for(j = i; j < 16; j += i+i) {
            viewmatrix_delta[0][j].deltaY = (viewmatrix_delta[0][j-i].deltaY+viewmatrix_delta[0][j+i].deltaY)>>1;
            viewmatrix_delta[0][j].deltaX = (viewmatrix_delta[0][j-i].deltaX+viewmatrix_delta[0][j+i].deltaX)>>1;
            viewmatrix_delta[8][j].deltaY = (viewmatrix_delta[8][j-i].deltaY+viewmatrix_delta[8][j+i].deltaY)>>1;
            viewmatrix_delta[8][j].deltaX = (viewmatrix_delta[8][j-i].deltaX+viewmatrix_delta[8][j+i].deltaX)>>1;
        }
    }
    /* and then the cells in between to complete the matrix */
    for(j = 0; j <= 16; j++) {
        for(j2 = 4; j2; j2 >>= 1) {
            for(k = j2; k < 8; k += j2+j2) {
                viewmatrix_delta[k][j].deltaY = (viewmatrix_delta[k-j2][j].deltaY+viewmatrix_delta[k+j2][j].deltaY)>>1;
                viewmatrix_delta[k][j].deltaX = (viewmatrix_delta[k-j2][j].deltaX+viewmatrix_delta[k+j2][j].deltaX)>>1;
            }
        }
    }
#if 0
    /* print the matrix (distance is the line; home is 7,7) */
    printf("     ");
    for(j=0; j<=16; ++j) {
        printf("     %2d     ", j);
    }
    printf("\n");
    for(j=0; j<=8; ++j) {
        printf("%2d : ", j);
        for(i=0; i<=16; ++i) {
            printf("%5.2f,%5.2f ", (float)viewmatrix_delta[j][i].deltaY/MAZE_CELL_SIZE, (float)viewmatrix_delta[j][i].deltaX/MAZE_CELL_SIZE);
        }
        printf("\n");
    }
    printf("\n");
#endif
}

/************************************************************
 *** Add a 3D wall to the objecttable. The wall is in relative
 *** distance to the viewing position (0,0) as full field-offsets
 *** For horizontal walls x1p==x2p and y1p=y2p+1
 *** For vertical walls: x1p=x2p+1 and y1p=y2p
 *** So technically only 3 parameters are needed: x1p, y1p and vertical/horizontal flag
 *** color allows support for 2 different color shades (only used on color monitors),
 *** vertial walls have a different color than horizontal ones (no matter how it is rotated)
 ************************************************************/
int draw_mazes_set_wall(int y1p,int x1p,int y2p,int x2p,int color,int leftRightFlag) {
int x2;
int y2;
int x1;
int y1;

    /* Get the distance relative to the viewing position for the wall. Negative values are front/left. Positive values are behind/right */
    y1 = viewmatrix_delta[y1p][x1p].deltaY;
    x1 = viewmatrix_delta[y1p][x1p].deltaX;
    y2 = viewmatrix_delta[y2p][x2p].deltaY;
    x2 = viewmatrix_delta[y2p][x2p].deltaX;
    if(draw_mazes_clip_wall(&y1, &x1, &y2, &x2)) { /* is the wall visible (clip to viewport, if necessary) */
        /* The wall is visible, if necessary clipped to the viewport */
        /* Now the yx coordinates are converted into screen coordinates: x and height */
        calc_yx_to_xh(&y1, &x1); /* convert y1 into scr_x1, x1 into scr_h1 */
        calc_yx_to_xh(&y2, &x2); /* convert y2 into scr_x2, x2 into scr_h2 */
        return objecttable_set_wall(y1, x1, y2, x2, color, leftRightFlag);
    }
    return YES; /* wall not drawn */
}


/************************************************************
 *** objecttable management
 ************************************************************/

/*
 table_list[] is a sorted table (by xleft), which contains ranges of coordinates.
 This is used for clipping detection: if walls cover the whole window,
 then there is no point in calculating or even drawing the walls behind.

 Not sure how the size of table_list[] was picked. 10 seems fine, because you
 probably need an incredible amount of complexity to get a higher value.
 As long as the viewport width is 90 degrees wide, the number of complex walls
 at the first level probably can never be that high.

 It works as follows: walls are one-by-one; each wall by X1/X2.
 They all have the same height, so there is no Y necessary (front walls are always
 taller than walls behind them). The list is searched if X1 is already within a wall segment
 and also if X2 is already within a wall segment. If that is the case, all segmente
 from [X1:X2] are merged into one single one. This can eventually result in one single
 segment covering the whole view. If the horizon is visible, because the wall is too far away,
 it can be e.g. two segments: [0:XA] and [XB:windowwidth] with the horizon being visible
 between XA and XB.
*/
static short table_size; /* number of entries in table_list[] */
static struct {
    short xleft;
    short xright;
} table_list[20];

static void objecttable_add(int xleft,int xright);
static int objecttable_search(int x1,int startIndex,int *pFoundFlag);
static void objecttable_shift_table(int lowerIndex,int upperIndex);

/************************************************************
 *** void objecttable_clear(void)
 ************************************************************/
void objecttable_clear(void) {
    table_size = 0;
}

/************************************************************
 *** Add a wall to the draw list. This has to be done front-to-back.
 *** Does clipping detection.
 ************************************************************/
int objecttable_set_wall(int x1,int h1,int x2,int h2,int color,int leftRightFlag) {
int xright;
int xleft;

    /* sort x1 and x2 */
    if(x1 <= x2) {
        xleft = x1;
        xright = x2;
    } else {
        xleft = x2;
        xright = x1;
    }

    /* depending if we draw a wall left or right of the centerline, we check for visibility */
    /* This is a performance optimization (you could comment it out - for lower performance) */
    if(leftRightFlag) {
        /* Is everything from 0 to the right side of the wall already covered? */
        if(objecttable_check_if_hidden(0, xright))
            return YES; /* then don't draw the wall */
    } else {
        /* Is everything from left side of the wall to the right side of the window already covered? */
        if(objecttable_check_if_hidden(xleft, viewscreen_hcenter+viewscreen_halfwidth))
            return YES; /* then don't draw the wall */
    }

    /* Only add the wall, if the wall is not hidden */
    if(!objecttable_check_if_hidden(xleft, xright)) {
        objecttable_add(xleft, xright);
        to_draw_list(DRAW_TYPE_WALL, color, x1, h1, x2, h2);
    }
    return NO; /* wall will be drawn */
}

/************************************************************
 *** int objecttable_check_if_hidden(int xleft,int xright)
 ************************************************************/
int objecttable_check_if_hidden(int xleft,int xright) {
int fullWidth;
int ret;
int i;

    /* full width of the viewing area */
    fullWidth = viewscreen_hcenter+viewscreen_halfwidth-1;

    if(xright < 0) /* right position < the left side of the viewing area? */
        return TRUE; /* object is invisible */
    if(xleft > fullWidth) /* left position > than the width of the viewing area? */
        return TRUE; /* also: the object has to be invisble */

    /* clipping to the viewing area */
    if(xleft < 0) xleft = 0;
    if(xright > fullWidth) xright = fullWidth;

    /* scan our object table, if the object is already hidden behind another one */
    ret = FALSE;
    for(i = 0; i < table_size; i++) {
        /* an object is hidden, if it is fully behind any other object in the table */
        if(table_list[i].xleft <= xleft && table_list[i].xright >= xright) {
            ret = TRUE;
            break;
        }
    }
    return ret;
}

/************************************************************
 *** Check if a single object covers the full view, because then
 *** we are already done
 ************************************************************/
int objecttable_check_view_fully_covered(void) {
    /* This is the case, if there is only one object in the table left (objects get merged) */
    /* that starts at 0 and has the full width */
    return table_size == 1 && table_list[0].xleft == 0 && viewscreen_hcenter+viewscreen_halfwidth == table_list[0].xright;
}

/************************************************************
 *** void objecttable_add(int xleft,int xright)
 ************************************************************/
void objecttable_add(int xleft,int xright) {
int newxright;
int newxleft;
int foundxrightFlag;
int foundxleftFlag;
int xrightIndex;
int xleftIndex;

    /* Is the left coordinate in the table? */
    xleftIndex = objecttable_search(xleft, 0, &foundxleftFlag);
    /* Is the right coordinate in the table (start searching at our left position) */
    xrightIndex = objecttable_search(xright, xleftIndex, &foundxrightFlag);
    /* if left was found, use it's left value, other use our new one */
    newxleft = foundxleftFlag ? table_list[xleftIndex].xleft : xleft;
    if(foundxrightFlag) { /* did we also find the right one? */
        /* then use the old right one as our match  */
        newxright = table_list[xrightIndex].xright;
        /* fold all entries from [xleftIndex:xrightIndex-1] into one */
        objecttable_shift_table(xleftIndex, xrightIndex-1);
    } else {
        /* the new right one, is our right one */
        newxright = xright;
        /* add an entry at xleftIndex, move the rest of the table up */
        objecttable_shift_table(xleftIndex+1, xrightIndex-1);
    }
    /* new larger entry ranging from [newxleft:newxright] */
    table_list[xleftIndex].xleft = newxleft;
    table_list[xleftIndex].xright = newxright;
}

/************************************************************
 *** Search the (sorted) table_list[] if x1 is within an
 *** existing segment
 ************************************************************/
int objecttable_search(int x1,int startIndex,int *pFoundFlag) {
int i;

    *pFoundFlag = FALSE; /* no match found */
    for(i = startIndex; i < table_size; i++) {
        if(table_list[i].xleft > x1) /* if left is already larger than our x1, we didn't find it (requires table_list[] to be sorted) */
            break;
        if(table_list[i].xright >= x1) { /* Is x1 within the segment? */
            *pFoundFlag = TRUE; /* found it */
            break;
        }
    }
    return i; /* return the found segment, test *pFoundFlag it there actually was a match */
}

/************************************************************
 *** Remove no longer needed entries in the table_list[]
 ************************************************************/
void objecttable_shift_table(int lowerIndex,int upperIndex) {
int width;

    /* size of the range */
    width = upperIndex-lowerIndex+1;
    if(width > 0) { /* more than one entry? */
        /* copy from the upperIndex down to the lowerIndex */
        /* This removes all elements from [lowerIndex:upperIndex]  */
        upperIndex++;
        while(upperIndex < table_size) {
            table_list[lowerIndex].xleft = table_list[upperIndex].xleft;
            table_list[lowerIndex++].xright = table_list[upperIndex++].xright;
        }
        table_size = lowerIndex; /* the table size is now shorter */
    } else {
        if(width >= 0) return; /* actually a ==0 test, meaning: nothing to do */

        /* we need to create an entry in the table */
        /* shift everything from lowerIndex+1 upwards, which creates one new entry at lowerIndex */
        for(upperIndex = table_size; upperIndex >= lowerIndex; upperIndex--) {
            table_list[upperIndex].xleft = table_list[upperIndex-1].xleft;
            table_list[upperIndex].xright = table_list[upperIndex-1].xright;
        }
        table_size++;
    }
}


/************************************************************
 *** This routine calculates the intersection of 2 vectors to do clipping of the walls.
 ***
 *** The viewport has a 90 degrees field of view: 45 degrees to the left
 *** and 45 degrees to the right. Any wall outside of this, will be
 *** clipped to exactly the intersection of this line with the wall.
 ***
 *** The generic form for a line–line intersection with 2 points for each line is:
 *** ix = ((x1*y2-y1*x2)*(x3-x4)) / ((x1-x2)*(y3-y4) - (y1-y2)*(x3-x4))
 *** iy = ((x1*y2-y1*x2)*(y3-y4)) / ((x1-x2)*(y3-y4) - (y1-y2)*(x3-x4))
 *** For this routine y3,x3 is (0,0) and y4,x4 is (slope,1) for slope==-1|1 or (1,0) for slope == 0 
 ***
 *** The code is optimized for the 3 special cases needed:
 *** The first vector is the actual wall: y1,x1 - y2,x2
 *** The second vector starts at 0,0 and is defined by the slope (only 3 values are possible: -1, 1 and 0)
 *** Also: The player _always_ looks down (y for visible walls is negative)
 ***
 *** slope =  1: a 45 degree to the "left" of our field of view
 ***             Returns y,x where the wall should end, because the field of view ends
 ***             Because of the 45 degree angle: y == x
 *** slope = -1: a 45 degree to the "right" of our field of view
 ***             Returns y,x where the wall should end, because the field of view ends
 ***             Because of the 45 degree angle: y == -x
 *** slope =  0: 0 degrees, on the Y axis.
 ***             Returns the where a wall would hit our view axis, if it would be extended.
 ***
 *** The function returns 1, if the calculation did make sense.
 *** In case there is no useful value (no intersection), it returns 0. 
 ************************************************************/
static int draw_mazes_wall_intersection(int y1,int x1,int y2,int x2,int slope,int *py,int *px) {
register int divisor;
register int diff;
register int deltaY;
register int deltaX;

    deltaY = y1; deltaY -= y2; /* delta of wall in Y */
    deltaX = x2; deltaX -= x1; /* delta of wall in X */
    divisor = y1*deltaX; divisor += x1*deltaY; /* divisor = y1*(x2-x1) + x1*(y1-y2) */
    if(slope == 1) { /* 45 degrees to the left */
        diff = deltaX;
        if(diff += deltaY) { /* avoid a divison by zero */
            *py = *px = divisor/diff; /* (y1*(x2-x1) + x1*(y1-y2)) / ((x2-x1)+(y1-y2)) */
            return YES;
        }
    } else if(slope == -1) { /* 45 degrees to the right */
        diff = deltaY;
        if(diff -= deltaX) { /* avoid a divison by zero */
            *py = -(*px = divisor/diff); /* (y1*(x2-x1) + x1*(y1-y2)) / ((y1-y2)-(x2-x1)) */
            return YES;
        }
    } else { /* slope == 0: Where does the line hit the Y-axis? */
        if(deltaX) { /* avoid a divison by zero (= line parallel to Y axis) */
            /* if `y = ax + b`, then this will calculate `b`. We don't need `a` */
            *py = divisor/deltaX; /* (y1*(x2-x1) + x1*(y1-y2)) / (x2-x1) */
            return YES;
        }
    }
    return NO;
}

/************************************************************
 *** Check that y1,x1 < y2,x2 < y3,x3 or y3,x3 > y2,x2 > y1,x1
 ************************************************************/
static int draw_mazes_check_order(int y1,int x1,int y2,int x2,int y3,int x3) {
int xCoordOkFlag;
int yCoordOkFlag;

    yCoordOkFlag = (y1 <= y3) ? (y1 <= y2 && y2 <= y3) : (y3 <= y2 && y2 <= y1);
    xCoordOkFlag = (x1 <= x3) ? (x1 <= x2 && x2 <= x3) : (x3 <= x2 && x2 <= x1);
    return yCoordOkFlag && xCoordOkFlag;
}

/************************************************************
 *** Check if a wall is visible and - if necessary - clip to the
 *** viewport. The viewport is 90 degrees view, so 45 degrees
 *** left and right to the main viewing axis.
 *** 
 *** Y1/X1 and Y2/X2 are in cell units. Negative values are front/left.
 *** Positive values are back/right.
 ***
 *** Example positions looking straight north (Coordinates are in Y,X - no fractions, to keep it simple)
 *** **,** is 0,0; our view position
 Y X:  0     1     2     3     4     5     6     7     8     9     10    11    12    13    14    15    16     
 0 : -7,-7 -7,-6 -7,-5 -7,-4 -7,-3 -7,-2 -7,-1 -7, 0 -7, 0 -7, 1 -7, 2 -7, 3 -7, 4 -7, 5 -7, 6 -7, 7 -7, 8 
 1 : -6,-7 -6,-6 -6,-5 -6,-4 -6,-3 -6,-2 -6,-1 -6, 0 -6, 0 -6, 1 -6, 2 -6, 3 -6, 4 -6, 5 -6, 6 -6, 7 -6, 8 
 2 : -5,-7 -5,-6 -5,-5 -5,-4 -5,-3 -5,-2 -5,-1 -5, 0 -5, 0 -5, 1 -5, 2 -5, 3 -5, 4 -5, 5 -5, 6 -5, 7 -5, 8 
 3 : -4,-7 -4,-6 -4,-5 -4,-4 -4,-3 -4,-2 -4,-1 -4, 0 -4, 0 -4, 1 -4, 2 -4, 3 -4, 4 -4, 5 -4, 6 -4, 7 -4, 8 
 4 : -3,-7 -3,-6 -3,-5 -3,-4 -3,-3 -3,-2 -3,-1 -3, 0 -3, 0 -3, 1 -3, 2 -3, 3 -3, 4 -3, 5 -3, 6 -3, 7 -3, 8 
 5 : -2,-7 -2,-6 -2,-5 -2,-4 -2,-3 -2,-2 -2,-1 -2, 0 -2, 0 -2, 1 -2, 2 -2, 3 -2, 4 -2, 5 -2, 6 -2, 7 -2, 8 
 6 : -1,-7 -1,-6 -1,-5 -1,-4 -1,-3 -1,-2 -1,-1 -1, 0 -1, 0 -1, 1 -1, 2 -1, 3 -1, 4 -1, 5 -1, 6 -1, 7 -1, 8 
 7 :  0,-7  0,-6  0,-5  0,-4  0,-3  0,-2  0,-1 **,**  0, 0  0, 1  0, 2  0, 3  0, 4  0, 5  0, 6  0, 7  0, 8 
 8 :  1,-7  1,-6  1,-5  1,-4  1,-3  1,-2  1,-1  1, 0  1, 0  1, 1  1, 2  1, 3  1, 4  1, 5  1, 6  1, 7  1, 8 
 ************************************************************/
int draw_mazes_clip_wall(int *py1,int *px1,int *py2,int *px2) {
int tmp;
int x;
int y;
int xy2InViewFlag;
int xy1InViewFlag;

    /* Is Y1/X1 within a 90 degree angle in front of the player? */
    /* If so, then we always draw the wall! The walls are always */
    /* fully visible, never clipped in width or height  */
    xy1InViewFlag = (*px1 >= 0) ? *py1 < -(*px1) : *py1 < *px1;
    /* Same test for Y2/X2 */
    xy2InViewFlag = (*px2 >= 0) ? *py2 < -(*px2) : *py2 < *px2;
    if(xy1InViewFlag && xy2InViewFlag)
        return YES; /* draw the wall */

    /* wall fully outside of the 90 degree angle? */
    if(!xy1InViewFlag && !xy2InViewFlag) {
        if(*px1 * *px2 >= 0) /* X1 and X2 have the same sign? */
            return NO; /* ignore the wall, it is too far out */

        /* Check where the wall will hit the our viewing axis. If this is */
        /* behind us, then we look "along" the wall just outside of our */
        /* field of view and it doesn't need to be drawn. */
        if(draw_mazes_wall_intersection(*py1, *px1, *py2, *px2, 0, &y, &x)) {
            if(y >= 0) /* is the wall at our level or behind us? */
                return NO; /* ignore the wall */

            /* Make sure x1,y1 is smaller than x2,y2 */
            if(*px1 > *px2) {
                /* swap coordinates */
                tmp = *py1; *py1 = *py2; *py2 = tmp;
                tmp = *px1; *px1 = *px2; *px2 = tmp;
            }
            /* clip the left side of the wall */
            if(draw_mazes_wall_intersection(*py1, *px1, *py2, *px2, 1, &y, &x)) {
                *py1 = y;
                *px1 = x;
            }
            /* clip the right side of the wall */
            if(draw_mazes_wall_intersection(*py1, *px1, *py2, *px2, -1, &y, &x)) {
                *py2 = y;
                *px2 = x;
            }
            return YES; /* draw the wall */
        }
        return NO; /* wall is not visible */
    }

    /* only part of the wall outside of our 90 degree angle? */
    if(xy2InViewFlag) { /* Y2/X2 is inside? */
        /* swap coordinates, so that only Y2/X2 is gonna be clipped */
        tmp = *py1; *py1 = *py2; *py2 = tmp;
        tmp = *px1; *px1 = *px2; *px2 = tmp;
    }
    /* now: Y1/X1 is always inside, but Y2/X2 is always outside */
    
    /* try shorten the right side of the wall, only set, if the shortening worked. */
    /* In the other case the intersection point will be way out. That's what draw_mazes_check_order() will check and avoid. */
    if(draw_mazes_wall_intersection(*py1, *px1, *py2, *px2, 1, &y, &x) && draw_mazes_check_order(*py1, *px1, y, x, *py2, *px2)) {
        *py2 = y;
        *px2 = x;
    }
    /* try shorten the left side of the wall, only set, if the shortening worked. */
    /* In the other case the intersection point will be way out. That's what draw_mazes_check_order() will check and avoid. */
    if(draw_mazes_wall_intersection(*py1, *px1, *py2, *px2, -1, &y, &x) && draw_mazes_check_order(*py1, *px1, y, x, *py2, *px2)) {
        *py2 = y;
        *px2 = x;
    }
    return YES; /* draw the wall */
}
