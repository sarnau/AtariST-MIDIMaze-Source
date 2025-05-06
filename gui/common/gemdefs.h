#ifndef __INTPTR_TYPE__
#define __INTPTR_TYPE__ long
#endif
typedef __INTPTR_TYPE__ intptr_t;

typedef struct
{
	short *cb_pcontrol;
	short *cb_pglobal;
	short *cb_pintin;
	short *cb_pintout;
	void **cb_padrin;
	void **cb_padrout;		
} AESPB;

typedef struct
{
	short mf_xhot; /* 0 */
	short mf_yhot; /* 2 */
	short mf_nplanes; /* 4 */
	short mf_fg; /* 6 */
	short mf_bg; /* 8 */
	short mf_mask[16]; /* 10 */
	short mf_data[16]; /* 42 */
} MFORM; /* 74 */

typedef struct
{
	void *fd_addr; /* 0 */
	short fd_w; /* 4 */
	short fd_h; /* 6 */
	short fd_wdwidth; /* 8 */
	short fd_stand; /* 10 */
	short fd_nplanes; /* 12 */
	short fd_rl; /* 14 */
	short fd_r2; /* 16 */
	short fd_r3; /* 18 */
} MFDB; /* 20 */

typedef struct
{
	short ob_next; /* 0 */
	short ob_head; /* 2 */
	short ob_tail; /* 4 */
	short ob_type; /* 6 */
	short ob_flags; /* 8 */
	short ob_state; /* 10 */
	intptr_t ob_spec; /* 12 */
	short ob_x; /* 16 */
	short ob_y; /* 18 */
	short ob_width; /* 20 */
	short ob_height; /* 22 */
} OBJECT; /* 24 */

typedef struct
{
	char	*te_ptext;		  /* 0 */
	char	*te_ptmplt; 	  /* 4 */
	char	*te_pvalid; 	  /* 8 */
	short    te_font; 		  /* 12 */
	short    te_fontid;		  /* 14 */
	short    te_just; 		  /* 16 */
	short    te_color;		  /* 18 */
	short    te_fontsize; 	  /* 20 */
	short    te_thickness;	  /* 22 */
	short    te_txtlen;		  /* 24 */
	short    te_tmplen;		  /* 26 */
} TEDINFO; /* 28 */

/** Icon block structure */
typedef struct icon_block
{
	short		*ib_pmask;			/**< Ptr to the icon mask*/
	short		*ib_pdata;			/**< Ptr to the icon image */
	char		*ib_ptext;			/**< Ptr to the icon text */
	short		ib_char;			/**< Icon character, foreground and background color \n Bits : \n 15-12 Icon foreground color\n 11-8 Icon Background color \n 7-0 ASCCI value of the Character to be displayed*/
	short		ib_xchar;			/**< X coordinate of the character */
	short		ib_ychar;			/**< Y coordinate of the character */
	short		ib_xicon;			/**< X coordinate of the icon */
	short		ib_yicon;			/**< Y coordinate of the icon */
	short		ib_wicon;			/**< Width of the icon */
	short		ib_hicon;			/**< Height of the icon */
	short		ib_xtext;			/**< X coordinate of the text (relative to the position of the icon) */
	short		ib_ytext;			/**< Y coordinate of the text (relative to the position of the icon)*/
	short		ib_wtext;			/**< Width in pixels of the text */
	short		ib_htext;			/**< Height in pixels of the text */
} ICONBLK;

/** Bit block */
typedef struct bit_block
{
	short		*bi_pdata;	/**< ptr to bit forms data  */
	short		bi_wb;		/**< width of form in bytes */
	short		bi_hl;		/**< height in lines */
	short		bi_x; 		/**< source x in bit form */
	short		bi_y; 		/**< source y in bit form */
	short		bi_color;	/**< fg color of blt */
} BITBLK;

/* Mouse forms */

#define ARROW			  0
#define TEXT_CRSR		  1
#define HOURGLASS		  2
#define BUSYBEE 		  2
#define POINT_HAND		  3
#define FLAT_HAND		  4
#define THIN_CROSS		  5
#define THICK_CROSS 	  6
#define OUTLN_CROSS 	  7
#define USER_DEF		255
#define M_OFF			256
#define M_ON			257

/****** Event definitions ***********************************************/

#define MU_KEYBD		0x0001
#define MU_BUTTON		0x0002
#define MU_M1			0x0004
#define MU_M2			0x0008
#define MU_MESAG		0x0010
#define MU_TIMER		0x0020

#define MN_SELECTED 	10
#define WM_REDRAW		20
#define WM_TOPPED		21
#define WM_CLOSED		22
#define WM_FULLED		23
#define WM_ARROWED		24
#define WM_HSLID		25
#define WM_VSLID		26
#define WM_SIZED		27
#define WM_MOVED		28
#define WM_NEWTOP		29
#define WM_UNTOPPED 	30
#define WM_ONTOP		31
#define AC_OPEN 		40
#define AC_CLOSE		41
#define CT_UPDATE		50
#define CT_MOVE 		51
#define CT_NEWTOP		52
#define AP_TERM 		50
#define AP_TFAIL		51
#define AP_RESCHG		57
#define SHUT_COMPLETED	60
#define RESCHG_COMPLETED  61
#define AP_DRAGDROP 	63
#define CH_EXIT 		90



/* Keyboard states */

#define K_RSHIFT		0x0001
#define K_LSHIFT		0x0002
#define K_CTRL			0x0004
#define K_ALT			0x0008

/****** Object definitions **********************************************/

#define G_BOX			20
#define G_TEXT			21
#define G_BOXTEXT		22
#define G_IMAGE 		23
#define G_USERDEF		24
#define G_IBOX			25
#define G_BUTTON		26
#define G_BOXCHAR		27
#define G_STRING		28
#define G_FTEXT 		29
#define G_FBOXTEXT		30
#define G_ICON			31
#define G_TITLE 		32
#define G_CICON 		33


/* Object flags */

#define NONE			0x0000
#define SELECTABLE		0x0001
#define DEFAULT 		0x0002
#define EXIT			0x0004
#define EDITABLE		0x0008
#define RBUTTON 		0x0010
#define LASTOB			0x0020
#define TOUCHEXIT		0x0040
#define HIDETREE		0x0080
#define INDIRECT		0x0100


/* Object states */

#define NORMAL			0x00
#define SELECTED		0x01
#define CROSSED 		0x02
#define CHECKED 		0x04
#define DISABLED		0x08
#define OUTLINED		0x10
#define SHADOWED		0x20

#define FMD_START		0
#define FMD_GROW		1
#define FMD_SHRINK		2
#define FMD_FINISH		3

/****** Window definitions **********************************************/

#define NAME	0x0001
#define CLOSER	0x0002
#define FULLER	0x0004
#define MOVER	0x0008
#define INFO	0x0010
#define SIZER	0x0020
#define UPARROW 0x0040
#define DNARROW 0x0080
#define VSLIDE	0x0100
#define LFARROW 0x0200
#define RTARROW 0x0400
#define HSLIDE	0x0800

#define WF_KIND 		 1
#define WF_NAME 		 2
#define WF_INFO 		 3
#define WF_WORKXYWH 	 4
#define WF_CURRXYWH 	 5
#define WF_PREVXYWH 	 6
#define WF_FULLXYWH 	 7
#define WF_HSLIDE		 8
#define WF_VSLIDE		 9
#define WF_TOP			10
#define WF_FIRSTXYWH	11
#define WF_NEXTXYWH 	12
#define WF_RESVD		13
#define WF_NEWDESK		14
#define WF_HSLSIZE		15
#define WF_VSLSIZE		16
#define WF_SCREEN		17
#define WF_COLOR		18
#define WF_DCOLOR		19
#define WF_OWNER		20
#define WF_BEVENT		24
#define WF_BOTTOM		25

#define W_BOX			 0
#define W_TITLE 		 1
#define W_CLOSER		 2
#define W_NAME			 3
#define W_FULLER		 4
#define W_INFO			 5
#define W_DATA			 6
#define W_WORK			 7
#define W_SIZER 		 8
#define W_VBAR			 9
#define W_UPARROW		10
#define W_DNARROW		11
#define W_VSLIDE		12
#define W_VELEV 		13
#define W_HBAR			14
#define W_LFARROW		15
#define W_RTARROW		16
#define W_HSLIDE		17
#define W_HELEV 		18

#define WA_UPPAGE 0
#define WA_DNPAGE 1
#define WA_UPLINE 2
#define WA_DNLINE 3
#define WA_LFPAGE 4
#define WA_RTPAGE 5
#define WA_LFLINE 6
#define WA_RTLINE 7

#define WC_BORDER 0 					/* wind calc flags */
#define WC_WORK   1

#define END_UPDATE 0					/* update flags */
#define BEG_UPDATE 1
#define END_MCTRL  2
#define BEG_MCTRL  3

/* Atari system calls */
extern short gl_apid;
void crystal(AESPB *pb);



short appl_init(void);
short appl_exit(void);
short evnt_multi(short flags, short bclk, short bmsk, short bst,
	short m1flags, short m1x, short m1y, short m1w, short m1h,
	short m2flags, short m2x, short m2y, short m2w, short m2h,
	short *mepbuff,
	short tlc, short thc,
	short *pmx, short *pmy, short *pmb, short *pks, short *pkr, short *pbr);
short form_do(OBJECT *form, short start);
short form_dial(short dtype, short ix, short iy, short iw, short ih, short x, short y, short w, short h);
short form_alert(short defbut, const char *astring);
short form_center(OBJECT *tree, short *pcx, short *pcy, short *pcw, short *pch);
short fsel_input(char *pipath, char *pisel, short *pbutton);
short graf_handle(short *pwchar, short *phchar, short *pwbox, short *phbox);
short graf_mouse(short m_number, const MFORM *m_addr);
short menu_bar(OBJECT *tree, short showit);
short menu_tnormal(OBJECT *tree, short titlenum, short normalit);
short objc_draw(OBJECT *tree, short drawob, short depth, short xc, short yc, short wc, short hc);
short rsrc_obfix(OBJECT *tree, short obj);
short wind_create(short kind, short wx, short wy, short ww, short wh);
short wind_open(short handle, short wx, short wy, short ww, short wh);
short wind_close(short handle);
short wind_delete(short handle);
short wind_get(short w_handle, short w_field, short *pw1, short *pw2, short *pw3, short *pw4);
short wind_update(short beg_update);

/****** VDI *************************************************************/

/* bit blt rules */

#define ALL_WHITE		 0
#define S_AND_D 		 1
#define S_AND_NOTD		 2
#define S_ONLY			 3
#define NOTS_AND_D		 4
#define D_ONLY			 5
#define S_XOR_D 		 6
#define S_OR_D			 7
#define NOT_SORD		 8
#define NOT_SXORD		 9
#define D_INVERT		10
#define NOT_D			11
#define S_OR_NOTD		12
#define NOTS_OR_D		13
#define NOT_SANDD		14
#define ALL_BLACK		15

#ifndef __VDIPB
#define __VDIPB
typedef struct
{
	short       *control;
	const short *intin;
	const short *ptsin;
	short       *intout;
	short       *ptsout;
} VDIPB;
#endif

void vdi(void);

void v_clrwk(short handle);
void v_clsvwk(short handle);
void v_opnvwk(short *work_in, short *handle, short *work_out);
void vro_cpyfm(short handle, short wr_mode, short *xy, MFDB *srcMFDB, MFDB *desMFDB);
