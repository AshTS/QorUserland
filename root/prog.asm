.section .text

_start:
    addi a7, zero, 60
    addi a0, zero, 42
    jal zero, skip
    addi a0, zero, 0
skip:
    ecall