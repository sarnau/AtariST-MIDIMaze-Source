#include "portab.h"
#include "gemdefs.h"
#include "crysbind.h"

int16_t menu_tnormal(OBJECT *tree, int16_t titlenum, int16_t normalit)
{
	MM_ITREE = tree;
	TITLE_NUM = titlenum;
	NORMAL_IT = normalit;
	return crys_if(MENU_TNORMAL);
}
