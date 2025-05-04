#include "portab.h"
#include "gemdefs.h"
#include "crysbind.h"

int16_t menu_bar(OBJECT *tree, int16_t showit)
{
	MM_ITREE = tree;
	SHOW_IT = showit;
	return crys_if(MENU_BAR);
}
