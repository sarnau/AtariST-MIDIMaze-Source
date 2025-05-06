#include "portab.h"
#include "gemdefs.h"
#include "crysbind.h"

int16_t graf_mouse(int16_t m_number, const MFORM *m_addr)
{
	GR_MNUMBER = m_number;
	GR_MADDR = NO_CONST(m_addr);
	return crys_if(GRAF_MOUSE);
}
