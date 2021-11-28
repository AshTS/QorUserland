#include "riscv.h"

#include <libc/assert.h>
#include <libc/stdio.h>

char* render_instruction(struct Instruction* inst)
{
    static char buffer[128];

    size_t length;

    switch (inst->instruction)
    {
        case ADDI:
            length = sprintf(buffer, "addi x%i, x%i, %ld", inst->rdest, inst->rs1, inst->imm);
            break;
        case SLTI:
            length = sprintf(buffer, "slti x%i, x%i, %ld", inst->rdest, inst->rs1, inst->imm);
            break;
        case SLTIU:
            length = sprintf(buffer, "sltiu x%i, x%i, %ld", inst->rdest, inst->rs1, inst->imm);
            break;
        case XORI:
            length = sprintf(buffer, "xori x%i, x%i, %ld", inst->rdest, inst->rs1, inst->imm);
            break;
        case ORI:
            length = sprintf(buffer, "ori x%i, x%i, %ld", inst->rdest, inst->rs1, inst->imm);
            break;
        case ANDI:
            length = sprintf(buffer, "andi x%i, x%i, %ld", inst->rdest, inst->rs1, inst->imm);
            break;
        case SLLI:
            length = sprintf(buffer, "slli x%i, x%i, %ld", inst->rdest, inst->rs1, inst->imm);
            break;
        case SRLI:
            length = sprintf(buffer, "srli x%i, x%i, %ld", inst->rdest, inst->rs1, inst->imm);
            break;
        case SRAI:
            length = sprintf(buffer, "srai x%i, x%i, %ld", inst->rdest, inst->rs1, inst->imm);
            break;
        case ADDIW:
            length = sprintf(buffer, "addiw x%i, x%i, %ld", inst->rdest, inst->rs1, inst->imm);
            break;
        case SLLIW:
            length = sprintf(buffer, "slliw x%i, x%i, %ld", inst->rdest, inst->rs1, inst->imm);
            break;
        case SRLIW:
            length = sprintf(buffer, "srliw x%i, x%i, %ld", inst->rdest, inst->rs1, inst->imm);
            break;
        case SRAIW:
            length = sprintf(buffer, "sraiw x%i, x%i, %ld", inst->rdest, inst->rs1, inst->imm);
            break;
        case ADD:
            length = sprintf(buffer, "add x%i, x%i, x%i", inst->rdest, inst->rs1, inst->rs2);
            break;
        case SUB:
            length = sprintf(buffer, "sub x%i, x%i, x%i", inst->rdest, inst->rs1, inst->rs2);
            break;
        case SLL:
            length = sprintf(buffer, "sll x%i, x%i, x%i", inst->rdest, inst->rs1, inst->rs2);
            break;
        case SLT:
            length = sprintf(buffer, "slt x%i, x%i, x%i", inst->rdest, inst->rs1, inst->rs2);
            break;
        case SLTU:
            length = sprintf(buffer, "sltu x%i, x%i, x%i", inst->rdest, inst->rs1, inst->rs2);
            break;
        case XOR:
            length = sprintf(buffer, "xor x%i, x%i, x%i", inst->rdest, inst->rs1, inst->rs2);
            break;
        case SRL:
            length = sprintf(buffer, "srl x%i, x%i, x%i", inst->rdest, inst->rs1, inst->rs2);
            break;
        case SRA:
            length = sprintf(buffer, "or x%i, x%i, x%i", inst->rdest, inst->rs1, inst->rs2);
            break;
        case OR:
            length = sprintf(buffer, "srl x%i, x%i, x%i", inst->rdest, inst->rs1, inst->rs2);
            break;
        case AND:
            length = sprintf(buffer, "and x%i, x%i, x%i", inst->rdest, inst->rs1, inst->rs2);
            break;
        case ADDW:
            length = sprintf(buffer, "addw x%i, x%i, x%i", inst->rdest, inst->rs1, inst->rs2);
            break;
        case SUBW:
            length = sprintf(buffer, "subw x%i, x%i, x%i", inst->rdest, inst->rs1, inst->rs2);
            break;
        case SLLW:
            length = sprintf(buffer, "sllw x%i, x%i, x%i", inst->rdest, inst->rs1, inst->rs2);
            break;
        case SRLW:
            length = sprintf(buffer, "srlw x%i, x%i, x%i", inst->rdest, inst->rs1, inst->rs2);
            break;
        case SRAW:
            length = sprintf(buffer, "sraw x%i, x%i, x%i", inst->rdest, inst->rs1, inst->rs2);
            break;
        case LUI:
            length = sprintf(buffer, "lui x%i, %ld", inst->rdest, inst->imm);
            break;
        case ECALL:
            length = sprintf(buffer, "ecall");
            break;
        case JAL:
            length = sprintf(buffer, "jal x%i, %s", inst->rdest, inst->link);
            break;
        case BEQ:
            length = sprintf(buffer, "beq x%i, x%i, %s", inst->rs1, inst->rs2, inst->link);
            break;
        case BNE:
            length = sprintf(buffer, "bne x%i, x%i, %s", inst->rs1, inst->rs2, inst->link);
            break;
        case BLT:
            length = sprintf(buffer, "blt x%i, x%i, %s", inst->rs1, inst->rs2, inst->link);
            break;
        case BGE:
            length = sprintf(buffer, "bge x%i, x%i, %s", inst->rs1, inst->rs2, inst->link);
            break;
        case BLTU:
            length = sprintf(buffer, "bltu x%i, x%i, %s", inst->rs1, inst->rs2, inst->link);
            break;
        case BGEU:
            length = sprintf(buffer, "bgeu x%i, x%i, %s", inst->rs1, inst->rs2, inst->link);
            break;
        default:
            assert(0);
    }

    assert(length < 127);

    return buffer;
}


#define INSTRUCTION_BUILD(opcode, rd, funct3, rs1, rs2, funct7) (uint32_t)((opcode) | ((rd) << 7) | ((funct3) << 12) | ((rs1) << 15) | ((rs2) << 20) | ((funct7) << 25))

uint32_t compile_instruction(struct Instruction* inst)
{
    uint32_t result = 0;

    switch (inst->instruction)
    {
        
        case LUI:
            return INSTRUCTION_BUILD(0b0110111, inst->rdest, 0, 0, 0, 0);
        case JAL:
            return INSTRUCTION_BUILD(0b1101111, inst->rdest, 0, 0, 0, 0);
        case JALR:
            return INSTRUCTION_BUILD(0b1100111, inst->rdest, 0b000, inst->rs1, 0, 0);
        case LB:
            return INSTRUCTION_BUILD(0b0000011, inst->rdest, 0b000, inst->rs1, inst->imm, 0);
        case LH:
            return INSTRUCTION_BUILD(0b0000011, inst->rdest, 0b001, inst->rs1, inst->imm, 0);
        case LW:
            return INSTRUCTION_BUILD(0b0000011, inst->rdest, 0b010, inst->rs1, inst->imm, 0);
        case LD:
            return INSTRUCTION_BUILD(0b0000011, inst->rdest, 0b011, inst->rs1, inst->imm, 0);
        case LBU:
            return INSTRUCTION_BUILD(0b0000011, inst->rdest, 0b100, inst->rs1, inst->imm, 0);
        case LHU:
            return INSTRUCTION_BUILD(0b0000011, inst->rdest, 0b101, inst->rs1, inst->imm, 0);
        case LWU:
            return INSTRUCTION_BUILD(0b0000011, inst->rdest, 0b110, inst->rs1, inst->imm, 0);
        case SB:
            return INSTRUCTION_BUILD(0b0100011, inst->imm & 0b11111, 0b000, inst->rs1, inst->rs2, inst->imm >> 5);
        case SH:
            return INSTRUCTION_BUILD(0b0100011, inst->imm & 0b11111, 0b001, inst->rs1, inst->rs2, inst->imm >> 5);
        case SW:
            return INSTRUCTION_BUILD(0b0100011, inst->imm & 0b11111, 0b010, inst->rs1, inst->rs2, inst->imm >> 5);
        case SD:
            return INSTRUCTION_BUILD(0b0100011, inst->imm & 0b11111, 0b011, inst->rs1, inst->rs2, inst->imm >> 5);
        case BEQ:
            return INSTRUCTION_BUILD(0b1100011, 0, 0b000, inst->rs1, inst->rs2, 0);
        case BNE:
            return INSTRUCTION_BUILD(0b1100011, 0, 0b001, inst->rs1, inst->rs2, 0);
        case BLT:
            return INSTRUCTION_BUILD(0b1100011, 0, 0b100, inst->rs1, inst->rs2, 0);
        case BGE:
            return INSTRUCTION_BUILD(0b1100011, 0, 0b101, inst->rs1, inst->rs2, 0);
        case BLTU:
            return INSTRUCTION_BUILD(0b1100011, 0, 0b110, inst->rs1, inst->rs2, 0);
        case BGEU:
            return INSTRUCTION_BUILD(0b1100011, 0, 0b111, inst->rs1, inst->rs2, 0);
        case ADDI:
            return INSTRUCTION_BUILD(0b0010011, inst->rdest, 0b000, inst->rs1, inst->imm, 0);
        case SLTI:
            return INSTRUCTION_BUILD(0b0010011, inst->rdest, 0b010, inst->rs1, inst->imm, 0);
        case SLTIU:
            return INSTRUCTION_BUILD(0b0010011, inst->rdest, 0b011, inst->rs1, inst->imm, 0);
        case XORI:
            return INSTRUCTION_BUILD(0b0010011, inst->rdest, 0b100, inst->rs1, inst->imm, 0);
        case ORI:
            return INSTRUCTION_BUILD(0b0010011, inst->rdest, 0b110, inst->rs1, inst->imm, 0);
        case ANDI:
            return INSTRUCTION_BUILD(0b0010011, inst->rdest, 0b111, inst->rs1, inst->imm, 0);
        case SLLI:
            return INSTRUCTION_BUILD(0b0010011, inst->rdest, 0b001, inst->rs1, inst->imm & 0x3F, 0);
        case SRLI:
            return INSTRUCTION_BUILD(0b0010011, inst->rdest, 0b111, inst->rs1, inst->imm & 0x3F, 0);
        case SRAI:
            return INSTRUCTION_BUILD(0b0010011, inst->rdest, 0b111, inst->rs1, inst->imm & 0x3F, 0b01000000);
        case ADDIW:
            return INSTRUCTION_BUILD(0b0011011, inst->rdest, 0b000, inst->rs1, inst->imm, 0);
        case SLLIW:
            return INSTRUCTION_BUILD(0b0011011, inst->rdest, 0b001, inst->rs1, inst->imm, 0);
        case SRLIW:
            return INSTRUCTION_BUILD(0b0011011, inst->rdest, 0b101, inst->rs1, inst->imm, 0);
        case SRAIW:
            return INSTRUCTION_BUILD(0b0011011, inst->rdest, 0b101, inst->rs1, inst->imm, 0b0100000);
        case ADD:
            return INSTRUCTION_BUILD(0b0110011, inst->rdest, 0b000, inst->rs1, inst->rs2, 0b0000000);
        case SUB:
            return INSTRUCTION_BUILD(0b0110011, inst->rdest, 0b000, inst->rs1, inst->rs2, 0b0100000);
        case SLL:
            return INSTRUCTION_BUILD(0b0110011, inst->rdest, 0b001, inst->rs1, inst->rs2, 0b0000000);
        case SLT:
            return INSTRUCTION_BUILD(0b0110011, inst->rdest, 0b010, inst->rs1, inst->rs2, 0b0000000);
        case SLTU:
            return INSTRUCTION_BUILD(0b0110011, inst->rdest, 0b011, inst->rs1, inst->rs2, 0b0000000);
        case XOR:
            return INSTRUCTION_BUILD(0b0110011, inst->rdest, 0b100, inst->rs1, inst->rs2, 0b0000000);
        case SRL:
            return INSTRUCTION_BUILD(0b0110011, inst->rdest, 0b101, inst->rs1, inst->rs2, 0b0000000);
        case SRA:
            return INSTRUCTION_BUILD(0b0110011, inst->rdest, 0b101, inst->rs1, inst->rs2, 0b0100000);
        case OR:
            return INSTRUCTION_BUILD(0b0110011, inst->rdest, 0b110, inst->rs1, inst->rs2, 0b0000000);
        case AND:
            return INSTRUCTION_BUILD(0b0110011, inst->rdest, 0b111, inst->rs1, inst->rs2, 0b0000000);
        case ADDW:
            return INSTRUCTION_BUILD(0b0111011, inst->rdest, 0b000, inst->rs1, inst->rs2, 0b0000000);
        case SUBW:
            return INSTRUCTION_BUILD(0b0111011, inst->rdest, 0b000, inst->rs1, inst->rs2, 0b0100000);
        case SLLW:
            return INSTRUCTION_BUILD(0b0111011, inst->rdest, 0b001, inst->rs1, inst->rs2, 0b0000000);
        case SRLW:
            return INSTRUCTION_BUILD(0b0111011, inst->rdest, 0b101, inst->rs1, inst->rs2, 0b0000000);
        case SRAW:
            return INSTRUCTION_BUILD(0b0111011, inst->rdest, 0b101, inst->rs1, inst->rs2, 0b0100000);
        case ECALL:
            return INSTRUCTION_BUILD(0b1110011, 0, 0, 0, 0, 0);
        default:
            printf("Unhandled Instruction `%s`\n", render_instruction(inst));
            assert(0 && "Not yet Implemented");
    }

}