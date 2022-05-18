#ifndef ASSEMBLE_H
#define ASSEMBLE_H

#include <libc/stdio.h>
#include <elf/libelf.h>
#include <riscv/riscv.h>
#include "tokenizer.h"
#include "vector.h"

struct section_data
{
    char* name;
    struct vector buffer;
    int type;
    int flags;
    int info;
    int addralign;
    int entsize;
};

struct symbol_data
{
    char* name;
    Elf64_Sym data;
};

struct relocation_data
{
    char* name;
    Elf64_Rela data;
};

struct relocation_group
{
    char* name;
    uint32_t link;
    struct vector relocations; // struct relocation_data
};

// Add an instruction to the current buffer
void add_instruction(struct vector* byte_buffer, struct riscv_inst_repr inst);

// Assemble a file from the file handle
void assemble_file_handle(FILE* file, const char* input_name, const char* output_name);

void place_symbol_at(struct vector sections, size_t section_index, struct symbol_data* symbol);

// Assemble an instruction and write out to the section data
int assemble_instruction(struct token* tokens, int* index, struct vector* byte_buffer, struct vector* relocation, struct vector* symbols, struct vector sections, size_t section_index);

#endif // ASSEMBLE_H