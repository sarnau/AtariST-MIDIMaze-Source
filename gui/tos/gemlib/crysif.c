#include "portab.h"
#include "gemdefs.h"
#include "crysbind.h"

int16_t control[C_SIZE];
int16_t global[G_SIZE];
int16_t int_in[I_SIZE];
int16_t int_out[O_SIZE];
void *addr_in[AI_SIZE];
void *addr_out[AO_SIZE];
AESPB aespb = { control, global, int_in, int_out, addr_in, addr_out };
int16_t gl_apid;

int16_t crys_if(int16_t opcode)
{
	const char *pctrl;
	int16_t *pc = control;

	*pc++ = opcode;
	pctrl = &ctrl_cnts[(opcode - 10) * 3];
	*pc++ = *pctrl++;
	*pc++ = *pctrl++;
	*pc = *pctrl;

	crystal(&aespb);
	return RET_CODE;
}
