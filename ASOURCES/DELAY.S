*// A short delay by maybe 100 cycles. Delays like this were typically used when
*// programming registers directly, but it is not used.
*// 

.globl	delay_unused
.text
delay_unused:
_delay_unused:
move.l    D0,-(A7)
moveq     #1,D0
L1:dbf    D0,L1
move.l    (A7)+,D0
rts
