#include "portab.h"
#include "gemdefs.h"
#include "crysbind.h"

int16_t appl_exit(void)
{
	crys_if(APPL_EXIT);
	return TRUE;
}
