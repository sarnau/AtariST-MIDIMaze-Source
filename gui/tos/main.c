/************************************************************
 *** main()
 ************************************************************/
#include "globals.h"

#ifdef __GNUC__
/* avoid references to __main */
void __main(void);
void __main(void)
{
}
#endif

/************************************************************
 *** This code only does some AES/VDI/XBIOS init and then calls setup_game()
 ************************************************************/
int main(void) {
static short vdi_handle;
int i;
short savedColors[16];
short intout[57];
short intin[11];

    /* Set the system up to do GEM calls */
    appl_init();
    graf_mouse(M_OFF, 0);

    /* Get the handle of the desktop */
    vdi_handle = graf_handle(&dummy, &dummy, &dummy, &dummy);

    /* Open the workstation */
    for(i = 0; i < 10; i++)
        intin[i] = 1;
    intin[10] = 2;
    v_opnvwk(intin, &vdi_handle, intout);
    /* clear the graphics */
    v_clrwk(vdi_handle);
    /* save the current color palette */
    for(i = 0; i < 16; i++)
        savedColors[i] = Setcolor(i, -1);

    setup_game();

    /* clear the graphics */
    v_clrwk(vdi_handle);
    /* reset colors */
    Setpalette((void*)&savedColors);
    /* close the workstation */
    v_clsvwk(vdi_handle);
    appl_exit();
    return 0;
}
