#include "portab.h"
#include "gemdefs.h"
#include "crysbind.h"

int16_t fsel_input(char *pipath, char *pisel, int16_t *pbutton)
{
	FS_IPATH = pipath;
	FS_ISEL = pisel;
	crys_if(FSEL_INPUT);
	*pbutton = FS_BUTTON;
	return RET_CODE;
}
