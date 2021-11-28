.section .data
numbers:
.bytes "0123456789ABCDEF"
newln:
.bytes 10

.section .text
.align 16

_start:
    li s0, 0
    li s1, 1
    li s2, 1
    li s3, 1000

loop:
    add a0, s0, zero
    call print_num

    add s0, s1, zero
    add s1, s2, zero
    add s2, s0, s1

    blt s0, s3, loop

    li a0, 42
    call exit


print_digit:
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

print_num:
    push ra
    push s0
    push s1

    li a7, 10
    div a1, a0, a7
    
    li s0, 1

print_num_loop0:
    div a2, a1, s0
    beq a2, zero, print_num_loop1

    mul s0, s0, a7
    j print_num_loop0

print_num_loop1:
    div a1, a0, s0
    push a0
    push a7
    rem a0, a1, a7
    call print_digit
    pop a7
    pop a0

    div s0, s0, a7
    bne s0, zero, print_num_loop1
    
    call print_newline

    pop s1
    pop s0
    pop ra
    ret

exit:
    li a7, 60
    ecall