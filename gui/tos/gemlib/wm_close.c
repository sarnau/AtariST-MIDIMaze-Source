#include "portab.h"
#include "gemdefs.h"
#include "crysbind.h"

int16_t wind_close(int16_t handle)
{
	WM_HANDLE = handle;
	return crys_if(WIND_CLOSE);
}
