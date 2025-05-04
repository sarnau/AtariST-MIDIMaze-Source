/************************************************************
 *** Managing the double buffering of the screens
 *** Also opens the window, checks the copy protection and
 *** shows the title screen by loading it from MIDIMAZE.D8A
 ************************************************************/
#include "globals.h"

static int load_color_titlescreen(void);
static int load_bw_titlescreen(void);

static char screenBuffer[32255];
static short screen_offset;
short screen_flag;
unsigned short *screen_ptr[2];
unsigned short *screen_offs_adr;

short midimaze_d8a_filehandle;

/************************************************************
 *** int init_screen(void)
 ************************************************************/
int init_screen(void) {
register unsigned short *srcScreenPtr;
register unsigned short *destScreenPtr;
register long i;

    set_screen_offs(viewscreen_wind_offset);

    /* screen #0 = private screenbuffer */
    screen_ptr[screen_flag = 0] = (unsigned short *)(((long)&screenBuffer[255]) & ~0xff);
    /* screen #1 = official screenbuffer at application launch */
    screen_ptr[1] = (unsigned short*)Physbase();

    /* setup AES with it's window */
    if(init_aes_window() < 0)
        return FAILURE;

    /* the copy protection returns a number != 0, if detected correctly */
    protectionIsValid = check_copy_protection();

    /* open our assets file */
    midimaze_d8a_filehandle = Fopen("MIDIMAZE.D8A", 0);
    if(midimaze_d8a_filehandle < 0) {
        switch_org_scr();
        form_alert(1, "[3][ |Boo-boo|opening|MIDIMAZE.D8A][OK]");
        return FAILURE;
    }

    /* depending on the screen resolution load the correct title screen */
    /* It is loaded in screen #0 (screen_flag), which is not visible yet */
    if((screen_rez ? load_bw_titlescreen() : load_color_titlescreen()) < 0)
        return FAILURE;

    /* switch logbase and physbase. This will show the just loaded title */
    switch_screens();

    /* copy the title screen from current physbase to logbase */
    destScreenPtr = screen_ptr[screen_flag];
    srcScreenPtr = screen_ptr[screen_flag^1];
    for(i = 0; i < 16000; i++)
        *destScreenPtr++ = *srcScreenPtr++;

    /* switch back to original physbase (because of further AES usage) */
    Setscreen(screen_ptr[1], screen_ptr[1], -1);
    screen_flag = 0; /* logbase is screen #0 */

    /* copy the maze window and the kills screen areas, so they can be */
    /* restored after an AES window requires a redraw */
    save_part_of_screen_for_dialog(0);

    return SUCCESS;
}

/************************************************************
 *** int load_color_titlescreen(void)
 ************************************************************/
int load_color_titlescreen(void) {
    /* our MIDImaze color table */
    Setpalette((void*)colortable);
    /* read the color title image */
    Fread(midimaze_d8a_filehandle, 20598, &load_buffer);
    decompress_image_to_screen((unsigned short *)load_buffer, screen_ptr[screen_flag]);
    /* now read the b/w title image (but ignore it) */
    /* This should have been an Fseek(), which would be much better performance */
    Fread(midimaze_d8a_filehandle, 23138, &load_buffer);

    return SUCCESS;
}

/************************************************************
 *** int load_bw_titlescreen(void)
 ************************************************************/
int load_bw_titlescreen(void) {
    /* no idea why this is here */
    Vsync();
    (void)Setcolor(0, 0); /* do not invert the background color */
    /* read the color title image first (but ignore it) */
    /* This should have been an Fseek(), which would be much better performance */
    Fread(midimaze_d8a_filehandle, 20598, &load_buffer);
    /* now read the b/w title image and decompress onto the screen */
    Fread(midimaze_d8a_filehandle, 23138, &load_buffer);
    decompress_image_to_screen((unsigned short *)load_buffer, screen_ptr[screen_flag]);

    return SUCCESS;
}

/************************************************************
 *** Update the screen offset address. It always points into the logbase.
 ************************************************************/
void set_screen_offs(int offset) {
    screen_offset = offset;
    screen_offs_adr = (unsigned short *)((char*)screen_ptr[screen_flag]+screen_offset);
}

/************************************************************
 *** Double-buffering: switch between logbase and physbase
 ************************************************************/
void switch_screens(void) {
    screen_flag ^= 1;
    screen_offs_adr = (unsigned short *)((char*)screen_ptr[screen_flag]+screen_offset);
    Setscreen(screen_ptr[screen_flag], screen_ptr[screen_flag^1], -1);
}

/************************************************************
 *** Toggle screens, but only switch the logbase.
 *** This allows filling both screens with the same data.
 ************************************************************/
void switch_logbase(void) {
    screen_flag ^= 1;
    screen_offs_adr = (unsigned short *)((char*)screen_ptr[screen_flag]+screen_offset);
    Setscreen(screen_ptr[screen_flag], (void*)-1, -1);
}

/************************************************************
 *** Switch back to original screenbuffer at application launch
 ************************************************************/
void switch_org_scr(void) {
    Setscreen(screen_ptr[1], screen_ptr[1], -1);
}
