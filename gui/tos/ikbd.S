/*
 *	IKBD 6301 interrupt routine
 */

	.xref own_mousevec
	.xref own_joyvec
	.xref kbdiorec
	.xref kbdvec
	.xref savedKbdVectors

ACIA_IKBD_BASE = 0xfffffc00
ACIA_IKBD_DATA = ACIA_IKBD_BASE+2

	.text

	.globl ikbd_install
ikbd_install:
	lea     -20(sp),sp
	movem.l d1-d2/a0-a2,(sp)
	lea     aciaread(pc),a0
	move.l  a0,curhandler
	pea     ikbd(pc)
	move.w  #0x118/4,-(sp)
	move.w  #5,-(sp) /* Setexc */
	trap #  13
	addq.l  #8,sp
	move.l  d0,old_ikbd
	movem.l (sp),d1-d2/a0-a2
	lea     20(sp),sp
	rts

	.globl ikbd_uninstall
ikbd_uninstall:
	lea     -20(sp),sp
	movem.l d1-d2/a0-a2,(sp)
	/*
	 * wait until current packet has been completely received
	 */
ikbd_uninstall1:
	move.l  curhandler,d0
	cmp.l   #aciaread,d0
	bne.s   ikbd_uninstall1
	move.l  old_ikbd(pc),-(sp)
	move.w  #0x118/4,-(sp)
	move.w  #5,-(sp) /* Setexc */
	trap #  13
	addq.l  #8,sp
	movem.l (sp),d1-d2/a0-a2
	lea     20(sp),sp
	rts

/*--- Our custom IKBD vector ---*/

xbra_ikbd:
	.dc.l   0x58425241 /* 'XBRA' */
	.dc.l	0x4d415a45 /* 'MAZE' */
old_ikbd:
	.dc.l	0
ikbd:
	lea		-20(sp),sp
	movem.l	d0-d2/a0-a1,(sp)

	/* Check if source is IKBD or MIDI */
intacia:
	moveq	#0,d0
	btst	d0,ACIA_IKBD_BASE.w
	beq.s	ikbd_oldmidi

	move.b	ACIA_IKBD_DATA.w,d0
	move.l  curhandler(pc),a0
	jsr     (a0)
	
	lea     0xfffffa01.w,a1
	btst    #4,(a1)          /* ACIA receiver buffer empty? */
	beq.s	intacia			 /* no, read more bytes */

	/* End of interrupt */
	move.b  #0xbf,16(a1)     /* clear in-service bit */
	movem.l	(sp),d0-d2/a0-a1
	lea		20(sp),sp
	rte

	/* Call old ACIA interrupt */

ikbd_oldmidi:
	movem.l	(sp),d0-d2/a0-a1
	lea		20(sp),sp
	move.l	old_ikbd(pc),-(sp)
	rts

	/* Joystick packet ? */

aciaread:
	cmp.b	#0xfd,d0
	bcc		ikbd_yes_joystick

	/* Mouse packet ? */

	cmp.b	#0xf8,d0
	blo.s	ikbd_no_mouse
	cmp.b	#0xfc,d0
	bhs.s	ikbd_no_mouse

	/* Mouse packet, byte #1 */

ikbd_yes_mouse:
	move.b	d0,mouserec_buf+0
	lea		ikbd_mousex(pc),a0
	move.l	a0,curhandler
	rts

	/* Mouse packet, byte #2 */

ikbd_mousex:
	move.b  d0,mouserec_buf+1
	lea		ikbd_mousey(pc),a0
	move.l	a0,curhandler
	rts

	/* Mouse packet, byte #3 */

ikbd_mousey:
	lea     mouserec_buf,a0
	move.b  d0,2(a0)
	/* call our game handler */
	move.l  a0,-(sp)
	jsr     own_mousevec
	addq.l	#4,sp

	/* and also call bios handler */
	lea     mouserec_buf,a0
	move.l  savedKbdVectors+16,a1
	move.l  a0,-(sp)
	jsr     (a1)
	addq.l	#4,sp
	
	lea		aciaread(pc),a0
	move.l	a0,curhandler
	rts

	/* Keyboard press/release */

ikbd_no_mouse:
	moveq	#0x7f,d1
	and.l	d0,d1
	lea		keyboard_state_buffer,a0
	move.b	d0,0(a0,d1.l)
	
	/*
	 * call also TOS kbdvec handler
	 * It must be called with
	 * - d0.b = received byte
	 * - a0.l = pointer to keyboard IOREC
	 * For old tos versions, a5 must be zero
	 */
	move.l a5,-(sp)
	suba.l a5,a5
	move.l kbdiorec,a0
	move.l kbdvec,a1
	jsr (a1)
	move.l (sp)+,a5
	rts

	/* Joystick packet, byte #1 */

ikbd_yes_joystick:
	move.b  d0,joyrec_buf+0
	cmp.b   #0xfe,d0
	beq     ikbd_joystick_fe
	cmp.b   #0xff,d0
	beq     ikbd_joystick_ff
	lea		ikbd_joystick(pc),a0
	move.l	a0,curhandler
	rts

	/* Joystick packet, byte #2 */

ikbd_joystick:
/*
 * For 0xfd packets, original TOS stores the 2 bytes at the start of the buffer,
 * overwriting the packet header. We do the same.
 */
	move.b  d0,joyrec_buf+0
	lea		ikbd_joystick2(pc),a0
	move.l	a0,curhandler
	rts

	/* Joystick packet, byte #3 */

ikbd_joystick2:
	lea     joyrec_buf,a0
	move.b	d0,1(a0)

call_joyhandler:
	/* call our handler */
	move.l  a0,-(sp)
	jsr     own_joyvec
	addq.l	#4,sp

	/* and also call bios handler */
	lea     joyrec_buf,a0
	move.l  savedKbdVectors+24,a1
	move.l  a0,-(sp)
	jsr     (a1)
	addq.l	#4,sp
	
	lea		aciaread(pc),a0
	move.l	a0,curhandler
	rts

ikbd_joystick_fe:
	lea		ikbd_joystick_got_fe(pc),a0
	move.l	a0,curhandler
	rts

ikbd_joystick_got_fe:
	lea     joyrec_buf,a0
	move.b  d0,1(a0)
	bra.s   call_joyhandler

ikbd_joystick_ff:
	lea		ikbd_joystick_got_ff(pc),a0
	move.l	a0,curhandler
	rts

ikbd_joystick_got_ff:
	lea     joyrec_buf,a0
	move.b  d0,2(a0)
	bra.s   call_joyhandler

curhandler:
	.dc.l	aciaread

	.bss

	.even
	.globl keyboard_state_buffer
keyboard_state_buffer:
	.ds.b	128

mouserec_buf: .ds.b 3
joyrec_buf:   .ds.b 3
