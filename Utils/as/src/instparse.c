#include "codegen.h"

#include <libc/assert.h>
#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <libc/string.h>

#include "parser.h"
#include "riscv.h"

#define ERROR_BUF_LEN 1024

static char ERROR_BUF[ERROR_BUF_LEN];

#define CONSUME_TOKEN(tokens) ((*tokens)++)

#define BUILD_ERROR_TEXT(pat, ...) ( sprintf(ERROR_BUF, pat, __VA_ARGS__), ERROR_BUF)

char* expect_identifier(struct Token** tokens, struct ParsingError* error);
char* expect_string(struct Token** tokens, struct ParsingError* error);
char* expect_instruction(struct Token** tokens, struct ParsingError* error);
int expect_register(struct Token** tokens, struct ParsingError* error);
bool expect_symbol(struct Token** tokens, struct ParsingError* error, char c);
bool expect_number(struct Token** tokens, struct ParsingError* error, size_t* number);

bool add_instruction(struct GenerationSettings* settings, struct ParsingError* error, struct Instruction* inst, Location);
bool parse_instruction(struct Token** tokens, struct GenerationSettings* settings, struct ParsingError* error);

bool parse_code(struct Token** tokens, struct GenerationSettings* settings, struct ParsingError* error)
{
    error->loc = (*tokens)[0].location;

    switch ((*tokens)[0].type)
    {
        // Handle the directives (.section, etc)
        case DIRECTIVE: 
            if (strcmp((tokens[0][0].data.str), ".section") == 0)
            {
                CONSUME_TOKEN(tokens);

                char* name = expect_identifier(tokens, error);
                if (name == 0) return false;
                CONSUME_TOKEN(tokens);

                settings_add_section(settings, name);
                
                return true;
            }
            else if (strcmp((tokens[0][0].data.str), ".align") == 0)
            {
                CONSUME_TOKEN(tokens);

                size_t number;
                if (!expect_number(tokens, error, &number)) return false;
                CONSUME_TOKEN(tokens);

                if (!settings_align(settings, number))
                {
                    error->error_text = "Unable to align undefined segment";
                    return false;
                }
                
                return true;
            }
            else if (strcmp((tokens[0][0].data.str), ".bytes") == 0)
            {
                CONSUME_TOKEN(tokens);

                while (1)
                {
                    if (tokens[0][0].type == NUMBER)
                    {
                        size_t number;
                        if (!expect_number(tokens, error, &number)) return false;
                        CONSUME_TOKEN(tokens);

                        if (!settings_add_to_current(settings, &number, 1))
                        {
                            error->error_text = "Unable to add data to undefined segment";
                            return false;
                        }
                    }
                    else if (tokens[0][0].type == STRING)
                    {
                        char* name = expect_string(tokens, error);
                        if (name == 0) return false;
                        CONSUME_TOKEN(tokens);

                        if (!settings_add_to_current(settings, name, strlen(name)))
                        {
                            error->error_text = "Unable to add data to undefined segment";
                            return false;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
                
                return true;
            }

            break;

        // Handle labels ([IDENTIFIER] [SYMBOL ':'])
        case IDENTIFIER:
            {
                char* name = expect_identifier(tokens, error);
                if (name == 0) return false;

                CONSUME_TOKEN(tokens);

                if (!expect_symbol(tokens, error, ':')) return false; 

                CONSUME_TOKEN(tokens);

                settings_add_label(settings, name, settings->current_addr);

                return true;
            }

            break;

        default:
            return parse_instruction(tokens, settings, error);
    }

    assert(0 && "Unreachable");
}


bool parse_rri(struct Token** tokens, struct Instruction* inst, struct ParsingError* error)
{
    inst->rdest = expect_register(tokens, error);
    CONSUME_TOKEN(tokens);
    if (inst->rdest > 31) return false;

    if (!expect_symbol(tokens, error, ','))
    {
        CONSUME_TOKEN(tokens);
        return false;
    }
    CONSUME_TOKEN(tokens);

    inst->rs1 = expect_register(tokens, error);
    CONSUME_TOKEN(tokens);
    if (inst->rs1 > 31) return false;

    if (!expect_symbol(tokens, error, ','))
    {
        CONSUME_TOKEN(tokens);
        return false;
    }
    CONSUME_TOKEN(tokens);

    if (!expect_number(tokens, error, &inst->imm))
    {
        CONSUME_TOKEN(tokens);
        return false;
    }
    CONSUME_TOKEN(tokens);

    return true;
}

bool parse_rrr(struct Token** tokens, struct Instruction* inst, struct ParsingError* error)
{
    inst->rdest = expect_register(tokens, error);
    CONSUME_TOKEN(tokens);
    if (inst->rdest > 31) return false;

    if (!expect_symbol(tokens, error, ','))
    {
        CONSUME_TOKEN(tokens);
        return false;
    }
    CONSUME_TOKEN(tokens);

    inst->rs1 = expect_register(tokens, error);
    CONSUME_TOKEN(tokens);
    if (inst->rs1 > 31) return false;

    if (!expect_symbol(tokens, error, ','))
    {
        CONSUME_TOKEN(tokens);
        return false;
    }
    CONSUME_TOKEN(tokens);

    inst->rs2 = expect_register(tokens, error);
    CONSUME_TOKEN(tokens);
    if (inst->rs2 > 31) return false;

    return true;
}

bool parse_rro(struct Token** tokens, struct Instruction* inst, struct ParsingError* error)
{
    inst->rs2 = expect_register(tokens, error);
    CONSUME_TOKEN(tokens);
    if (inst->rs2 > 31) return false;

    if (!expect_symbol(tokens, error, ','))
    {
        CONSUME_TOKEN(tokens);
        return false;
    }
    CONSUME_TOKEN(tokens);

    if (!expect_number(tokens, error, &inst->imm))
    {
        CONSUME_TOKEN(tokens);
        return false;
    }
    CONSUME_TOKEN(tokens);

    if (!expect_symbol(tokens, error, '('))
    {
        CONSUME_TOKEN(tokens);
        return false;
    }
    CONSUME_TOKEN(tokens);

    inst->rs1 = expect_register(tokens, error);
    CONSUME_TOKEN(tokens);
    if (inst->rs1 > 31) return false;
    
    if (!expect_symbol(tokens, error, ')'))
    {
        CONSUME_TOKEN(tokens);
        return false;
    }
    CONSUME_TOKEN(tokens);

    return true;
}

bool parse_rzi(struct Token** tokens, struct Instruction* inst, struct ParsingError* error)
{
    inst->rdest = expect_register(tokens, error);
    CONSUME_TOKEN(tokens);
    if (inst->rdest > 31) return false;

    if (!expect_symbol(tokens, error, ','))
    {
        CONSUME_TOKEN(tokens);
        return false;
    }
    CONSUME_TOKEN(tokens);

    inst->rs1 = 0;

    if (!expect_number(tokens, error, &inst->imm))
    {
        CONSUME_TOKEN(tokens);
        return false;
    }
    CONSUME_TOKEN(tokens);

    return true;
}

bool parse_rz_ia(struct Token** tokens, struct Instruction* inst, struct ParsingError* error, Location* loc)
{
    inst->rdest = expect_register(tokens, error);
    CONSUME_TOKEN(tokens);
    if (inst->rdest > 31) return false;

    if (!expect_symbol(tokens, error, ','))
    {
        CONSUME_TOKEN(tokens);
        return false;
    }
    CONSUME_TOKEN(tokens);

    inst->rs1 = 0;

    if (tokens[0][0].type == IDENTIFIER)
    {
        inst->link = expect_identifier(tokens, error);
        *loc = tokens[0][0].location;
        CONSUME_TOKEN(tokens);
        if (inst->link == 0) return false;
        inst->link_type = IMMEDIATE_LINK;
    }
    else
    {
        if (!expect_number(tokens, error, &inst->imm))
        {
            CONSUME_TOKEN(tokens);
            return false;
        }
        CONSUME_TOKEN(tokens);
    }
    

    return true;
}

bool parse_ra(struct Token** tokens, struct Instruction* inst, struct ParsingError* error, Location* loc)
{
    inst->rdest = expect_register(tokens, error);
    CONSUME_TOKEN(tokens);
    if (inst->rdest > 31) return false;

    if (!expect_symbol(tokens, error, ','))
    {
        CONSUME_TOKEN(tokens);
        return false;
    }
    CONSUME_TOKEN(tokens);

    inst->link = expect_identifier(tokens, error);
    *loc = tokens[0][0].location;
    CONSUME_TOKEN(tokens);
    if (inst->link == 0) return false;

    return true;
}

bool parse_rra(struct Token** tokens, struct Instruction* inst, struct ParsingError* error, Location* loc)
{
    inst->rs1 = expect_register(tokens, error);
    CONSUME_TOKEN(tokens);
    if (inst->rs1 > 31) return false;

    if (!expect_symbol(tokens, error, ','))
    {
        CONSUME_TOKEN(tokens);
        return false;
    }
    CONSUME_TOKEN(tokens);

    inst->rs2 = expect_register(tokens, error);
    CONSUME_TOKEN(tokens);
    if (inst->rs2 > 31) return false;

    if (!expect_symbol(tokens, error, ','))
    {
        CONSUME_TOKEN(tokens);
        return false;
    }
    CONSUME_TOKEN(tokens);

    inst->link = expect_identifier(tokens, error);
    *loc = tokens[0][0].location;
    CONSUME_TOKEN(tokens);
    if (inst->link == 0) return false;

    return true;
}

bool parse_za(struct Token** tokens, struct Instruction* inst, struct ParsingError* error, Location* loc)
{
    inst->rdest = 0;

    inst->link = expect_identifier(tokens, error);
    *loc = tokens[0][0].location;
    CONSUME_TOKEN(tokens);
    if (inst->link == 0) return false;

    return true;
}

bool parse_instruction(struct Token** tokens, struct GenerationSettings* settings, struct ParsingError* error)
{
    struct Instruction inst;
    Location identifier_loc;

    inst.link_type = NONE;

    #define RRI_COMMAND(s, INST) (strcmp(op, s) == 0) \
    { \
        inst.instruction = INST; \
         \
        if (!parse_rri(tokens, &inst, error)) return false; \
    }

    #define RRR_COMMAND(s, INST) (strcmp(op, s) == 0) \
    { \
        inst.instruction = INST; \
         \
        if (!parse_rrr(tokens, &inst, error)) return false; \
    }

    #define RRO_COMMAND(s, INST) (strcmp(op, s) == 0) \
    { \
        inst.instruction = INST; \
         \
        if (!parse_rro(tokens, &inst, error)) return false; \
    }

    #define BRANCH_COMMAND(s, INST) (strcmp(op, s) == 0) \
    { \
        inst.instruction = INST; \
        \
        if (!parse_rra(tokens, &inst, error, &identifier_loc)) return false; \
        inst.link_type = BRANCH_LINK; \
    }

    char* op = expect_instruction(tokens, error);
    CONSUME_TOKEN(tokens);
    if (op == 0) return false;

    if (strcmp(op, "ecall") == 0)
    {
        inst.instruction = ECALL;
    }
    else if RRI_COMMAND("addi", ADDI)
    else if RRI_COMMAND("slti", SLTI)
    else if RRI_COMMAND("sltiu", SLTIU)
    else if RRI_COMMAND("xori", XORI)
    else if RRI_COMMAND("ori", ORI)
    else if RRI_COMMAND("andi", ANDI)
    else if RRI_COMMAND("slli", SLLI)
    else if RRI_COMMAND("srli", SRLI)
    else if RRI_COMMAND("srai", SRAI)
    else if RRI_COMMAND("addiw", ADDIW)
    else if RRI_COMMAND("slliw", SLLIW)
    else if RRI_COMMAND("srliw", SRLIW)
    else if RRI_COMMAND("sraiw", SRAIW)
    else if RRR_COMMAND("add", ADD)
    else if RRR_COMMAND("sub", SUB)
    else if RRR_COMMAND("sll", SLL)
    else if RRR_COMMAND("slt", SLT)
    else if RRR_COMMAND("sltu", SLTU)
    else if RRR_COMMAND("xor", XOR)
    else if RRR_COMMAND("srl", SRL)
    else if RRR_COMMAND("sra", SRA)
    else if RRR_COMMAND("or", OR)
    else if RRR_COMMAND("and", AND)
    else if RRR_COMMAND("addw", ADDW)
    else if RRR_COMMAND("subw", SUBW)
    else if RRR_COMMAND("sllw", SLLW)
    else if RRR_COMMAND("srlw", SRLW)
    else if RRR_COMMAND("sraw", SRAW)
    else if RRI_COMMAND("lb", LB)
    else if RRI_COMMAND("lh", LH)
    else if RRI_COMMAND("lw", LW)
    else if RRI_COMMAND("lbu", LBU)
    else if RRI_COMMAND("lhu", LHU)
    else if RRI_COMMAND("lwu", LWU)
    else if RRI_COMMAND("ld", LD)
    else if RRO_COMMAND("sb", SB)
    else if RRO_COMMAND("sh", SH)
    else if RRO_COMMAND("sw", SW)
    else if RRO_COMMAND("sd", SD)
    else if BRANCH_COMMAND("beq", BEQ)
    else if BRANCH_COMMAND("bne", BNE)
    else if BRANCH_COMMAND("blt", BLT)
    else if BRANCH_COMMAND("bge", BGE)
    else if BRANCH_COMMAND("bltu", BLTU)
    else if BRANCH_COMMAND("bgeu", BGEU)
    else if RRR_COMMAND("mul", MUL)
    else if RRR_COMMAND("mulh", MULH)
    else if RRR_COMMAND("mulhsu", MULHSU)
    else if RRR_COMMAND("mulhu", MULHU)
    else if RRR_COMMAND("div", DIV)
    else if RRR_COMMAND("divu", DIVU)
    else if RRR_COMMAND("rem", REM)
    else if RRR_COMMAND("remu", REMU)
    else if RRR_COMMAND("mulw", MULW)
    else if RRR_COMMAND("divw", DIVW)
    else if RRR_COMMAND("divuw", DIVUW)
    else if RRR_COMMAND("remw", REMW)
    else if RRR_COMMAND("remuw", REMUW)
    else if (strcmp(op, "li") == 0)
    {
        inst.instruction = ADDI;

        if (!parse_rzi(tokens, &inst, error)) return false;
    }
    else if (strcmp(op, "la") == 0)
    {
        inst.instruction = LUI;
        inst.imm = 0;

        if (!parse_ra(tokens, &inst, error, &identifier_loc)) return false;

        inst.link_type = UPPER_IMMEDIATE_LINK;

        if (!add_instruction(settings, error, &inst, identifier_loc)) return false;

        inst.rs1 = inst.rdest;
        inst.link_type = IMMEDIATE_LINK;

        inst.instruction = ADDI;
    }
    else if (strcmp(op, "jal") == 0)
    {
        inst.instruction = JAL;
        
        if (!parse_ra(tokens, &inst, error, &identifier_loc)) return false;
        inst.link_type = JUMP_LINK;
    }
    else if (strcmp(op, "jalr") == 0)
    {
        inst.instruction = JALR;
        
        if (!parse_rri(tokens, &inst, error)) return false;
    }
    else if (strcmp(op, "j") == 0)
    {
        inst.instruction = JAL;
        
        if (!parse_za(tokens, &inst, error, &identifier_loc)) return false;
        inst.link_type = JUMP_LINK;
    }
    else if (strcmp(op, "call") == 0)
    {
        inst.instruction = JAL;
        
        if (!parse_za(tokens, &inst, error, &identifier_loc)) return false;

        inst.rdest = 1;

        inst.link_type = JUMP_LINK;
    }
    else if (strcmp(op, "push") == 0)
    {
        inst.instruction = ADDI;

        inst.rs1 = 2;
        inst.rdest = 2;
        inst.imm = -8;
        add_instruction(settings, error, &inst, identifier_loc);

        inst.instruction = SD;
        
        inst.rs2 = expect_register(tokens, error);
        CONSUME_TOKEN(tokens);
        if (inst.rs2 > 31) return false;

        inst.rs1 = 2;
        inst.imm = 0;
    }
    else if (strcmp(op, "pop") == 0)
    {
        inst.instruction = LD;
        
        inst.rdest = expect_register(tokens, error);
        CONSUME_TOKEN(tokens);
        if (inst.rdest > 31) return false;

        inst.rs1 = 2;
        inst.imm = 0;
        add_instruction(settings, error, &inst, identifier_loc);

        inst.instruction = ADDI;

        inst.rs1 = 2;
        inst.rdest = 2;
        inst.imm = 8;
    }
    else if (strcmp(op, "ret") == 0)
    {
        inst.instruction = JALR;

        inst.rdest = 0;
        inst.rs1 = 1;
    }
    else
    {
        printf("Unhandled operation `%s`\n", op);
        assert(0);
    }

    return add_instruction(settings, error, &inst, identifier_loc);
}
 
char* expect_identifier(struct Token** tokens, struct ParsingError* error)
{
    if ((*tokens)[0].type == IDENTIFIER)
    {
        return tokens[0][0].data.str;
    }
    else
    {
        error->loc = tokens[0][0].location;
        error->error_text = BUILD_ERROR_TEXT("Expected Identifier, got `%s`", render_token(tokens[0]));
        return 0;
    }
}

char* expect_string(struct Token** tokens, struct ParsingError* error)
{
    if ((*tokens)[0].type == STRING)
    {
        return tokens[0][0].data.str;
    }
    else
    {
        error->loc = tokens[0][0].location;
        error->error_text = BUILD_ERROR_TEXT("Expected String, got `%s`", render_token(tokens[0]));
        return 0;
    }
}

char* expect_instruction(struct Token** tokens, struct ParsingError* error)
{
    if ((*tokens)[0].type == INSTRUCTION)
    {
        return tokens[0][0].data.str;
    }
    else
    {
        error->loc = tokens[0][0].location;
        error->error_text = BUILD_ERROR_TEXT("Expected Instruction, got `%s`", render_token(tokens[0]));
        return 0;
    }
}

int expect_register(struct Token** tokens, struct ParsingError* error)
{
    if ((*tokens)[0].type == REGISTER)
    {
        return tokens[0][0].data.num;
    }
    else
    {
        error->loc = tokens[0][0].location;
        error->error_text = BUILD_ERROR_TEXT("Expected Register, got `%s`", render_token(tokens[0]));
        return -1;
    }
}

bool expect_symbol(struct Token** tokens, struct ParsingError* error, char c)
{
    if ((*tokens)[0].type == SYMBOL)
    {
        if (tokens[0][0].data.c == c) return true;
    }

    error->loc = tokens[0][0].location;
    error->error_text = BUILD_ERROR_TEXT("Expected `%c`, got `%s`", c, render_token(tokens[0]));

    return false;
}

bool expect_number(struct Token** tokens, struct ParsingError* error, size_t* number)
{
    if ((*tokens)[0].type == NUMBER)
    {
        *number = tokens[0][0].data.num;
        return true;
    }
    else
    {
        error->loc = tokens[0][0].location;
        error->error_text = BUILD_ERROR_TEXT("Expected Number, got `%s`", render_token(tokens[0]));
        return false;
    }
}

bool add_instruction(struct GenerationSettings* settings, struct ParsingError* error, struct Instruction* inst, Location loc)
{
    if (!settings_add_instruction(settings, inst, loc))
    {
        error->error_text = "Cannot add instruction to nonexistant section, try adding a section directive";
        return false;
    }

    return true;
}