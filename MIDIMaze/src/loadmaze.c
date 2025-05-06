/************************************************************
 *** LOAD_MAZE.C
 ************************************************************/
#include "globals.h"

short maze_size;

/************************************************************
 *** int LoadMaze(const char *pathname,const char *filename)
 ************************************************************/
int LoadMaze(const char *pathname,const char *filename) {
char alertBuf[64];
char tmp;
char lineBuf[MAZE_MAX_SIZE + 2];
int error;
int fhandle;
int fieldX;
int fieldY;
static char const alert_mazefile_header_booboo[] = "[3][ |Maze file|header boo-boo][OK]";

    error = NO;

    /* open the MAZ file */
    fhandle = (int)Fopen(pathname, 0);
    if(fhandle < 0) {
        /* Generate an alert text with the filename, if the maze file can't be opened */
        fieldY = 0;
        fieldX = 0;
        while((alertBuf[fieldY++] = "[3][ |Can't open|\""[fieldX++]) != 0) {}
        fieldY--;
        fieldX = 0;
        while((alertBuf[fieldY++] = filename[fieldX++]) != 0) {}
        fieldY--;
        fieldX = 0;
        while((alertBuf[fieldY++] = "\"][OK]"[fieldX++]) != 0) {}
        form_alert(1, alertBuf);
        return FAILURE;
    }

    /* the first 2 bytes are the size of the maze as ASCII numbers */
    Fread(fhandle, 1, &tmp);
    if(tmp < '0' || tmp > '9') {
        form_alert(1, alert_mazefile_header_booboo);
        return -3;
    }
    maze_size = (tmp-'0')*10;
    Fread(fhandle, 1, &tmp);
    if(tmp < '0' || tmp > '9') {
        form_alert(1, alert_mazefile_header_booboo);
        return -3;
    }
    maze_size += tmp-'0';

    /* read over the CR/LF (because a MAZ-File is an Atari ST text file) */
    Fread(fhandle, 1, &tmp);
#if ALLOW_JUST_LF_IN_MAZ_FILES
    if (tmp == '\r')
#endif
        Fread(fhandle, 1, &tmp);

    for(fieldY = 0; fieldY <= MAZE_MAX_SIZE-1; fieldY++) {
        /* read a line of the MAZ (+ 3, because the final 'X' and CR/LF at the end) */
        if(fieldY <= maze_size){
#if ALLOW_JUST_LF_IN_MAZ_FILES
            if(fieldY <= maze_size && Fread(fhandle, maze_size+3, &lineBuf) != maze_size+3) {
                 form_alert(1, "[3][ |Error reading|maze file][OK]");
                 return -2;
             if(fieldY <= maze_size){
                 if (Fread(fhandle, maze_size+2, lineBuf) != maze_size+2) {
                     form_alert(1, "[3][ |Error reading|maze file][OK]");
                     return -2;
                 }
                 if (lineBuf[maze_size + 1] == '\r')
                     Fread(fhandle, 1, &tmp);
             }
#else
            if (Fread(fhandle, maze_size+2, lineBuf) != maze_size+2) {
                form_alert(1, "[3][ |Error reading|maze file][OK]");
                return -2;
            }
#endif
            if (lineBuf[maze_size + 1] == '\r')
                Fread(fhandle, 1, &tmp);
        }
        for(fieldX = 0; fieldX <= MAZE_MAX_SIZE-1; fieldX++) {
            if(fieldY <= maze_size && fieldX <= maze_size) {
                /* on odd-coordinates there have to be an empty space '.', otherwise the maze is defect */
                if((fieldY & 1) == 1 && (fieldX & 1) == 1 && lineBuf[fieldX] != '.')
                    error = YES;
                if(lineBuf[fieldX] == 'X') /* a wall */
                    set_maze_data(fieldY, fieldX, MAZE_FIELD_WALL);
                else if(lineBuf[fieldX] == '.') /* an empty space */
                    set_maze_data(fieldY, fieldX, MAZE_FIELD_EMPTY);
                else /* an unknown character */
                    error = YES;
            } else { /* all the area outsize of the maze is filled with 1x1 boxes */
                set_maze_data(fieldY, fieldX, (fieldY & fieldX & 1) ? MAZE_FIELD_EMPTY : MAZE_FIELD_WALL);
            }
        }
    }
    Fclose(fhandle);
    if(error)
        form_alert(1, "[3][ |Maze file|format boo-boo][Who cares?]");
    return SUCCESS;
}
