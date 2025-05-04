#include "portab.h"
#include "gemdefs.h"
#include "crysbind.h"

int16_t rsrc_obfix(OBJECT *tree, int16_t obj)
{
	RS_TREE = tree;
	RS_OBJ = obj;
	return crys_if(RSRC_OBFIX);
}
