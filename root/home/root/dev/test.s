.section .rodata
.globl hello
hello:
    .asciz "Hello World!\n"

.section .text
.globl output
output:
    addi a7, zero, 1
    ecall
    ret

.globl _start
_start:
    addi a0, zero, 1
    la a1, hello
    addi a2, zero, 13
    call output

    addi a0, zero, 0
    addi a7, zero, 60
    ecall