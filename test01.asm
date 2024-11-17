; NAME    : JUHI SAHNI
; ROLL No : 2301CS88
; This program stores the larger out of two variables in the main memory.
; Declaration of Authorship
; This file, test01.asm, is part of the miniproject of CS2102 at the
; Department of Computer Science and Engineering, IIT Patna.

main: ldc 4
	ldc 2
	sub
	brlz else
if:	
    ldc 4
    ldc 0
    stnl 0
    HALT
else:
    ldc 2
    ldc 0
    stnl 0
	HALT