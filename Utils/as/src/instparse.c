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
    if (inst->rdest < 0) return false;

    if (!expect_symbol(tokens, error, ','))
    {
        CONSUME_TOKEN(tokens);
        return false;
    }
    CONSUME_TOKEN(tokens);

    inst->rs1 = expect_register(tokens, error);
    CONSUME_TOKEN(tokens);
    if (inst->rs1 < 0) return false;

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

bool parse_rzi(struct Token** tokens, struct Instruction* inst, struct ParsingError* error)
{
    inst->rdest = expect_register(tokens, error);
    CONSUME_TOKEN(tokens);
    if (inst->rdest < 0) return false;

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

bool parse_ra(struct Token** tokens, struct Instruction* inst, struct ParsingError* error, Location* loc)
{
    inst->rdest = expect_register(tokens, error);
    CONSUME_TOKEN(tokens);
    if (inst->rdest < 0) return false;

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

    inst.j_link = false;
    inst.b_link = false;

    char* op = expect_instruction(tokens, error);
    CONSUME_TOKEN(tokens);
    if (op == 0) return false;

    if (strcmp(op, "ecall") == 0)
    {
        inst.instruction = ECALL;
    }
    else if (strcmp(op, "addi") == 0)
    {
        inst.instruction = ADDI;
        
        if (!parse_rri(tokens, &inst, error)) return false;
    }
    else if (strcmp(op, "li") == 0)
    {
        inst.instruction = ADDI;
        
        if (!parse_rzi(tokens, &inst, error)) return false;
    }
    else if (strcmp(op, "jal") == 0)
    {
        inst.instruction = JAL;
        
        if (!parse_ra(tokens, &inst, error, &identifier_loc)) return false;
        inst.j_link = true;
    }
    else if (strcmp(op, "j") == 0)
    {
        inst.instruction = JAL;
        
        if (!parse_za(tokens, &inst, error, &identifier_loc)) return false;
        inst.j_link = true;
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
        return 0;
    }
}

bool expect_symbol(struct Token** tokens, struct ParsingError* error, char c)
{
    if ((*tokens)[0].type == SYMBOL)
    {
        if (tokens[0][0].data.c == c) return true;
    }

    error->loc = tokens[0][0].location;
    error->error_text = BUILD_ERROR_TEXT("Expected `:`, got `%s`", render_token(tokens[0]));

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