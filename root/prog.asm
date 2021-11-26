.section .data
numbers:
.bytes "0123456789ABCDEF"
newln:
.bytes 10

.section .text
.align 16

_start:
    li s0, 0

loop:
    add a0, s0, zero

    call print_num

    addi s0, s0, 1
    li s1, 10
    blt s0, s1, loop

    call print_newline

    li a0, 42
    call exit


print_num:
    la a1, numbers
    add a1, a1, a0
    li a7, 1
    li a0, 1
    li a2, 1
    ecall
    ret

print_newline:
    la a1, newln
    li a7, 1
    li a0, 1
    li a2, 1
    ecall
    ret

exit:
    li a7, 60
    ecall