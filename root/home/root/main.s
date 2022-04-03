.globl value

.section .text
.globl _start
_start:
    addi sp, sp, -8
    sd ra, 0(sp)

    call value

    ld ra, 0(sp)
    addi sp, sp, 8

    addi a7, zero, 60
    ecall