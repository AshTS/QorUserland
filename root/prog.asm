.section .data
numbers:
.bytes "0123456789ABCDEF"
newln:
.bytes 10

.section .text
.align 16

_start:
    li a0, 5
    call print_num
exit:
    li a7, 60
    li a0, 0
    ecall

print_num:
    la a1, numbers
    push ra
    add a1, a1, z0
    li a7, 1
    li a0, 1
    li a2, 1
    ecall
    la a1, newln
    li a7, 1
    li a0, 1
    li a2, 1
    ecall
    la ra, _start
    pop ra
    ret