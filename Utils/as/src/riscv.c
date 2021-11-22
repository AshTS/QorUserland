#include "riscv.h"

#include <libc/assert.h>

#include "stdio.h"

char* render_instruction(struct Instruction* inst)
{
    static char buffer[128];

    size_t length;

    switch (inst->instruction)
    {
        case ADDI:
            length = sprintf(buffer, "addi x%i, x%i, %ld", inst->rdest, inst->rs1, inst->imm);
            break;
        case ECALL:
            length = sprintf(buffer, "ecall");
            break;
        default:
            assert(0);
    }

    assert(length < 127);

    return buffer;
}


uint32_t compile_instruction(struct Instruction* inst)
{
    uint32_t result = 0;

    switch (inst->instruction)
    {
        case ADDI:
            result = (uint32_t)0b00000000000000000000000000010011;
            result |= inst->imm << 20;
            result |= inst->rs1 << 15;
            result |= inst->rdest << 7;
            return result;
        case ECALL:
            return   (uint32_t)0b00000000000000000000000001110011;
        default:
            printf("Unhandled Instruction `%s`\n", render_instruction(inst));
            assert(0 && "Not yet Implemented");
    }

}