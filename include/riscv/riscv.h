#ifndef _RISCV_RISCV_H
#define _RISCV_RISCV_H

#include <libc/stdint.h>

#define RISCV_INSTRUCTION_FROM(funct7, funct3, opcode) (opcode) | ((funct3) << 7) | ((funct7) << 10)

// Enumeration of RISC-V instructions, formed by packing the funct7, funct3, and opcode fields together
enum riscv_inst
{
    UNK = 0,

    LUI = RISCV_INSTRUCTION_FROM(0b0000000, 0b000, 0b0110111),
    AUIPC = RISCV_INSTRUCTION_FROM(0b0000000, 0b000, 0b0010111),
    JAL = RISCV_INSTRUCTION_FROM(0b0000000, 0b000, 0b1101111),
    JALR = RISCV_INSTRUCTION_FROM(0b0000000, 0b000, 0b1100111),

    BEQ = RISCV_INSTRUCTION_FROM(0b0000000, 0b000, 0b1100011),
    BNE = RISCV_INSTRUCTION_FROM(0b0000000, 0b001, 0b1100011),
    BLT = RISCV_INSTRUCTION_FROM(0b0000000, 0b100, 0b1100011),
    BGE = RISCV_INSTRUCTION_FROM(0b0000000, 0b101, 0b1100011),
    BLTU = RISCV_INSTRUCTION_FROM(0b0000000, 0b110, 0b1100011),
    BGEU = RISCV_INSTRUCTION_FROM(0b0000000, 0b111, 0b1100011),

    LB = RISCV_INSTRUCTION_FROM(0b0000000, 0b000, 0b0000011),
    LH = RISCV_INSTRUCTION_FROM(0b0000000, 0b001, 0b0000011),
    LW = RISCV_INSTRUCTION_FROM(0b0000000, 0b010, 0b0000011),
    LD = RISCV_INSTRUCTION_FROM(0b0000000, 0b011, 0b0000011),
    LBU = RISCV_INSTRUCTION_FROM(0b0000000, 0b100, 0b0000011),
    LHU = RISCV_INSTRUCTION_FROM(0b0000000, 0b101, 0b0000011),
    LWU = RISCV_INSTRUCTION_FROM(0b0000000, 0b110, 0b0000011),

    SB = RISCV_INSTRUCTION_FROM(0b0000000, 0b000, 0b0100011),
    SH = RISCV_INSTRUCTION_FROM(0b0000000, 0b001, 0b0100011),
    SW = RISCV_INSTRUCTION_FROM(0b0000000, 0b010, 0b0100011),
    SD = RISCV_INSTRUCTION_FROM(0b0000000, 0b011, 0b0100011),

    ADDI = RISCV_INSTRUCTION_FROM(0b0000000, 0b000, 0b0010011),
    SLTI = RISCV_INSTRUCTION_FROM(0b0000000, 0b010, 0b0010011),
    SLTIU = RISCV_INSTRUCTION_FROM(0b0000000, 0b011, 0b0010011),
    XORI = RISCV_INSTRUCTION_FROM(0b0000000, 0b100, 0b0010011),
    ORI = RISCV_INSTRUCTION_FROM(0b0000000, 0b110, 0b0010011),
    ANDI = RISCV_INSTRUCTION_FROM(0b0000000, 0b111, 0b0010011),
    SLLI = RISCV_INSTRUCTION_FROM(0b0000000, 0b001, 0b0010011),
    SRLI = RISCV_INSTRUCTION_FROM(0b0000000, 0b101, 0b0010011),
    SRAI = RISCV_INSTRUCTION_FROM(0b0100000, 0b101, 0b0010011),

    ADD = RISCV_INSTRUCTION_FROM(0b0000000, 0b000, 0b0110011),
    SUB = RISCV_INSTRUCTION_FROM(0b0100000, 0b000, 0b0110011),
    SLL = RISCV_INSTRUCTION_FROM(0b0000000, 0b001, 0b0110011),
    SLT = RISCV_INSTRUCTION_FROM(0b0000000, 0b010, 0b0110011),
    SLTU = RISCV_INSTRUCTION_FROM(0b0000000, 0b011, 0b0110011),
    XOR = RISCV_INSTRUCTION_FROM(0b0000000, 0b100, 0b0110011),
    SRL = RISCV_INSTRUCTION_FROM(0b0000000, 0b101, 0b0110011),
    SRA = RISCV_INSTRUCTION_FROM(0b0100000, 0b101, 0b0110011),
    OR = RISCV_INSTRUCTION_FROM(0b0000000, 0b110, 0b0110011),
    AND = RISCV_INSTRUCTION_FROM(0b0000000, 0b111, 0b0110011),

    ECALL = RISCV_INSTRUCTION_FROM(0b0000000, 0b000, 0b1110011),
    EBREAK = RISCV_INSTRUCTION_FROM(0b0000001, 0b000, 0b1110011),

    ADDIW = RISCV_INSTRUCTION_FROM(0b0000000, 0b000, 0b0011011),
    SLLIW = RISCV_INSTRUCTION_FROM(0b0000000, 0b001, 0b0011011),
    SRLIW = RISCV_INSTRUCTION_FROM(0b0000000, 0b101, 0b0011011),
    SRAIW = RISCV_INSTRUCTION_FROM(0b0100000, 0b101, 0b0011011),

    ADDW = RISCV_INSTRUCTION_FROM(0b0000000, 0b000, 0b0111011),
    SUBW = RISCV_INSTRUCTION_FROM(0b0100000, 0b000, 0b0111011),
    SLLW = RISCV_INSTRUCTION_FROM(0b0000000, 0b001, 0b0111011),
    SRLW = RISCV_INSTRUCTION_FROM(0b0000000, 0b101, 0b0111011),
    SRAW = RISCV_INSTRUCTION_FROM(0b0100000, 0b101, 0b0111011),
};

// Enumeration of instruction formats
enum riscv_fmt
{
    SPECIAL,
    RFORMAT,
    IFORMAT,
    SFORMAT,
    BFORMAT,
    UFORMAT,
    JFORMAT
};

// Representation of a RISC-V instruction
struct riscv_inst_repr
{
    enum riscv_inst inst;
    enum riscv_fmt fmt;

    uint8_t r_dest;
    uint8_t r_src1;
    uint8_t r_src2;

    uint32_t imm;

    uint8_t width;
};

// Convert an enum riscv_inst to a string
char* riscv_inst_to_str(enum riscv_inst);

// Parse a uint32_t into a RISC-V instruction
int riscv_parse_inst(uint32_t raw, struct riscv_inst_repr* result);

// Convert a RISC-V instruction represented as a struct riscv_inst_repr into a string (give atleast 64) bytes of space for the string
int riscv_render_inst(char* s, struct riscv_inst_repr* result, uint64_t addr);

// Convert a RISC-V instruction represented as a struct riscv_inst_repr into a string (give atleast 256) bytes of space for the string
// in this version of the function, there is a function pointer passed that every address reference is passed to, this allows desymbolication or pretty printing of addresses
int riscv_render_inst_symbols(char* s, struct riscv_inst_repr* result, uint64_t addr, int (*render_addr)(char*, uint64_t));

// Assemble a riscv_inst_repr into a 32 bit integer
uint32_t riscv_assemble(struct riscv_inst_repr inst);

#endif // _RISCV_RISCV_H