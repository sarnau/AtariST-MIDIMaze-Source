/************************************************************
 *** Embedded RSC file and the AES initialization/exit code
 ************************************************************/
#include "globals.h"
#include "rstest.h"

/*
 * types needed by generated inline resource
 */
#define WORD short
#define LONG intptr_t
#define BYTE char

#undef NUM_STRINGS
#undef NUM_FRSTR
#undef NUM_UD
#undef NUM_IMAGES
#undef NUM_BB
#undef NUM_FRIMG
#undef NUM_IB
#undef NUM_CIB
#undef NUM_TI
#undef NUM_OBS
#undef NUM_TREE
#include "rstest.rsh"


static short pxy[8];
static OBJECT *rsrc_tree_ptr; /* unused */
static MFDB src_MFDB;
static MFDB dest_MFDB;
OBJECT *rsrc_object_array[NUM_TREE]; /* adresses of all used resource trees found at rs_trindex[] */
short wind_handle;
short vdi_handle;

/************************************************************
 *** Copy the window background from logbase as a redraw
 ************************************************************/
void redraw_window_background(int wi_ghandle) {
short wi_gw_h;
short wi_gw_w;
short wi_gw_y;
short wi_gw_x;
short wi_gfield;

    wind_update(BEG_UPDATE);
    wi_gfield = WF_FIRSTXYWH;
    do {
        wind_get(wi_ghandle, wi_gfield, &wi_gw_x, &wi_gw_y, &wi_gw_w, &wi_gw_h);
        if(wi_gw_w == 0 || wi_gw_h == 0) break;
        pxy[0] = pxy[4] = wi_gw_x;
        pxy[1] = pxy[5] = wi_gw_y;
        pxy[2] = pxy[6] = wi_gw_x+wi_gw_w-1;
        pxy[3] = pxy[7] = wi_gw_y+wi_gw_h-1;
        vro_cpyfm(vdi_handle, S_ONLY, pxy, &src_MFDB, &dest_MFDB);
        wi_gfield = WF_NEXTXYWH;
    } while(1);
    wind_update(END_UPDATE);
}

/************************************************************
 *** int init_aes_window(void)
 ************************************************************/
int init_aes_window(void) {
int index;
int obj;
OBJECT *tree;

    rs_object[6].ob_x--; /* fix the position of the Atari menu title box */
    index = 0;
    rsrc_tree_ptr = rs_object; /* unused */
    /* fix the whole resource tree */
    for(obj = 0; obj < NUM_OBS; obj++) {
        rsrc_obfix(rs_object, obj);
        if(rs_object[obj].ob_type == G_TITLE || rs_object[obj].ob_type == G_STRING || rs_object[obj].ob_type == G_BUTTON) {
            rs_object[obj].ob_spec = (intptr_t)rs_strings[index++];
        } else if(rs_object[obj].ob_type == G_FBOXTEXT) {
            index += 3; /* that is setup further down */
        }
    }
    /* generate an address table for all used trees */
    for(obj = 0; obj < NUM_TREE; obj++)
        rsrc_object_array[obj] = rs_trindex[obj]+rs_object;

    /* The background of the play dialog is transparent  */
    tree = rsrc_object_array[RSCTREE_PLAY_DIALOG];
    tree[0].ob_type = G_IBOX;
    ((char *)tree[DUMB_VAL].ob_spec)[1] = '0';
    ((char *)tree[PLAIN_VAL].ob_spec)[1] = '0';
    ((char *)tree[NINJA_VAL].ob_spec)[1] = '0';
    /* setup the TEDINFO for the set name dialog */
    rsrc_object_array[RSCTREE_SET_NAME][1].ob_spec = (intptr_t)&rs_tedinfo[0];
    rs_tedinfo[0].te_ptext = rs_strings[(intptr_t)rs_tedinfo[0].te_ptext];
    rs_tedinfo[0].te_ptmplt = rs_strings[(intptr_t)rs_tedinfo[0].te_ptmplt];
    rs_tedinfo[0].te_pvalid = rs_strings[(intptr_t)rs_tedinfo[0].te_pvalid];

    /* show the menu bar */
    menu_bar(rsrc_object_array[RSCTREE_MENU], 1);

    /* copy from logbase to Physbase() to restore the background in a redraw */
    src_MFDB.fd_addr = screen_ptr[0];
    dest_MFDB.fd_addr = Physbase();
    src_MFDB.fd_w = dest_MFDB.fd_w = screen_rez ? SCREEN_BW_WIDTH : SCREEN_COL_WIDTH;
    src_MFDB.fd_h = dest_MFDB.fd_h = screen_rez ? SCREEN_BW_HEIGHT : SCREEN_COL_HEIGHT;
    src_MFDB.fd_wdwidth = dest_MFDB.fd_wdwidth = screen_rez ? (SCREEN_BW_WIDTH/16) : (SCREEN_COL_WIDTH/16);
    src_MFDB.fd_stand = dest_MFDB.fd_stand = 0;
    src_MFDB.fd_nplanes = dest_MFDB.fd_nplanes = screen_rez ? 1 : 4;
    pxy[0] = 0; pxy[1] = 0; pxy[2] = screen_rez ? (SCREEN_BW_WIDTH-1) : (SCREEN_COL_WIDTH-1); pxy[3] = screen_rez ? (SCREEN_BW_HEIGHT-1) : (SCREEN_COL_HEIGHT-1);
    pxy[4] = 0; pxy[5] = 0; pxy[6] = screen_rez ? (SCREEN_BW_WIDTH-1) : (SCREEN_COL_WIDTH-1); pxy[7] = screen_rez ? (SCREEN_BW_HEIGHT-1) : (SCREEN_COL_HEIGHT-1);

    /* Create a window. The window is larger than the screen to hide the borders */
    wind_handle = wind_create(0, -1, screen_rez ? 18 : 10, screen_rez ? (SCREEN_BW_WIDTH+2) : (SCREEN_COL_WIDTH+2), screen_rez ? (SCREEN_BW_HEIGHT-9) : (SCREEN_COL_HEIGHT-9));
    if(wind_handle < 0) {
        form_alert(1, "[3][ |Can't open| window!][OK]");
        return FAILURE;
    }
    wind_open(wind_handle, -1, screen_rez ? 18 : 10, screen_rez ? (SCREEN_BW_WIDTH+2) : (SCREEN_COL_WIDTH+2), screen_rez ? (SCREEN_BW_HEIGHT-9) : (SCREEN_COL_HEIGHT-9));

    return SUCCESS;
}

/************************************************************
 *** void exit_aes_window(void)
 ************************************************************/
void exit_aes_window(void) {
    wind_close(wind_handle);
    wind_delete(wind_handle);
    /* hide the menu bar */
    menu_bar(rsrc_object_array[RSCTREE_MENU], 0);
}
