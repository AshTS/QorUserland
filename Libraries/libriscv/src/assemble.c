#include <riscv/riscv.h>

#include <libc/assert.h>
#include <libc/stdio.h>

// Assemble i format instructions
uint32_t assemble_iformat(uint32_t opcode, uint32_t funct3, uint32_t rd, uint32_t rs1, uint32_t imm)
{
    uint32_t result = 0;

    result |= opcode;
    result |= (rd << 7);
    result |= (funct3 << 12);
    result |= (rs1 << 15);
    result |= (imm << 20);

    return result;
}

// Assemble s format instructions
uint32_t assemble_sformat(uint32_t opcode, uint32_t funct3, uint32_t rd, uint32_t rs1, uint32_t rs2, uint32_t imm)
{
    uint32_t result = 0;

    result |= opcode;
    result |= (((imm >> 0) & (0b11111)) << 7);
    result |= (funct3 << 12);
    result |= (rs1 << 15);
    result |= (rs2 << 20);
    result |= (((imm >> 5) & (0b1111111)) << 25);

    return result;
}

// Assemble u format instructions
uint32_t assemble_uformat(uint32_t opcode, uint32_t rd, uint32_t imm)
{
    uint32_t result = 0;

    result |= opcode;
    result |= (rd << 7);
    result |= ((imm >> 12) << 12);

    return result;
}

// Assemble a riscv_inst_repr into a 32 bit integer
uint32_t riscv_assemble(struct riscv_inst_repr inst)
{
    // RV32I
    if (inst.inst == AUIPC)
    {
        return assemble_uformat(0b0010111, inst.r_dest, inst.imm);
    }
    else if (inst.inst == JALR)
    {
        return assemble_iformat(0b1100111, 0b000, inst.r_dest, inst.r_src1, inst.imm);
    }
    else if (inst.inst == ADDI)
    {
        return assemble_iformat(0b0010011, 0b000, inst.r_dest, inst.r_src1, inst.imm);
    }
    else if (inst.inst == ECALL)
    {
        return 0b1110011;
    }
    // RV64I
    else if (inst.inst == LD)
    {
        return assemble_iformat(0b0000011, 0b011, inst.r_dest, inst.r_src1, inst.imm);
    }
    else if (inst.inst == SD)
    {
        return assemble_iformat(0b0100011, 0b011, inst.r_dest, inst.r_src1, inst.imm);
    }
    else
    {
        printf("Not yet implemented\n");
        return 0;
    }

    
}
