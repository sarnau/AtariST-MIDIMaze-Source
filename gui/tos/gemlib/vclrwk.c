#include "portab.h"
#include "gemdefs.h"
#include "vdilib.h"

void v_clrwk(int16_t handle)
{
	contrl[0] = 3;
	contrl[1] = 0;
	contrl[3] = 0;
	contrl[6] = handle;
	vdi();
}
