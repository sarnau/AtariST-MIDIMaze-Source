#include "portab.h"
#include "gemdefs.h"
#include "crysbind.h"

int16_t wind_update(int16_t beg_update)
{
	WM_BEGUP = beg_update;
	return crys_if(WIND_UPDATE);
}
