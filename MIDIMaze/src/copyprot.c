#include "globals.h"

/*
; The protection of Midi Maze is quite simple: Track 0, Sector 0 on Side 0 is read twice and
; while the first 6 bytes have to contain '100004', the rest of the sector has to change between
; these two reads.

check_copy_protection:
#if DISABLE_COPY_PROTECTION
      ST        D0          ;This is how the copy protection was disabled in the public domain version of MIDImaze
      RTS
#else
      MOVEM.L   A0-A6/D1-D7,-(A7)
#endif
      MOVEQ     #2,D6       ;test drive A and B
      MOVEQ     #2,D5       ;read the sector twice

      MOVE.W    #$19,-(A7) 	;DGETDRV
      TRAP      #1
      ADDQ.W    #2,A7
      MOVE.W    D0,D7
      CMPI.W    #2,D7
      BLT.S     check_copy_protection_rd_loop
      CLR.W     D7          ;if loaded from harddisk, search drive A

check_copy_protection_rd_loop:
      MOVE.W    #1,-(A7)    ;1 sector
      CLR.W     -(A7)       ;Side 0
      CLR.W     -(A7)       ;Track 0
      CLR.W     -(A7)       ;Sector 0
      MOVE.W    D7,-(A7)    ;drive
      CLR.L     -(A7)       ;reserved
      PEA       check_copy_protection_buffer
      MOVE.W    #8,-(A7) 	;FLOPRD
      TRAP      #14
      ADDA.W    #20,A7
      TST.W     D0			;E_OK
      BEQ.S     check_copy_protection_verify
      CMP.W     #-4,D0		;E_CRC
      BEQ.S     check_copy_protection_verify

check_copy_protection_rd_retry:
      SUBQ.W    #1,D6		;out of tries?
      BEQ.S     check_copy_protection_fail	;=> protection fail
      MOVEQ     #2,D5       ;read the sector twice (again)

	  ; try the potentially other floppy drive
      ADDQ.W    #1,D7       ;try reading drive B
      CMPI.W    #2,D7		;reached C?
      BNE.S     check_copy_protection_rd_loop
      CLR.W     D7          ;then try drive A
      BRA.S     check_copy_protection_rd_loop

check_copy_protection_fail:
      CLR.W     D0          ;fail
      BRA.S     check_copy_protection_return

      DC.B      'RMP  V1.00  31-July-86'

check_copy_protection_verify:
      CLR.W     D0

      ; first: compare the first 6 bytes for identity
      LEA       check_copy_protection_buffer,A0
      LEA       check_copy_protection_magic,A1
      CMPM.L    (A0)+,(A1)+
      BNE.S     check_copy_protection_rd_retry
      CMPM.W    (A0)+,(A1)+
      BNE.S     check_copy_protection_rd_retry

      ; count the number of set bits in the remaining 506 bytes
      MOVE.W    #505,D1
check_copy_protection_byteloop:
      MOVEQ     #7,D2
      MOVE.B    (A0)+,D3
check_copy_protection_bitloop:
      ASR.W     #1,D3
      BCC.S     check_copy_protection_bitloop2
      ADDQ.W    #1,D0
check_copy_protection_bitloop2:
      DBF       D2,check_copy_protection_bitloop
      DBF       D1,check_copy_protection_byteloop

      SUBQ.W    #1,D5       ;already read twice?
      BEQ.S     check_copy_protection_check     ;yes! => compare the bitcounter

      MOVE.W    D0,D4       ; save the number of bits
      BRA       check_copy_protection_rd_loop	;read sector again

check_copy_protection_check:
      SUB.W     D4,D0       ;same number of bits set?
      BEQ.S     check_copy_protection_rd_retry ;=> that is a fail!

      ST        D0          ;bitcounter change => success
check_copy_protection_return:
      MOVEM.L   (A7)+,A0-A6/D1-D7
      RTS


check_copy_protection_buffer:
      DCB.B     512,0

      ALIGN 4
check_copy_protection_magic:
      DC.B      '100004'
*/

int   check_copy_protection(void)
{
    return -1; // protection is valid
}
