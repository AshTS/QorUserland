#ifndef _RISCV_H
#define _RISCV_H

#include <libc/stdbool.h>
#include <libc/stdint.h>

enum RISCVInstruction
{
    JAL,
    ADDI,
    ECALL
};

struct Instruction
{
    enum RISCVInstruction instruction;
    uint8_t rdest;
    uint8_t rs1;
    uint8_t rs2;

    uint64_t imm;

    bool j_link;
    bool b_link;

    char* link;
};

char* render_instruction(struct Instruction*);

uint32_t compile_instruction(struct Instruction*);

#endif // _RISCV_H