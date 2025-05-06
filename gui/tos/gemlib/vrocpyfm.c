#include "portab.h"
#include "gemdefs.h"
#include "vdilib.h"

/*
 * avoid dereferencing a type-casted ptr
 */
#define i_ptr(p) t = (void **)&contrl[7]; *t = (void *)(p)
#define i_ptr2(p) t = (void **)&contrl[9]; *t = (void *)(p)


void vro_cpyfm(int16_t handle, int16_t wr_mode, int16_t * xy, MFDB *srcMFDB, MFDB *desMFDB)
{
	void **t;

	intin[0] = wr_mode;
	i_ptr(srcMFDB);
	i_ptr2(desMFDB);
	i_ptsin(xy);

	contrl[0] = 109;
	contrl[1] = 4;
	contrl[3] = 1;
	contrl[6] = handle;
	vdi();

	i_ptsin(ptsin);
}
