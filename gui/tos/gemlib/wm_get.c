#include "portab.h"
#include "gemdefs.h"
#include "crysbind.h"

int16_t wind_get(int16_t w_handle, int16_t w_field, int16_t *pw1, int16_t *pw2, int16_t *pw3, int16_t *pw4)
{
	WM_HANDLE = w_handle;
	WM_WFIELD = w_field;
	crys_if(WIND_GET);
	*pw1 = WM_OX;
	*pw2 = WM_OY;
	*pw3 = WM_OW;
	*pw4 = WM_OH;
	return RET_CODE;
}
