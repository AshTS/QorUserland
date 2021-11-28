#ifndef _RISCV_H
#define _RISCV_H

#include <libc/stdbool.h>
#include <libc/stdint.h>

#include "linking.h"

enum RISCVInstruction
{
    LUI,
    JAL,
    JALR,
    BEQ,
    BNE,
    BLT,
    BGE,
    BLTU,
    BGEU,
    LB,
    LH,
    LW,
    LD,
    LBU,
    LHU,
    LWU,
    SB,
    SH,
    SW,
    SD,
    ADDI,
    SLTI,
    SLTIU,
    XORI,
    ORI,
    ANDI,
    SLLI,
    SRLI,
    SRAI,
    ADDIW,
    SLLIW,
    SRLIW,
    SRAIW,
    ADDW,
    SUBW,
    SLLW,
    SRLW,
    SRAW,
    ADD,
    SUB,
    SLL,
    SLT,
    SLTU,
    XOR,
    SRL,
    SRA,
    OR,
    AND,
    ECALL,
};

struct Instruction
{
    enum RISCVInstruction instruction;
    uint8_t rdest;
    uint8_t rs1;
    uint8_t rs2;

    uint64_t imm;

    enum LinkingType link_type;

    char* link;
};

char* render_instruction(struct Instruction*);

uint32_t compile_instruction(struct Instruction*);

#endif // _RISCV_H