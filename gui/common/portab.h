#include <stdio.h>
#include <stdlib.h>

#define NON_ATARI_HACK 1 // Because we don't support MIDI, we disable the master/slave recognition at launch
#define ATARI_LONG_HACK 1 // A long on a 68000k is 32 bit big-endian. This fix makes it work either way. 

extern void dumpMapInfos(void);
extern void dumpPlayerInfo(void);

#define INTELSWAP16(a) ((unsigned short)(((unsigned short)a >> 8) | ((unsigned short)a << 8)))
#define INTELSWAP32(a) ((unsigned int)((((unsigned int)a << 24) & 0xFF000000L) | (((unsigned int)a << 8) & 0x00FF0000L) | (((unsigned int)a >> 8) & 0x0000FF00L) | (((unsigned int)a >> 24) & 0x000000FFL)))


// This is all Atari specific code, which shipped with the compiler:

#ifndef NO
    #define NO 0            /* "FALSE" */
#endif
#ifndef YES
    #define YES 1            /* "TRUE" */
#endif

#define TRUE 1
#define FALSE 0

#define FAILURE    (-1)        /* Function failure return val */
#define SUCCESS    (0)        /* Function success return val */

// Global Types
typedef struct
{
    long        cb_pcontrol; /* 0 */
    long        cb_pglobal; /* 4 */
    long        cb_pintin; /* 8 */
    long        cb_pintout; /* 12 */
    long        cb_padrin; /* 16 */
    long        cb_padrout; /* 20 */
} CBLK; /* 24 */

typedef struct
{
    int mf_xhot; /* 0 */
    int mf_yhot; /* 2 */
    int mf_nplanes; /* 4 */
    int mf_fg; /* 6 */
    int mf_bg; /* 8 */
    int mf_mask[16]; /* 10 */
    int mf_data[16]; /* 42 */
} MFORM; /* 74 */

typedef struct
{
    void *fd_addr; /* 0 */
    int fd_w; /* 4 */
    int fd_h; /* 6 */
    int fd_wdwidth; /* 8 */
    int fd_stand; /* 10 */
    int fd_nplanes; /* 12 */
    int fd_rl; /* 14 */
    int fd_r2; /* 16 */
    int fd_r3; /* 18 */
} MFDB; /* 20 */

typedef struct
{
    int ob_next; /* 0 */
    int ob_head; /* 2 */
    int ob_tail; /* 4 */
    unsigned int ob_type; /* 6 */
    unsigned int ob_flags; /* 8 */
    unsigned int ob_state; /* 10 */
    long ob_spec; /* 12 */
    int ob_x; /* 16 */
    int ob_y; /* 18 */
    int ob_width; /* 20 */
    int ob_height; /* 22 */
} OBJECT; /* 24 */

typedef struct
{
    char    *te_ptext;        /* 0 */
    char    *te_ptmplt;       /* 4 */
    char    *te_pvalid;       /* 8 */
    int    te_font;           /* 12 */
    int    te_fontid;         /* 14 */
    int    te_just;           /* 16 */
    int    te_color;          /* 18 */
    int    te_fontsize;       /* 20 */
    int    te_thickness;      /* 22 */
    int    te_txtlen;         /* 24 */
    int    te_tmplen;         /* 26 */
} TEDINFO; /* 28 */

/* Mouse forms */

#define ARROW             0
#define TEXT_CRSR         1
#define HOURGLASS         2
#define BUSYBEE           2
#define POINT_HAND        3
#define FLAT_HAND         4
#define THIN_CROSS        5
#define THICK_CROSS       6
#define OUTLN_CROSS       7
#define USER_DEF        255
#define M_OFF           256
#define M_ON            257

/****** Event definitions ***********************************************/

#define MU_KEYBD        0x0001
#define MU_BUTTON       0x0002
#define MU_M1           0x0004
#define MU_M2           0x0008
#define MU_MESAG        0x0010
#define MU_TIMER        0x0020

#define MN_SELECTED     10
#define WM_REDRAW       20
#define WM_TOPPED       21
#define WM_CLOSED       22
#define WM_FULLED       23
#define WM_ARROWED      24
#define WM_HSLID        25
#define WM_VSLID        26
#define WM_SIZED        27
#define WM_MOVED        28
#define WM_NEWTOP       29
#define WM_UNTOPPED     30
#define WM_ONTOP        31
#define AC_OPEN         40
#define AC_CLOSE        41
#define CT_UPDATE       50
#define CT_MOVE         51
#define CT_NEWTOP       52
#define AP_TERM         50
#define AP_TFAIL        51
#define AP_RESCHG       57
#define SHUT_COMPLETED  60
#define RESCHG_COMPLETED  61
#define AP_DRAGDROP	    63
#define CH_EXIT         80



/* Keybord states */

#define K_RSHIFT        0x0001
#define K_LSHIFT        0x0002
#define K_CTRL          0x0004
#define K_ALT           0x0008

/****** Object definitions **********************************************/

#define G_BOX           20
#define G_TEXT          21
#define G_BOXTEXT       22
#define G_IMAGE         23
#define G_USERDEF       24
#define G_IBOX          25
#define G_BUTTON        26
#define G_BOXCHAR       27
#define G_STRING        28
#define G_FTEXT         29
#define G_FBOXTEXT      30
#define G_ICON          31
#define G_TITLE         32
#define G_CICON         33


/* Object flags */

#define NONE            0x0000
#define SELECTABLE      0x0001
#define DEFAULT         0x0002
#define EXIT            0x0004
#define EDITABLE        0x0008
#define RBUTTON         0x0010
#define LASTOB          0x0020
#define TOUCHEXIT       0x0040
#define HIDETREE        0x0080
#define INDIRECT        0x0100


/* Object states */

#define NORMAL          0x00
#define SELECTED        0x01
#define CROSSED         0x02
#define CHECKED         0x04
#define DISABLED        0x08
#define OUTLINED        0x10
#define SHADOWED        0x20

#define FMD_START       0
#define FMD_GROW        1
#define FMD_SHRINK      2
#define FMD_FINISH      3

/****** Window definitions **********************************************/

#define NAME    0x0001
#define CLOSER  0x0002
#define FULLER  0x0004
#define MOVER   0x0008
#define INFO    0x0010
#define SIZER   0x0020
#define UPARROW 0x0040
#define DNARROW 0x0080
#define VSLIDE  0x0100
#define LFARROW 0x0200
#define RTARROW 0x0400
#define HSLIDE  0x0800

#define WF_KIND          1
#define WF_NAME          2
#define WF_INFO          3
#define WF_WORKXYWH      4
#define WF_CURRXYWH      5
#define WF_PREVXYWH      6
#define WF_FULLXYWH      7
#define WF_HSLIDE        8
#define WF_VSLIDE        9
#define WF_TOP          10
#define WF_FIRSTXYWH	11
#define WF_NEXTXYWH     12
#define WF_RESVD        13
#define WF_NEWDESK      14
#define WF_HSLSIZE      15
#define WF_VSLSIZE      16
#define WF_SCREEN       17
#define WF_COLOR        18
#define WF_DCOLOR       19
#define WF_OWNER        20
#define WF_BEVENT       24
#define WF_BOTTOM       25

#define W_BOX            0
#define W_TITLE          1
#define W_CLOSER         2
#define W_NAME           3
#define W_FULLER         4
#define W_INFO           5
#define W_DATA           6
#define W_WORK           7
#define W_SIZER          8
#define W_VBAR           9
#define W_UPARROW       10
#define W_DNARROW       11
#define W_VSLIDE        12
#define W_VELEV         13
#define W_HBAR          14
#define W_LFARROW       15
#define W_RTARROW       16
#define W_HSLIDE        17
#define W_HELEV         18

#define WA_UPPAGE 0
#define WA_DNPAGE 1
#define WA_UPLINE 2
#define WA_DNLINE 3
#define WA_LFPAGE 4
#define WA_RTPAGE 5
#define WA_LFLINE 6
#define WA_RTLINE 7

#define WC_BORDER 0                     /* wind calc flags */
#define WC_WORK   1

#define END_UPDATE 0                    /* update flags */
#define BEG_UPDATE 1
#define END_MCTRL  2
#define BEG_MCTRL  3

// Atari system calls
extern int appl_init(void);
extern int appl_exit(void);
extern int evnt_multi(int flags,int bclk,int bmsk,int bst,int m1flags,int m1x,int m1y,int m1w,int m1h,int m2flags,int m2x,int m2y,int m2w,int m2h,int *mepbuff,int tlc,int thc,int *pmx,int *pmy,int *pmb,int *pks,int *pkr,int *pbr);
extern int form_do(OBJECT *form,int start);
extern int form_dial(int flag,int littlx,int littly,int littlw,int littlh,int bigx,int bigy,int bigw,int bigh);
extern int form_alert(int defbut,const char *astring);
extern int form_center(OBJECT *tree,int *pcx,int *pcy,int *pcw,int *pch);
extern int fsel_input(char *pipath,char *pisel,int *pbutton);
extern int graf_handle(int *pwchar,int *phbox,int *phchar,int *pwbox);
extern int graf_mouse(int m_number,MFORM *m_addr);
extern int menu_bar(OBJECT *tree,int showit);
extern int menu_tnormal(OBJECT *tree,int titlenumm,int normalit);
extern int objc_draw(OBJECT *tree,int drawob,int depth,int xc,int yc,int wc,int hc);
extern int rsrc_obfix(OBJECT *tree,int obj);
extern int wind_create(int kind,int wx,int wy,int ww,int wh);
extern int wind_open(int handle,int wx,int wy,int ww,int wh);
extern int wind_close(int handle);
extern int wind_delete(int handle);
extern int wind_get(int w_handle,int w_field,int *pw1,int *pw2,int *pw3,int *pw4);
extern int wind_update(int beg_update);

/****** VDI *************************************************************/

/* bit blt rules */

#define ALL_WHITE        0
#define S_AND_D          1
#define S_AND_NOTD       2
#define S_ONLY           3
#define NOTS_AND_D       4
#define D_ONLY           5
#define S_XOR_D          6
#define S_OR_D           7
#define NOT_SORD         8
#define NOT_SXORD        9
#define D_INVERT        10
#define NOT_D           11
#define S_OR_NOTD       12
#define NOTS_OR_D       13
#define NOT_SANDD       14
#define ALL_BLACK       15

extern void v_clrwk(int handle);
extern void v_clsvwk(int handle);
extern void v_opnvwk(int *work_in,int *handle,int *work_out);
extern void vro_cpyfm(int handle,int wr_mode,int *pxyarray,MFDB *psrcMFDB,MFDB *pdesMFDB);


/****** Tos *************************************************************/

int     Fopen( const char *filename, int mode );
int     Fclose( int handle );
long    Fread( int handle, long count, void *buf );
int     Dgetdrv( void );
int     Dgetpath( char *path, int driveno );

/****** Bios ************************************************************/

/* Device Identifiers */

#define    PRT    0
#define    AUX    1
#define    CON    2
#define    MIDI    3
#define    IKBD    4

int     Bconstat( int dev );
long    Bconin( int dev );
void    Bconout( int dev, int c );

/****** XBios ***********************************************************/

typedef struct          /* used by Kbdvbase */
{
        void    *kb_midivec;
        void    *kb_vkbderr;
        void    *kb_vmiderr;
        void    *kb_statvec;
        void    *kb_mousevec;
        void    *kb_clockvec;
        void    *kb_joyvec;
        void    *kb_midisys;
        void    *kb_kbdsys;
} KBDVBASE;

void    *Physbase( void );
void    *Logbase( void );
int     Getrez( void );
void    Setscreen( void *laddr, void *paddr, int rez );
void    Setpalette( void *pallptr );
int     Setcolor( int colornum, int color );
void    Vsync( void );
char    Giaccess( char data, int regno );
void    *Dosound( void *buf );
long    Supexec( void (*func)(void) );
long    Random( void );
KBDVBASE *Kbdvbase( void );
void    Initmouse( int type, char *par, void (*mousevec)(void) );
