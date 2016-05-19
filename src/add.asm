;#######################################################
;# add.asm
;#------------------------------------------------------
;# by bk14722lge.com
;# 1부터 10까지 합산하는 프로그램
;#######################################################
		.text
_start:
    li      r0, #10              ; R0 <- 10
    li      r1, #0               ; R1 <- 0
    li      r2, #seed            ; R2 <- addr(seed)
    ld      r2, [r2]             ; R2 <- addr(R2)
.loop:                           ;
    add     r1, r1, r2           ; R1 <- R1 + R2
    inc     r2                   ; R2++
    dec     r0                   ; R0--
    brz     r0, .cdone           ; if(R0 == 0) jump .cdone
    jmp     .loop                ; unconditional jump to .loop

		.data
seed:                            ; add seed value
    .word 0x0001                 ;
data:                            ; Data memory
    .word 0x0000                 ;	

		.text
.cdone:                          ;
    li      r7, #data            ; R7 <- addr(data)
    st      r1, [r7]             ; addr(R7) <- R1
    ext                          ; Exit
