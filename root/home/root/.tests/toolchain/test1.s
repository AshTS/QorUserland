.section .text


.globl _start
_start:
    call value
    
    li a7, 60
    ecall

.globl value
value:
    li a0, 32
    ret