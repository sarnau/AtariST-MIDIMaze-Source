#include "portab.h"
#include "gemdefs.h"
#include "crysbind.h"

int16_t evnt_multi(int16_t flags, int16_t bclk, int16_t bmsk, int16_t bst,
	int16_t m1flags, int16_t m1x, int16_t m1y, int16_t m1w, int16_t m1h,
	int16_t m2flags, int16_t m2x, int16_t m2y, int16_t m2w, int16_t m2h,
	int16_t * mepbuff,
	int16_t tlc, int16_t thc,
	int16_t *pmx, int16_t *pmy, int16_t *pmb, int16_t *pks, int16_t *pkr, int16_t *pbr)
{
	MU_FLAGS = flags;

	MB_CLICKS = bclk;
	MB_MASK = bmsk;
	MB_STATE = bst;

	MMO1_FLAGS = m1flags;
	MMO1_X = m1x;
	MMO1_Y = m1y;
	MMO1_WIDTH = m1w;
	MMO1_HEIGHT = m1h;

	MMO2_FLAGS = m2flags;
	MMO2_X = m2x;
	MMO2_Y = m2y;
	MMO2_WIDTH = m2w;
	MMO2_HEIGHT = m2h;

	MME_PBUFF = mepbuff;

	MT_LOCOUNT = tlc;
	MT_HICOUNT = thc;

	crys_if(EVNT_MULTI);

	*pmx = EV_MX;
	*pmy = EV_MY;
	*pmb = EV_MB;
	*pks = EV_KS;
	*pkr = EV_KRET;
	*pbr = EV_BRET;
	return RET_CODE;
}
