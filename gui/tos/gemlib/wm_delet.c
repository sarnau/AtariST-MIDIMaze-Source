#include "portab.h"
#include "gemdefs.h"
#include "crysbind.h"

int16_t wind_delete(int16_t handle)
{
	WM_HANDLE = handle;
	return crys_if(WIND_DELETE);
}
