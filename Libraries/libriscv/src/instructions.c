#include <riscv/riscv.h>

#include <libc/assert.h>
#include <libc/stdio.h>

// Convert an enum riscv_inst to a string
char* riscv_inst_to_str(enum riscv_inst inst)
{
    switch (inst)
    {
        case LUI: return "lui";
        case AUIPC: return "auipc";
        case JAL: return "jal";
        case JALR: return "jalr";

        case BEQ: return "beq";
        case BNE: return "bne";
        case BLT: return "blt";
        case BGE: return "bge";
        case BLTU: return "bltu";
        case BGEU: return "bgeu";

        case LB: return "lb";
        case LH: return "lh";
        case LW: return "lw";
        case LD: return "ld";
        case LBU: return "lbu";
        case LHU: return "lhu";
        case LWU: return "lwu";

        case SB: return "sb";
        case SH: return "sh";
        case SW: return "sw";
        case SD: return "sd";

        case ADDI: return "addi";
        case SLTI: return "slti";
        case SLTIU: return "sltiu";
        case XORI: return "xori";
        case ORI: return "ori";
        case ANDI: return "andi";
        case SLLI: return "slli";
        case SRLI: return "srli";
        case SRAI: return "srai";

        case ADD: return "add";
        case SUB: return "sub";
        case SLL: return "sll";
        case SLT: return "slt";
        case SLTU: return "sltu";
        case XOR: return "xor";
        case SRL: return "srl";
        case SRA: return "sra";
        case OR: return "or";
        case AND: return "and";

        case EBREAK: return "ebreak";
        case ECALL: return "ecall";

        case ADDIW: return "addiw";
        case SLLIW: return "slliw";
        case SRLIW: return "srliw";
        case SRAIW: return "sraiw";

        case ADDW: return "addw";
        case SUBW: return "subw";
        case SLLW: return "sllw";
        case SRLW: return "srlw";
        case SRAW: return "sraw";

        default: return "unk";
    }
}

// Sign extend a value from the given bit width to 32 bits wide
int riscv_sign_extend(int v, int width)
{
    int mask = (1 << width);
    int or_mask = ~(mask - 1);

    if (v & (mask >> 1))
    {
        v |= or_mask;
    }

    return v;
}

// Parse a uint32_t into a RISC-V instruction
int riscv_parse_inst(uint32_t raw, struct riscv_inst_repr* result)
{
    uint32_t opcode = raw & 0b1111111;
    
    uint8_t last = opcode & 0b11;

    uint32_t funct3c = (raw >> 13) & 0b111;
    uint32_t funct4c = (raw >> 12) & 0b1111;
    uint32_t funct3 = (raw >> 12) & 0b111;
    uint32_t funct7 = (raw >> 25) & 0b1111111;

    result->imm = 0;
    result->inst = UNK;
    result->r_dest = 0;
    result->r_src1 = 0;
    result->r_src2 = 0;
    result->width = 2;

    result->fmt = SPECIAL;

    if ((raw & 0xFFFF) == 0)
    {
        return 0;
    }

    if (last == 0b11)
    {
        result->width = 4;

        switch (opcode)
        {
            case 0b0110011:
            case 0b0111011:
                // R format
                result->r_dest = (raw >> 7) & 0b11111;
                result->r_src1 = (raw >> 15) & 0b11111;
                result->r_src2 = (raw >> 20) & 0b11111;

                result->fmt = RFORMAT;
                break;
            
            case 0b1100111:
            case 0b0000011:
            case 0b0010011:
            case 0b0011011:
                // I format

                result->r_dest = (raw >> 7) & 0b11111;
                result->r_src1 = (raw >> 15) & 0b11111;

                result->imm = (raw >> 20);

                result->imm = riscv_sign_extend(result->imm, 12);

                result->fmt = IFORMAT;

                // Add a special case for the shift functions that doesn't remove the funct7
                if ((opcode == 0b0010011 || opcode == 0b0011011) && (funct3 == 0b001 || funct3 == 0b101))
                {
                    result->imm &= 0b111111;
                    funct7 &= 0b1111110;
                }
                else
                {
                    funct7 &= 0;
                }

                break;

            case 0b0100011:
                // S format
                funct7 &= 0;

                result->r_src1 = (raw >> 15) & 0b11111;
                result->r_src2 = (raw >> 20) & 0b11111;

                result->imm = (raw >> 20) & (~0b11111);
                result->imm |= (raw >> 7) & 0b11111;

                result->imm = riscv_sign_extend(result->imm, 12);

                result->fmt = SFORMAT;
                break;

            case 0b1100011:
                // B format
                funct7 &= 0;

                result->r_src1 = (raw >> 15) & 0b11111;
                result->r_src2 = (raw >> 20) & 0b11111;

                result->imm = ((raw >> 7) & 0b1) << 11;
                result->imm |= ((raw >> 8) & 0b1111) << 1;
                result->imm |= ((raw >> 25) & 0b111111) << 5;
                result->imm |= ((raw >> 31) & 0b1) << 12;

                result->imm = riscv_sign_extend(result->imm, 13);

                result->fmt = BFORMAT;
                break;

            case 0b0110111:
            case 0b0010111:
                // U format
                funct3 &= 0;
                funct7 &= 0;

                result->r_dest = (raw >> 7) & 0b11111;

                result->imm = raw & (~0b111111111111);

                result->fmt = UFORMAT;
                break;

            case 0b1101111:
                // J format
                funct3 &= 0;
                funct7 &= 0;

                result->r_dest = (raw >> 7) & 0b11111;

                result->imm = raw & (0b11111111000000000000);
                result->imm |= (raw >> 20) & 0b11110;
                result->imm |= ((raw >> 20) & 1) << 11;
                result->imm |= ((raw >> 25) & 1) << 5;
                result->imm |= ((raw >> 31) & 1) << 20;

                result->imm = riscv_sign_extend(result->imm, 21);

                result->fmt = JFORMAT;
                break;
        }

        result->inst = RISCV_INSTRUCTION_FROM(funct7, funct3, opcode);
    }
    else if (last == 0b00)
    {
        if (funct3c == 0b000)
        {
            result->imm = ((raw >> 5) & 0b1) << 3;
            result->imm |= ((raw >> 6) & 0b1) << 2;
            result->imm |= ((raw >> 7) & 0b1111) << 6;
            result->imm |= ((raw >> 11) & 0b11) << 4;

            result->imm = riscv_sign_extend(result->imm, 10);

            result->r_dest = 8 + (raw >> 2) & 0b111;
            result->r_src1 = 2;

            result->inst = ADDI;
            result->fmt = IFORMAT;
        }
        else if (funct3c == 0b010)
        {
            result->imm = ((raw >> 5) & 0b1) << 6;
            result->imm |= ((raw >> 6) & 0b1) << 2;
            result->imm |= ((raw >>10) & 0b111) << 3;

            result->r_dest = 8 + ((raw >> 2) & 0b111);
            result->r_src1 = 8 + ((raw >> 7) & 0b111);

            result->inst = LW;
            result->fmt = IFORMAT;
        }
        else if (funct3c == 0b011)
        {
            result->imm = ((raw >> 5) & 0b11) << 6;
            result->imm |= ((raw >> 10) & 0b111) << 3;

            result->r_dest = 8 + ((raw >> 2) & 0b111);
            result->r_src1 = 8 + ((raw >> 7) & 0b111);

            result->inst = LD;
            result->fmt = IFORMAT;
        }
        else if (funct3c == 0b110)
        {
            result->imm = ((raw >> 5) & 0b1) << 6;
            result->imm |= ((raw >> 6) & 0b1) << 2;
            result->imm |= ((raw >> 10) & 0b111) << 3;

            result->r_src1 = 8 + ((raw >> 7) & 0b111);
            result->r_src2 = 8 + ((raw >> 2) & 0b111);

            result->inst = SW;
            result->fmt = SFORMAT;
        }
        else if (funct3c == 0b111)
        {
            result->imm = ((raw >> 5) & 0b11) << 6;
            result->imm |= ((raw >> 10) & 0b111) << 3;

            result->r_src2 = 8 + ((raw >> 2) & 0b111);
            result->r_src1 = 8 + ((raw >> 7) & 0b111);

            result->inst = SD;
            result->fmt = SFORMAT;
        }
        else
        {
            printf("Value: %x\n", funct3c);
            assert(0 && "Not yet implemented 0b00");
        }
    }
    else if (last == 0b01)
    {
        if (funct3c == 0b000)
        {
            result->imm = (raw >> 2) & 0b11111;
            result->imm |= ((raw >> 12) & 0b1) << 5;

            result->imm = riscv_sign_extend(result->imm, 6);

            uint32_t reg = (raw >> 7) & 0b11111;

            result->r_dest = reg;
            result->r_src1 = reg;

            result->inst = ADDI;
            result->fmt = IFORMAT;
        }
        else if (funct3c == 0b001)
        {
            result->imm = (raw >> 2) & 0b11111;
            result->imm |= ((raw >> 12) & 0b1) << 5;

            result->imm = riscv_sign_extend(result->imm, 6);

            uint32_t reg = (raw >> 7) & 0b11111;

            result->r_dest = reg;
            result->r_src1 = reg;

            result->inst = ADDIW;
            result->fmt = IFORMAT;
        }
        else if (funct3c == 0b010)
        {
            result->imm = (raw >> 2) & 0b11111;
            result->imm |= ((raw >> 12) & 0b1) << 5;

            result->imm = riscv_sign_extend(result->imm, 6);

            uint32_t reg = (raw >> 7) & 0b11111;

            result->r_dest = reg;
            result->r_src1 = 0;

            result->inst = ADDI;
            result->fmt = IFORMAT;
        }
        else if (funct3c == 0b011)
        {
            uint32_t reg = (raw >> 7) & 0b11111;

            if (reg == 2)
            {
                result->imm = ((raw >> 2) & 0b11111) << 12;
                result->imm |= ((raw >> 12) & 0b1) << 17;

                result->imm = riscv_sign_extend(result->imm, 18);

                result->r_dest = 2;
                result->r_src1 = 2;

                result->inst = ADDI;
                result->fmt = IFORMAT;
            }
            else
            {
                result->imm = ((raw >> 2) & 0b1) << 5;
                result->imm |= ((raw >> 3) & 0b11) << 7;
                result->imm |= ((raw >> 5) & 0b1) << 6;
                result->imm |= ((raw >> 6) & 0b1) << 4;
                result->imm |= ((raw >> 12) & 0b1) << 9;

                result->imm = riscv_sign_extend(result->imm, 10);

                result->r_dest = reg;

                result->inst = LUI;
                result->fmt = UFORMAT;
            }
        }
        else if (funct3c == 0b100)
        {
            result->imm = (raw >> 2) & 0b11111;
            result->imm |= ((raw >> 12) & 0b1) << 5;

            uint32_t reg = 8 + ((raw >> 7) & 0b111);

            uint32_t f2a = (raw >> 5) & 0b11;
            uint32_t f2b = (raw >> 10) & 0b11;
            uint32_t f3b = (raw >> 10) & 0b111;

            if (f2b == 0b00)
            {
                result->r_dest = reg;
                result->r_src1 = reg;

                result->inst = SRLI;
                result->fmt = IFORMAT;
            }
            else if (f2b == 0b01)
            {
                result->r_dest = reg;
                result->r_src1 = reg;

                result->inst = SRAI;
                result->fmt = IFORMAT;
            }
            else if (f2b == 0b10)
            {
                result->r_dest = reg;
                result->r_src1 = reg;

                result->imm = riscv_sign_extend(result->imm, 6);

                result->inst = ANDI;
                result->fmt = IFORMAT;
            }
            else
            {
                result->r_dest = reg;
                result->r_src1 = reg;
                result->r_src2 = 8 + ((raw >> 2) & 0b111);

                result->fmt = RFORMAT;

                switch (f3b)
                {
                    case 0b000:
                        result->inst = SUB;
                        break;
                    case 0b001:
                        result->inst = XOR;
                        break;
                    case 0b010:
                        result->inst = OR;
                        break;
                    case 0b011:
                        result->inst = AND;
                        break;
                    case 0b100:
                        result->inst = SUBW;
                        break;
                    case 0b101:
                        result->inst = ADDW;
                        break;
                    case 0b110:
                    case 0b111:
                        result->inst = UNK;
                        result->fmt = SPECIAL;
                        break;
                }
            }
        }
        else if (funct3c == 0b101)
        {
            result->imm = ((raw >> 2) & 0b1) << 5;
            result->imm |= ((raw >> 3) & 0b111) << 1;
            result->imm |= ((raw >> 6) & 0b1) << 7;
            result->imm |= ((raw >> 7) & 0b1) << 6;
            result->imm |= ((raw >> 8) & 0b1) << 10;
            result->imm |= ((raw >> 9) & 0b11) << 8;
            result->imm |= ((raw >> 11) & 0b1) << 4;
            result->imm |= ((raw >> 12) & 0b1) << 11;

            result->imm = riscv_sign_extend(result->imm, 12);

            result->r_dest = 0;

            result->inst = JAL;
            result->fmt = JFORMAT;
        }
        else if (funct3c == 0b110)
        {
            result->imm = ((raw >> 2) & 0b1) << 5;
            result->imm |= ((raw >> 3) & 0b11) << 1;
            result->imm |= ((raw >> 5) & 0b11) << 6;
            result->imm |= ((raw >> 10) & 0b11) << 3;
            result->imm |= ((raw >> 12) & 0b1) << 8;

            result->imm = riscv_sign_extend(result->imm, 8);

            result->r_src1 = 8 + ((raw >> 7) & 0b111);
            result->r_src2 = 0;

            result->inst = BEQ;
            result->fmt = BFORMAT;
        }
        else if (funct3c == 0b111)
        {
            result->imm = ((raw >> 2) & 0b1) << 5;
            result->imm |= ((raw >> 3) & 0b11) << 1;
            result->imm |= ((raw >> 5) & 0b11) << 6;
            result->imm |= ((raw >> 10) & 0b11) << 3;
            result->imm |= ((raw >> 12) & 0b1) << 8;

            result->imm = riscv_sign_extend(result->imm, 8);

            result->r_src1 = 8 + ((raw >> 7) & 0b111);
            result->r_src2 = 0;

            result->inst = BNE;
            result->fmt = BFORMAT;
        }
        else
        {
            printf("Value: %x\n", funct3c);
            assert(0 && "Not yet implemented 0b01");
        }
    }
    else if (last == 0b10)
    {
        if (funct3c == 0b000)
        {
            result->imm = ((raw >> 2) & 0b11111) << 0;
            result->imm |= ((raw >> 12) & 0b1) << 3;

            uint32_t reg = (raw >> 7) & 0b11111;

            result->r_dest = reg;
            result->r_src1 = reg;

            result->inst = SLLI;
            result->fmt = IFORMAT;
        }
        else if (funct3c == 0b010)
        {
            result->imm = ((raw >> 2) & 0b11) << 6;
            result->imm |= ((raw >> 4) & 0b111) << 2;
            result->imm |= ((raw >> 12) & 0b1) << 5;

            result->r_dest = (raw >> 7) & 0b11111;
            result->r_src1 = 2;

            result->inst = LW;
            result->fmt = IFORMAT;
        }
        else if (funct3c == 0b011)
        {
            result->imm = ((raw >> 2) & 0b111) << 6;
            result->imm |= ((raw >> 5) & 0b11) << 3;
            result->imm |= ((raw >> 12) & 0b1) << 5;

            result->r_dest = (raw >> 7) & 0b11111;
            result->r_src1 = 2;

            result->inst = LD;
            result->fmt = IFORMAT;
        }
        else if (funct4c == 0b1000)
        {
            uint32_t v0 = (raw >> 2) & 0b11111;
            uint32_t v1 = (raw >> 7) & 0b11111;
            
            if (v0 == 0)
            {
                result->r_src1 = v1;

                result->inst = JALR;
                result->fmt = IFORMAT;
            }
            else
            {
                result->r_dest = v0;
                result->r_src1 = 0;
                result->r_src2 = v1;

                result->inst = ADD;
                result->fmt = RFORMAT;
            }
        }
        else if (funct4c == 0b1001)
        {
            uint32_t v0 = (raw >> 2) & 0b11111;
            uint32_t v1 = (raw >> 7) & 0b11111;
            
            if (v0 == 0 && v1 == 0)
            {
                result->inst = EBREAK;
                result->fmt = SPECIAL;
            }
            else if (v0 == 0)
            {
                result->r_dest = 1;
                result->r_src1 = v1;

                result->inst = JALR;
                result->fmt = IFORMAT;
            }
            else
            {
                result->r_dest = v0;
                result->r_src1 = v0;
                result->r_src2 = v1;

                result->inst = ADD;
                result->fmt = RFORMAT;
            }
        }
        else if (funct3c == 0b110)
        {
            result->imm = ((raw >> 7) & 0b11) << 6;
            result->imm |= ((raw >> 9) & 0b1111) << 2;

            result->r_src1 = 2;
            result->r_src2 = (raw >> 2) & 0b11111;

            result->inst = SW;
            result->fmt = SFORMAT;
        }
        else if (funct3c == 0b111)
        {
            result->imm = ((raw >> 7) & 0b111) << 6;
            result->imm |= ((raw >> 10) & 0b111) << 3;

            result->r_src1 = 2;
            result->r_src2 = (raw >> 2) & 0b11111;

            result->inst = SD;
            result->fmt = SFORMAT;
        }
        else
        {
            printf("Value: %x\n", funct3c);
            assert(0 && "Not yet implemented 0b10");
        }
    }

    return 0;
}

// Convert a RISC-V register into a string
char* riscv_register_name(int i)
{
    switch (i & 31)
    {
        case 0:
            return "zero";
        case 1:
            return "ra";
        case 2:
            return "sp";
        case 3:
            return "gp";
        case 4:
            return "tp";
        case 5:
            return "t0";
        case 6:
            return "t1";
        case 7:
            return "t2";
        case 8:
            return "s0";
        case 9:
            return "s1";
        case 10:
            return "a0";
        case 11:
            return "a1";
        case 12:
            return "a2";
        case 13:
            return "a3";
        case 14:
            return "a4";
        case 15:
            return "a5";
        case 16:
            return "a6";
        case 17:
            return "a7";
        case 18:
            return "s2";
        case 19:
            return "s3";
        case 20:
            return "s4";
        case 21:
            return "s5";
        case 22:
            return "s6";
        case 23:
            return "s7";
        case 24:
            return "s8";
        case 25:
            return "s9";
        case 26:
            return "s10";
        case 27:
            return "s11";
        case 28:
            return "t3";
        case 29:
            return "t4";
        case 30:
            return "t5";
        case 31:
            return "t6";
    }
}

int addr_render(char* s, int64_t addr)
{
    sprintf(s, "(%lx)", addr);
    return 0;
}

// Convert a RISC-V instruction represented as a struct riscv_inst_repr into a string (give atleast 64) bytes of space for the string
int riscv_render_inst(char* s, struct riscv_inst_repr* inst, uint64_t addr)
{
    riscv_render_inst_symbols(s, inst, addr, addr_render);
}

int riscv_render_inst_symbols(char* s, struct riscv_inst_repr* inst, uint64_t addr, int (*render_addr)(char*, uint64_t))
{
    if(inst->inst == UNK)
    {
        sprintf(s, "unk");
        return 0;
    }

    char buf[64];

    switch (inst->fmt)
    {
        case IFORMAT:
            sprintf(s, "%s %s, %s, %i", riscv_inst_to_str(inst->inst), riscv_register_name(inst->r_dest), riscv_register_name(inst->r_src1), inst->imm);
            break;

        case RFORMAT:
            sprintf(s, "%s %s, %s, %s", riscv_inst_to_str(inst->inst), riscv_register_name(inst->r_dest), riscv_register_name(inst->r_src1), riscv_register_name(inst->r_src2));
            break;

        case SFORMAT:
            sprintf(s, "%s %s, %i(%s)", riscv_inst_to_str(inst->inst), riscv_register_name(inst->r_src2), inst->imm, riscv_register_name(inst->r_src1));
            break;
        
        case UFORMAT:
            sprintf(s, "%s %s, %i", riscv_inst_to_str(inst->inst), riscv_register_name(inst->r_dest), inst->imm);
            break;

        case BFORMAT:
            render_addr(buf, addr + (int64_t)(int32_t)inst->imm);
            sprintf(s, "%s %s, %s, %i %s", riscv_inst_to_str(inst->inst), riscv_register_name(inst->r_src1), riscv_register_name(inst->r_src2), inst->imm, buf);
            break;

        case JFORMAT:
            render_addr(buf, addr + (int64_t)(int32_t)inst->imm);
            sprintf(s, "%s %s, %i %s", riscv_inst_to_str(inst->inst), riscv_register_name(inst->r_dest), inst->imm, buf);
            break;
        
        default:
            sprintf(s, "%s imm: %x, rd: %i, rs1: %i, rs2: %i", riscv_inst_to_str(inst->inst), inst->imm, inst->r_dest, inst->r_src1, inst->r_src2);
            break;
    }

    return 0;
}