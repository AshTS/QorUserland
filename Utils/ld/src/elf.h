#ifndef ELF_OUTPUT_H
#define ELF_OUTPUT_H

#include "vector.h"
#include "elf/libelf.h"

#include "database.h"

struct section_data
{
    char* name;
    struct vector buffer;
    int type;
    int flags;
    int info;
    int addralign;
    int entsize;
    uint64_t offset; // Offset into the ELF file, this is written as the elf file is being generated and is only used in program header generation
    uint64_t address;
};

struct symbol_data
{
    char* name;
    uint64_t expanded_value;
    Elf64_Sym data;
};

struct section_data create_section_data(char* name);

struct vector construct_elf_file(struct vector sections, struct vector symbols, uint64_t start_symbol_offset);
int link(struct vector* elf_buffer);
int apply_relocation(void* data, struct symbol_data* symbol, struct relocation_database_entry relocation, uint64_t current_section_addr, struct vector* relocations);

#endif // ELF_OUTPUT_H