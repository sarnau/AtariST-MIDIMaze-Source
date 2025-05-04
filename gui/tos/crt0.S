/*
 * Initialization code; this is common to both 16 and 32 bit libraries,
 * so be careful!
 *
 * No __FASTCALL__ modifications required.
 */

#define C_SYMBOL_NAME2(pref, name) pref##name
#define C_SYMBOL_NAME1(pref, name) C_SYMBOL_NAME2(pref, name)
#define C_SYMBOL_NAME(name) C_SYMBOL_NAME1(__USER_LABEL_PREFIX__, name)

/*
 * The following external symbols are declared with .globl rather than
 * .extern in order to ensure they will be linked into the executable
 * even if they are not used in this file.
 */

	.globl	C_SYMBOL_NAME(_base)		/* BASEPAGE *, defined in globals.c */

/* Functions defined elsewhere. */

	.extern C_SYMBOL_NAME(main)

	.text
	.even
#ifdef __ELF__
	.section	.text.startup,"ax",@progbits
#endif /* __ELF__ */
	.globl	C_SYMBOL_NAME(_start)
C_SYMBOL_NAME(_start):
	subl	%a6,%a6		/* clear a6 for debuggers */
/*
 * program startup code: doesn''t actually do much, other than push
 * the basepage onto the stack and call main
 */
__startprg:
	movel	4(%sp),%a5	/* get basepage */
	move.l	#ustk,%sp	/* set local stack */
	movel	%a5,C_SYMBOL_NAME(_base)	/* save it */

	move.l	12(%a5),%a0
	add.l	20(%a5),%a0
	add.l	28(%a5),%a0
	lea     256(%a0),%a0
	move.l	%a0,-(%sp)
	move.l	%a5,-(%sp)
	clr.w	-(%sp)	/* junk word */
	move	#74,-(%sp) /* Mshrink */
	trap	#1
	lea     12(%sp),%sp

	jsr	C_SYMBOL_NAME(main)

	.globl	C_SYMBOL_NAME(exit)	
C_SYMBOL_NAME(exit):
	clr.w -(%sp)
	trap #1 /* Pterm0 */
wait:
	bra.s wait

	.bss
C_SYMBOL_NAME(_base):
	.ds.l 1

	.bss
	.even
	.ds.l	4096
ustk:	.ds.l	1
	.ds.l 16		/* see note in mintlib/setstack.S */


#ifdef __ELF__
	.section .note.GNU-stack,"",@progbits
#ifdef __MSHORT__
#ifdef __FASTCALL__
	.gnu_attribute 8,7
#else
	.gnu_attribute 8,3
#endif
#else
#ifdef __FASTCALL__
	.gnu_attribute 8,5
#else
	.gnu_attribute 8,1
#endif
#endif
#endif
