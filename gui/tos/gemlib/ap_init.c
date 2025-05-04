#include "portab.h"
#include "gemdefs.h"
#include "crysbind.h"

int16_t appl_init(void)
{
	crys_if(APPL_INIT);
	return gl_apid = RET_CODE;
}
