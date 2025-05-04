#include "portab.h"
#include "gemdefs.h"
#include "crysbind.h"

int16_t form_alert(int16_t defbut, const char *astring)
{
	FM_DEFBUT = defbut;
	FM_ASTRING = NO_CONST(astring);
	return crys_if(FORM_ALERT);
}
