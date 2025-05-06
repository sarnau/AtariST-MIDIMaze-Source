#include "portab.h"
#include "gemdefs.h"
#include "crysbind.h"

int16_t graf_handle(int16_t *pwchar, int16_t *phchar, int16_t *pwbox, int16_t *phbox)
{
	crys_if(GRAF_HANDLE);
	*pwchar = GR_WCHAR;
	*phchar = GR_HCHAR;
	*pwbox = GR_WBOX;
	*phbox = GR_HBOX;
	return RET_CODE;
}
