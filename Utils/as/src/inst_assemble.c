#include "assemble.h"

#include "as.h"

#include "libc/string.h"

size_t NEXT_UNIQUE_IDENTIFIER_INDEX = 0;

// Add a relocation
void add_relocation(uint64_t offset, char* name, uint32_t type, uint64_t addend, struct vector* relocations)
{
    uint64_t value = type;

    struct relocation_data relocation = (struct relocation_data){.name = name, .data=(Elf64_Rela){.r_addend = addend, .r_info=value, .r_offset=offset}};

    vector_append_ptr(relocations, &relocation);
}

// Create a new symbol at the current location in the current section
struct symbol_data place_new_symbol(char* name, struct vector sections, size_t section_index, struct vector* symbols)
{
    struct symbol_data symbol = (struct symbol_data){
        .name = strdup(name), 
        .data = (Elf64_Sym){.st_info = 0, .st_name = 0, .st_other = 0, .st_shndx = 0, .st_size = 0, .st_value = 0}};

    place_symbol_at(sections, section_index, &symbol);
    vector_append_ptr(symbols, &symbol);

    LOG("Adding symbol %s has value %#lx\n", name, symbol.data.st_value);
}

// Create a new PC Relative High symbol
char* place_pcrel_hi(struct vector sections, size_t section_index, struct vector* symbols)
{
    char buffer[256];

    sprintf(buffer, ".Lpcrel_hi%lu", NEXT_UNIQUE_IDENTIFIER_INDEX++);

    place_new_symbol(buffer, sections, section_index, symbols);

    return strdup(buffer);
}

// Add an instruction to the current buffer
void add_instruction(struct vector* byte_buffer, struct riscv_inst_repr inst)
{
    char buffer[128];
    riscv_render_inst(buffer, &inst, 0);
    printf("Adding Instruction: %s\n", buffer);

    // Compile the instruction
    uint32_t inst_data = riscv_assemble(inst);

    // Write the instruction to the buffer
    vector_append_buffer(byte_buffer, &inst_data, 4);
}

// Print an error if the current token is an EOF
int ensure_not_eof(struct token* tokens, int* index)
{
    char buffer[128];

    if (tokens[*index].type == EOF)
    {
        printf("Unexpected EOF while parsing at %s\n", render_location(buffer, tokens[*index].location));
        return 1;
    }

    return 0;
}

// Expect a certain symbol from a token stream
int expect_symbol(struct token* tokens, int* index, char symbol)
{
    char buffer[128];
    char buffer2[128];

    ensure_not_eof(tokens, index);

    if (tokens[*index].type == TOK_SYMBOL && tokens[*index].data.character == symbol)
    {
        (*index)++;
        return 0;
    }
    else
    {
        printf("Expected symbol `%c`, got `%s` at %s\n", symbol, render_token(buffer, tokens[*index]), render_location(buffer2, tokens[*index].location));
        return 1;
    }
}

// Parse an immediate value
int64_t parse_immediate(struct token* tokens, int* index)
{
    char buffer[128];
    char buffer2[128];

    ensure_not_eof(tokens, index);

    if (tokens[*index].type == TOK_NUMBER)
    {
        int64_t number = tokens[*index].data.number;
        (*index)++;

        return number;
    }

    printf("Expected number, got `%s` at %s\n", render_token(buffer, tokens[*index]), render_location(buffer2, tokens[*index].location));

    return 0;
}

// Parse an identifier
char* parse_identifier(struct token* tokens, int* index)
{
    char buffer[128];
    char buffer2[128];

    ensure_not_eof(tokens, index);

    if (tokens[*index].type == TOK_IDENTIFIER)
    {
        char* string = tokens[*index].data.string;
        (*index)++;

        return string;
    }

    printf("Expected identifier, got `%s` at %s\n", render_token(buffer, tokens[*index]), render_location(buffer2, tokens[*index].location));

    return 0;
}

// Parse a register from the token stream
#define CASE(name0, name1, number) if (strcmp(tokens[*index].data.string, name0) == 0 || strcmp(tokens[*index].data.string, name1) == 0) { (*index)++; return number; }
int parse_register(struct token* tokens, int* index) 
{
    char buffer[128];
    char buffer2[128];

    ensure_not_eof(tokens, index);

    if (tokens[*index].type == TOK_IDENTIFIER)
    {
        CASE("zero", "x0", 0)
        else CASE("ra", "x1", 1)
        else CASE("sp", "x2", 2)
        else CASE("gp", "x3", 3)
        else CASE("tp", "x4", 4)
        else CASE("t0", "x5", 5)
        else CASE("t1", "x6", 6)
        else CASE("t2", "x7", 7)
        else CASE("s0", "x8", 8)
        else CASE("s1", "x9", 9)
        else CASE("a0", "x10", 10)
        else CASE("a1", "x11", 11)
        else CASE("a2", "x12", 12)
        else CASE("a3", "x13", 13)
        else CASE("a4", "x14", 14)
        else CASE("a5", "x15", 15)
        else CASE("a6", "x16", 16)
        else CASE("a7", "x17", 17)
        else CASE("s2", "x18", 18)
        else CASE("s3", "x19", 19)
        else CASE("s4", "x20", 20)
        else CASE("s5", "x21", 21)
        else CASE("s6", "x22", 22)
        else CASE("s7", "x23", 23)
        else CASE("s8", "x24", 24)
        else CASE("s9", "x25", 25)
        else CASE("s10", "x26", 26)
        else CASE("s11", "x27", 27)
        else CASE("t3", "x28", 28)
        else CASE("t4", "x29", 29)
        else CASE("t5", "x30", 30)
        else CASE("t6", "x31", 31)
    }

    printf("Expected register, got `%s` at %s\n", render_token(buffer, tokens[*index]), render_location(buffer2, tokens[*index].location));
    return -1;
}
#undef CASE

// Assemble an instruction and write out to the section
int assemble_instruction(struct token* tokens, int* index, struct vector* byte_buffer, struct vector* relocations, struct vector* symbols, struct vector sections, size_t section_index)
{
    char buffer[128];
    char buffer2[128];

    if (tokens[*index].type == TOK_EOF)
    {
        (*index)++;
        return 0;
    }
    else if (tokens[*index].type != TOK_IDENTIFIER)
    {
        printf("Expected Identifier, got `%s` at %s\n", render_token(buffer, tokens[*index]), render_location(buffer2, tokens[*index].location));
        (*index)++;
    }
    // addi instruction parsing
    else if (strcmp(tokens[*index].data.string, "addi") == 0)
    {
        (*index)++;

        int rd = parse_register(tokens, index);
        expect_symbol(tokens, index, ',');
        int rs1 = parse_register(tokens, index);
        expect_symbol(tokens, index, ',');
        int64_t imm = parse_immediate(tokens, index);

        add_instruction(byte_buffer, 
            (struct riscv_inst_repr){.inst = ADDI, .r_dest = rd, .r_src1=rs1, .imm=imm, .fmt=IFORMAT}
        );
    }
    // li instruction parsing
    else if (strcmp(tokens[*index].data.string, "li") == 0)
    {
        (*index)++;

        int rd = parse_register(tokens, index);
        expect_symbol(tokens, index, ',');
        int64_t imm = parse_immediate(tokens, index);

        add_instruction(byte_buffer, 
            (struct riscv_inst_repr){.inst = ADDI, .r_dest = rd, .r_src1=0, .imm=imm, .fmt=IFORMAT}
        );
    }
    // ecall instruction parsing
    else if (strcmp(tokens[*index].data.string, "ecall") == 0)
    {
        (*index)++;

        add_instruction(byte_buffer, 
            (struct riscv_inst_repr){.inst = ECALL, .fmt=SPECIAL}
        );
    }
    // sd instruction parsing
    else if (strcmp(tokens[*index].data.string, "sd") == 0)
    {
        (*index)++;

        int rs2 = parse_register(tokens, index);
        expect_symbol(tokens, index, ',');
        int64_t imm = parse_immediate(tokens, index);
        expect_symbol(tokens, index, '(');
        int rs1 = parse_register(tokens, index);
        expect_symbol(tokens, index, ')');

        add_instruction(byte_buffer, 
            (struct riscv_inst_repr){.inst = SD, .r_src1 = rs1, .r_src2=rs2, .imm=imm, .fmt=SFORMAT}
        );
    }
    // ld instruction parsing
    else if (strcmp(tokens[*index].data.string, "ld") == 0)
    {
        (*index)++;

        int rd = parse_register(tokens, index);
        expect_symbol(tokens, index, ',');
        int64_t imm = parse_immediate(tokens, index);
        expect_symbol(tokens, index, '(');
        int rs1 = parse_register(tokens, index);
        expect_symbol(tokens, index, ')');

        add_instruction(byte_buffer, 
            (struct riscv_inst_repr){.inst = LD, .r_dest = rd, .r_src1=rs1, .imm=imm, .fmt=IFORMAT}
        );
    }
    // la instruction parsing
    else if (strcmp(tokens[*index].data.string, "la") == 0)
    {
        (*index)++;

        int rd = parse_register(tokens, index);
        expect_symbol(tokens, index, ',');
        char* identifier = parse_identifier(tokens, index);

        char* pcrel_symbol = place_pcrel_hi(sections, section_index, symbols);

        add_relocation(byte_buffer->length, identifier, R_RISCV_PCREL_HI20, 0, relocations);

        add_instruction(byte_buffer, 
            (struct riscv_inst_repr){.inst = AUIPC, .r_dest = rd, .imm=0, .fmt=UFORMAT}
        );

        add_relocation(byte_buffer->length, pcrel_symbol, R_RISCV_PCREL_LO12_I, 0, relocations);

        add_instruction(byte_buffer, 
            (struct riscv_inst_repr){.inst = ADDI, .r_dest = rd, .r_src1=rd, .imm=0, .fmt=IFORMAT}
        );
    }
    // call instruction parsing
    else if (strcmp(tokens[*index].data.string, "call") == 0)
    {
        (*index)++;

        char* identifier = parse_identifier(tokens, index);

        add_relocation(byte_buffer->length, identifier, R_RISCV_CALL, 0, relocations);

        // auipc x6, offset[31:12]
        add_instruction(byte_buffer, 
            (struct riscv_inst_repr){.inst = AUIPC, .r_dest = 6, .imm=0, .fmt=UFORMAT}
        );

        // jalr x1, x6, offset[11:0]
        add_instruction(byte_buffer, 
            (struct riscv_inst_repr){.inst = JALR, .r_dest = 1, .r_src1=6, .imm=0, .fmt=IFORMAT}
        );
    }
    // ret instruction parsing
    else if (strcmp(tokens[*index].data.string, "ret") == 0)
    {
        (*index)++;

        add_instruction(byte_buffer, 
            (struct riscv_inst_repr){.inst = JALR, .r_dest = 0, .r_src1=1, .imm=0, .fmt=IFORMAT}
        );
    }
    
    else
    {
        printf("Unknown instruction `%s` at %s\n", render_token(buffer, tokens[*index]), render_location(buffer2, tokens[*index].location));
        (*index)++;
    }

    return 0;
}