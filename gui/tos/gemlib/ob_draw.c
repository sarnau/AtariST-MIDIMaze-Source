#include "portab.h"
#include "gemdefs.h"
#include "crysbind.h"

int16_t objc_draw(OBJECT *tree, int16_t drawob, int16_t depth, int16_t xc, int16_t yc, int16_t wc, int16_t hc)
{
	OB_TREE = tree;
	OB_DRAWOB = drawob;
	OB_DEPTH = depth;
	OB_XCLIP = xc;
	OB_YCLIP = yc;
	OB_WCLIP = wc;
	OB_HCLIP = hc;
	return crys_if(OBJC_DRAW);
}
