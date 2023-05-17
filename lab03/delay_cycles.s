.global delay_cycles
.p2align 2
.type delay_cycles, %function

delay_cycles:
.fnstart

	LSRS r0, #2
	BEQ done
loop:
	SUBS r0, #1
	NOP
	BNE loop
done:
	BX lr
	
.fnend