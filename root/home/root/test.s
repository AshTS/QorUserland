.section .section
hello:
    .asciz "Hello World"

.globl puts

.section .text
.globl main
main    :
    addi sp, sp, -8
    sd ra, 0(sp)

    la a0, hello
    call puts

    la a0, hello
    call puts

    ld ra, 0(sp)
    addi sp, sp, 8

    ret
