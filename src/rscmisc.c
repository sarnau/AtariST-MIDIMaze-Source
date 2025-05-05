/************************************************************
 *** Routines to show dialogs with/without buttons
 ************************************************************/
#include "globals.h"


/************************************************************
 *** Show a dialog without any button (used for Slaves)
 ************************************************************/
void rsc_draw_buttonless_dialog(int treeIndex) {
short fo_cx;
short fo_cy;
short fo_cw;
short fo_ch;

    form_center(rsrc_object_array[treeIndex], &fo_cx, &fo_cy, &fo_cw, &fo_ch);
    form_dial(FMD_START, fo_cx, fo_cy, fo_cw, fo_ch, fo_cx, fo_cy, fo_cw, fo_ch);
    objc_draw(rsrc_object_array[treeIndex], 0, 999, fo_cx, fo_cy, fo_cw, fo_ch);
    form_dial(FMD_FINISH, fo_cx, fo_cy, fo_cw, fo_ch, fo_cx, fo_cy, fo_cw, fo_ch);
}

/************************************************************
 *** Show a dialog with a button (only used for the About box)
 ************************************************************/
void do_about_dialog(int treeIndex) {
short fo_cx;
short fo_cy;
short fo_cw;
short fo_ch;

    form_center(rsrc_object_array[treeIndex], &fo_cx, &fo_cy, &fo_cw, &fo_ch);
    form_dial(FMD_START, fo_cx, fo_cy, fo_cw, fo_ch, fo_cx, fo_cy, fo_cw, fo_ch);
    objc_draw(rsrc_object_array[treeIndex], 0, 999, fo_cx, fo_cy, fo_cw, fo_ch);
    form_do(rsrc_object_array[treeIndex], 0);
    form_dial(FMD_FINISH, fo_cx, fo_cy, fo_cw, fo_ch, fo_cx, fo_cy, fo_cw, fo_ch);
    graf_mouse(M_OFF, NULL);
    redraw_window_background(wind_handle);
    graf_mouse(M_ON, NULL);
}
