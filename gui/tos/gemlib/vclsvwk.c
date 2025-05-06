#include "portab.h"
#include "gemdefs.h"
#include "vdilib.h"

void v_clsvwk(int16_t handle)
{
	contrl[0] = 101;
	contrl[1] = 0;
	contrl[3] = 0;
	contrl[6] = handle;
	vdi();
}
