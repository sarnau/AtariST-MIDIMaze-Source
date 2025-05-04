#include "portab.h"
#include "gemdefs.h"
#include "crysbind.h"

int16_t form_center(OBJECT *tree, int16_t *pcx, int16_t *pcy, int16_t *pcw, int16_t *pch)
{
	FM_FORM = tree;
	crys_if(FORM_CENTER);
	*pcx = FM_XC;
	*pcy = FM_YC;
	*pcw = FM_WC;
	*pch = FM_HC;
	return RET_CODE;
}
