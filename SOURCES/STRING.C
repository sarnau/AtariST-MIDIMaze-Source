/************************************************************
 *** STRING.C
 ************************************************************/
#include "GLOBALS.H"


/************************************************************
 *** Just like ANSI-C strcpy(), except the parameters are swapped.
 ************************************************************/
void strcpy_srcdst(const char *srcPtr,char *destPtr) {
    while((*destPtr++ = *srcPtr++) != 0) {}
}
