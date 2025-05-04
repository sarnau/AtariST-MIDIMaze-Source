*	APSTART.S	4/30/85			MIKE SCHMAL
*
*	APPLICATION STARTUP PROGRAM
*
* This version only contains what is necessary for a application to run
* All other garbage has been removed as not to waste memory.
*
* Sample application startup code. This must be the first object file in
*  the link statement so that the base page address can be accessed.
*  When a program is executed, GEMDOS gives all available memory to it.
*  If this program needs to do any memory management, then one must first
*  free unused memory by a setblock call. All "segment" lengths in the base
*  page are totaled and 0x100 is added for the base page length for the setblock
*  call.
*

#ifndef __GNUC__

	.text
	.globl	_main
	.globl	_crystal
*
*  Must be first object file in link statement
*
	move.l	a7,a5		* save a7 so we can get the base page address
	move.l	#ustk,a7	* set local stack
	move.l	4(a5),a5	* basepage address
	move.l	$c(a5),d0
	add.l	$14(a5),d0
	add.l	$1c(a5),d0
	add.l	#$100,d0	* skip los pageos baseos
	move.l	d0,-(sp)
	move.l	a5,-(sp)
	move	d0,-(sp)	* junk word
	move	#$4a,-(sp)
	trap	#1
	add.l	#12,sp
*
	jsr	_main		* go to program
	move.l	#0,-(a7)	* back to gemdos
	trap	#1
*
*	For GEMAES calls from AESBIND.ARC or cryslib.o
*
_crystal:
	move.l	4(a7),d1
	move.w	#200,d0
	trap	#2
	rts
*
*
	.bss
	.even
	.ds.l	256
ustk:	.ds.l	1
*
#endif
