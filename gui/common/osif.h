/*
 * compiler dependant macros
 */

long __CDECL bios(short code, ...);
long __CDECL xbios(short code, ...);
long __CDECL gemdos(short code, ...);

#ifdef __GNUC__
#ifdef __m68k__
#define trap_1_w(n)							\
__extension__								\
({									\
	register long __retvalue __asm__("d0");				\
	    								\
	__asm__ volatile						\
	(								\
		"movw	%1,%%sp@-\n\t"					\
		"trap	#1\n\t"						\
		"addql	#2,%%sp\n\t"					\
	: "=r"(__retvalue)			/* outputs */		\
	: "g"(n)				/* inputs  */		\
	: __CLOBBER_RETURN("d0") "d1", "d2", "a0", "a1", "a2", "cc"    /* clobbered regs */	\
	  AND_MEMORY							\
	);								\
	__retvalue;							\
})

#define trap_1_ww(n, a)							\
__extension__								\
({									\
	register long __retvalue __asm__("d0");				\
	short _a = (short)(a);						\
	    								\
	__asm__ volatile						\
	(								\
		"movw	%2,%%sp@-\n\t"					\
		"movw	%1,%%sp@-\n\t"					\
		"trap	#1\n\t"						\
		"addql	#4,%%sp"						\
	: "=r"(__retvalue)			/* outputs */		\
	: "g"(n), "r"(_a)			/* inputs  */		\
	: __CLOBBER_RETURN("d0") "d1", "d2", "a0", "a1", "a2", "cc"    /* clobbered regs */	\
	  AND_MEMORY							\
	);								\
	__retvalue;							\
})

#define trap_1_wlw(n, a, b)						\
__extension__								\
({									\
	register long __retvalue __asm__("d0");				\
	long  _a = (long) (a);						\
	short _b = (short)(b);						\
	    								\
	__asm__ volatile						\
	(								\
		"movw	%3,%%sp@-\n\t"					\
		"movl	%2,%%sp@-\n\t"					\
		"movw	%1,%%sp@-\n\t"					\
		"trap	#1\n\t"						\
		"addql	#8,%%sp"						\
	: "=r"(__retvalue)			/* outputs */		\
	: "g"(n), "r"(_a), "r"(_b)		/* inputs  */		\
	: __CLOBBER_RETURN("d0") "d1", "d2", "a0", "a1", "a2", "cc"    /* clobbered regs */	\
	  AND_MEMORY							\
	);								\
	__retvalue;							\
})

#define trap_1_wwll(n, a, b, c)						\
__extension__								\
({									\
	register long __retvalue __asm__("d0");				\
	short _a = (short)(a);						\
	long  _b = (long) (b);						\
	long  _c = (long) (c);						\
	    								\
	__asm__ volatile						\
	(								\
		"movl	%4,%%sp@-\n\t"					\
		"movl	%3,%%sp@-\n\t"					\
		"movw	%2,%%sp@-\n\t"					\
		"movw	%1,%%sp@-\n\t"					\
		"trap	#1\n\t"						\
		"lea	%%sp@(12),%%sp"					\
	: "=r"(__retvalue)			/* outputs */		\
	: "g"(n), "r"(_a), "r"(_b), "r"(_c)     /* inputs  */		\
	: __CLOBBER_RETURN("d0") "d1", "d2", "a0", "a1", "a2", "cc"    /* clobbered regs */	\
	  AND_MEMORY							\
	);								\
	__retvalue;							\
})

#define trap_13_ww(n, a)						\
__extension__								\
({									\
	register long __retvalue __asm__("d0");				\
	short _a = (short)(a);						\
	    								\
	__asm__ volatile						\
	(								\
		"movw	%2,%%sp@-\n\t"					\
		"movw	%1,%%sp@-\n\t"					\
		"trap	#13\n\t"					\
		"addql	#4,%%sp"						\
	: "=r"(__retvalue)			/* outputs */		\
	: "g"(n), "r"(_a)			/* inputs  */		\
	: __CLOBBER_RETURN("d0") "d1", "d2", "a0", "a1", "a2", "cc"    /* clobbered regs */	\
	  AND_MEMORY							\
	);								\
	__retvalue;							\
})

#define trap_13_www(n, a, b)						\
__extension__								\
({									\
	register long __retvalue __asm__("d0");				\
	short _a = (short)(a);						\
	short _b = (short)(b);						\
	    								\
	__asm__ volatile						\
	(								\
		"movw	%3,%%sp@-\n\t"					\
		"movw	%2,%%sp@-\n\t"					\
		"movw	%1,%%sp@-\n\t"					\
		"trap	#13\n\t"					\
		"addql	#6,%%sp"						\
	: "=r"(__retvalue)			/* outputs */		\
	: "g"(n), "r"(_a), "r"(_b)		/* inputs  */		\
	: __CLOBBER_RETURN("d0") "d1", "d2", "a0", "a1", "a2", "cc"    /* clobbered regs */	\
	  AND_MEMORY							\
	);								\
	__retvalue;							\
})

#define trap_13_wwl(n, a, b)						\
__extension__								\
({									\
	register long __retvalue __asm__("d0");				\
	short _a = (short)(a);						\
	long  _b = (long) (b);						\
	    								\
	__asm__ volatile						\
	(								\
		"movl	%3,%%sp@-\n\t"					\
		"movw	%2,%%sp@-\n\t"					\
		"movw	%1,%%sp@-\n\t"					\
		"trap	#13\n\t"					\
		"addql	#8,%%sp"						\
	: "=r"(__retvalue)			/* outputs */		\
	: "g"(n), "r"(_a), "r"(_b)		/* inputs  */		\
	: __CLOBBER_RETURN("d0") "d1", "d2", "a0", "a1", "a2", "cc"    /* clobbered regs */	\
	  AND_MEMORY							\
	);								\
	__retvalue;							\
})

#define trap_14_w(n)							\
__extension__								\
({									\
	register long __retvalue __asm__("d0");				\
	    								\
	__asm__ volatile						\
	(								\
		"movw	%1,%%sp@-\n\t"					\
		"trap	#14\n\t"					\
		"addql	#2,%%sp"						\
	: "=r"(__retvalue)			/* outputs */		\
	: "g"(n)				/* inputs  */		\
	: __CLOBBER_RETURN("d0") "d1", "d2", "a0", "a1", "a2", "cc"    /* clobbered regs */	\
	  AND_MEMORY							\
	);								\
	__retvalue;							\
})

#define trap_14_ww(n, a)						\
__extension__								\
({									\
	register long __retvalue __asm__("d0");				\
	short _a = (short)(a);						\
	    								\
	__asm__ volatile						\
	(								\
		"movw	%2,%%sp@-\n\t"					\
		"movw	%1,%%sp@-\n\t"					\
		"trap	#14\n\t"					\
		"addql	#4,%%sp"						\
	: "=r"(__retvalue)			/* outputs */		\
	: "g"(n), "r"(_a)			/* inputs  */		\
	: __CLOBBER_RETURN("d0") "d1", "d2", "a0", "a1", "a2", "cc"    /* clobbered regs */	\
	  AND_MEMORY							\
	);								\
	__retvalue;							\
})

#define trap_14_wllw(n, a, b, c)					\
__extension__								\
({									\
	register long __retvalue __asm__("d0");				\
	long  _a = (long) (a);						\
	long  _b = (long) (b);						\
	short _c = (short)(c);						\
	    								\
	__asm__ volatile						\
	(								\
		"movw	%4,%%sp@-\n\t"					\
		"movl	%3,%%sp@-\n\t"					\
		"movl	%2,%%sp@-\n\t"					\
		"movw	%1,%%sp@-\n\t"					\
		"trap	#14\n\t"					\
		"lea	%%sp@(12),%%sp"					\
	: "=r"(__retvalue)			/* outputs */		\
	: "g"(n), "r"(_a), "r"(_b), "r"(_c)       /* inputs  */		\
	: __CLOBBER_RETURN("d0") "d1", "d2", "a0", "a1", "a2", "cc"    /* clobbered regs */	\
	  AND_MEMORY							\
	);								\
	__retvalue;							\
})

#define trap_14_wwll(n, a, b, c)					\
__extension__								\
({									\
	register long __retvalue __asm__("d0");				\
	short _a = (short)(a);						\
	long  _b = (long) (b);						\
	long  _c = (long) (c);						\
	    								\
	__asm__ volatile						\
	(								\
		"movl	%4,%%sp@-\n\t"					\
		"movl	%3,%%sp@-\n\t"					\
		"movw	%2,%%sp@-\n\t"					\
		"movw	%1,%%sp@-\n\t"					\
		"trap	#14\n\t"					\
		"lea	%%sp@(12),%%sp"					\
	: "=r"(__retvalue)			/* outputs */		\
	: "g"(n), "r"(_a), "r"(_b), "r"(_c)     /* inputs  */		\
	: __CLOBBER_RETURN("d0") "d1", "d2", "a0", "a1", "a2", "cc"    /* clobbered regs */	\
	  AND_MEMORY							\
	);								\
	__retvalue;							\
})

#define trap_14_wl(n, a)						\
__extension__								\
({									\
	register long __retvalue __asm__("d0");				\
	long  _a = (long) (a);						\
	    								\
	__asm__ volatile						\
	(								\
		"movl	%2,%%sp@-\n\t"					\
		"movw	%1,%%sp@-\n\t"					\
		"trap	#14\n\t"					\
		"addql	#6,%%sp"						\
	: "=r"(__retvalue)			/* outputs */		\
	: "g"(n), "r"(_a)			/* inputs  */		\
	: __CLOBBER_RETURN("d0") "d1", "d2", "a0", "a1", "a2", "cc"    /* clobbered regs */	\
	  AND_MEMORY							\
	);								\
	__retvalue;							\
})

#define trap_14_www(n, a, b)						\
__extension__								\
({									\
	register long __retvalue __asm__("d0");				\
	short _a = (short)(a);						\
	short _b = (short)(b);						\
	    								\
	__asm__ volatile						\
	(								\
		"movw	%3,%%sp@-\n\t"					\
		"movw	%2,%%sp@-\n\t"					\
		"movw	%1,%%sp@-\n\t"					\
		"trap	#14\n\t"					\
		"addql	#6,%%sp"						\
	: "=r"(__retvalue)			/* outputs */		\
	: "g"(n), "r"(_a), "r"(_b)		/* inputs  */		\
	: __CLOBBER_RETURN("d0") "d1", "d2", "a0", "a1", "a2", "cc"    /* clobbered regs */	\
	  AND_MEMORY							\
	);								\
	__retvalue;							\
})

#endif
#endif


#ifndef trap_1_wwll
#define trap_1_w gemdos
#define trap_1_wlw gemdos
#define trap_1_ww gemdos
#define trap_1_wwll gemdos
#define trap_13_ww bios
#define trap_13_www bios
#define trap_13_wwl bios
#define trap_14_w xbios
#define trap_14_ww xbios
#define trap_14_wllw xbios
#define trap_14_wwll xbios
#define trap_14_wl xbios
#define trap_14_www xbios
#endif


#ifdef __PUREC__
/*
 * inlining gemdos should be safe, because gemdos entry point in ROM saves all registers
 * But not so for bios/xbios, which will trash a2
 */
#if 0
static long cdecl gemdos(short code, ...)  0x4e41; /* trap #1 */
#endif
#endif

/****** Tos *************************************************************/

#ifdef __m68k__
#define	Fopen(fn, mode)	trap_1_wlw(0x3D, (const void *)(fn), (short)(mode))
#define	Fclose(handle)	trap_1_ww(0x3E, (short)(handle))
#define	Fread(handle, count, buffer) trap_1_wwll(0x3f, (short)(handle), (long)(count), (void *)(buffer))
#define	Dgetdrv() (short)trap_1_w(0x19)
#define	Dgetpath(path, drv)	trap_1_wlw(0x47, (void *)(path), (short)(drv))
#else
int Fopen(const char *filename, int mode);
int Fclose(int handle);
long Fread(int handle, long count, void *buf);
int Dgetdrv(void);
int Dgetpath(char *path, int driveno);
#endif

/****** Bios ************************************************************/

/* Device Identifiers */

#define    PRT	  0
#define    AUX	  1
#define    CON	  2
#define    MIDI    3
#define    IKBD    4

#ifdef __m68k__
#define	Bconstat(dev) trap_13_ww(1, (short)(dev))
#define	Bconin(dev)	trap_13_ww(2, (short)(dev))
#define	Bconout(dev, ch) trap_13_www(3, (short)(dev), (short)(ch))
#define	Setexc(vnum,vptr) (void *)trap_13_wwl(5, (short)(vnum), (long)(vptr))
#else
int Bconstat(int dev);
long Bconin(int dev);
void Bconout(int dev, int c);
#endif

/****** XBios ***********************************************************/

typedef struct			/* used by Kbdvbase */
{
		void *kb_midivec;
		void *kb_vkbderr;
		void *kb_vmiderr;
		void *kb_statvec;
		void *kb_mousevec;
		void *kb_clockvec;
		void *kb_joyvec;
		void *kb_midisys;
		void *kb_kbdsys;
} KBDVBASE;

/* Structure returned by Iorec() */
typedef struct {
    char    *ibuf;
    short   ibufsiz;
    volatile short ibufhd;
    volatile short ibuftl;
    short   ibuflow;
    short   ibufhi;
} IOREC;

#ifdef __m68k__
#define Initmouse(type, par, mousevec) (void)trap_14_wwll(0, (short)(type), (const void *)(par), (void *)(mousevec))
#define	Physbase()	((void *)trap_14_w(2))
#define	Logbase()	((void *)trap_14_w(3))
#define	Getrez()	(short)trap_14_w(4)
#define	Setscreen(lscrn, pscrn, rez) (void)trap_14_wllw(5, (void *)(lscrn), (void *)(pscrn), (short)(rez))
#define Setpalette(palptr)	(void)trap_14_wl(6, (void *)(palptr))
#define Setcolor(num, color)	(short)trap_14_www(7, (short)(num), (short)(color))
#define	Iorec(dev) (IOREC *)trap_14_ww(14, (short)(dev))
#define Vsync()		(void)trap_14_w(0x25)
#define	Giaccess(data, reg)	(short)trap_14_www(28, (short)(data), (short)(reg))
#define	Dosound(ptr)	trap_14_wl(32, (void *)(ptr))
#define Supexec(fn)	trap_14_wl(0x26, (void *)(fn))
#define	Random()	trap_14_w(17)
#define	Kbdvbase()	((KBDVBASE *)trap_14_w(34))
#else
void Initmouse(int type, const char *par, void (*mousevec)(void));
void *Physbase(void);
void *Logbase(void);
int Getrez(void);
void Setscreen(void *laddr, void *paddr, int rez);
void Setpalette(void *palptr);
int Setcolor(int colornum, int color);
void Vsync(void);
char Giaccess(char data, int regno);
void *Dosound(void *buf);
long Supexec(long (*func)(void));
long Random(void);
KBDVBASE *Kbdvbase(void);
#endif
