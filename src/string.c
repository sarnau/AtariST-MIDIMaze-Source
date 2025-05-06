/************************************************************
 *** STRING.C
 ************************************************************/
#include "globals.h"


/************************************************************
 *** Just like ANSI-C strcpy(), except the parameters are swapped.
 ************************************************************/
void strcpy_srcdst(const char *srcPtr,char *destPtr) {
    while((*destPtr++ = *srcPtr++) != 0) {}
}


#ifdef __GNUC__
#include <string.h>
/*
 * newer compilers may optimize some loops to strlen() calls
 */
size_t strlen(const char *s)
{
	const char *start = s;

	while (*s++)
		;

	return s - start - 1;
}
#endif
