.section .text

_start:
    li a7, 60
    li a0, 42
    j skip
    li a0, 0
skip:
    ecall