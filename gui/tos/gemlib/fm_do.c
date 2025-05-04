#include "portab.h"
#include "gemdefs.h"
#include "crysbind.h"

int16_t form_do(OBJECT *form, int16_t start)
{
	FM_FORM = form;
	FM_START = start;
	return crys_if(FORM_DO);
}
