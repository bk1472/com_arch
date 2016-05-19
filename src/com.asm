;#######################################################
;# com.asm
;#------------------------------------------------------
;# by bk14722lge.com
;# data memory 이동 프로그램
;#######################################################
		.text
_start:
	li		r7, #my_data      ; R7 <- addr(my_data)
	li		r1, #2            ; R1 <- 2
	add		r6, r7, r1        ; R6 <- R7 + R1
	ld		r4, [r7]          ; R4 <- addr(R7)
	add		r4, r4, r4        ; R4 <- R4 + R4
	st		r4, [r6]          ; addr(R6) <- R4
	ext                       ; Exit

		.data
my_data:                      ; Data section
	.word 0x1234              ; Read data
	.word 0x0000              ; Write Date
