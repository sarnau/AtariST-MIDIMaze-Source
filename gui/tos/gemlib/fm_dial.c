#include "portab.h"
#include "gemdefs.h"
#include "crysbind.h"

int16_t form_dial(int16_t dtype, int16_t ix, int16_t iy, int16_t iw, int16_t ih, int16_t x, int16_t y, int16_t w, int16_t h)
{
	FM_TYPE = dtype;
	FM_IX = ix;
	FM_IY = iy;
	FM_IW = iw;
	FM_IH = ih;
	FM_X = x;
	FM_Y = y;
	FM_W = w;
	FM_H = h;
	return crys_if(FORM_DIAL);
}
