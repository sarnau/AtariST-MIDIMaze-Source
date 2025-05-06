#include "portab.h"
#include "gemdefs.h"
#include "crysbind.h"

int16_t wind_create(int16_t kind, int16_t wx, int16_t wy, int16_t ww, int16_t wh)
{
	WM_KIND = kind;
	WM_WX = wx;
	WM_WY = wy;
	WM_WW = ww;
	WM_WH = wh;
	return crys_if(WIND_CREATE);
}
