#ifndef DATABASE_H
#define DATABASE_H

#include <stdbool.h>

#include <elf/libelf.h>

#include "as.h"
#include "vector.h"

struct symbol_database_entry
{
    char* name;
    char* file;
    char* section;
    Elf64_Sym symbol;
    bool defined;
};

struct relocation_database_entry
{
    char* symbol_name;

    Elf64_Rela relocation;
};

struct section_database_entry
{
    char* file;
    char* name;
    uint32_t index_in_elf;
    Elf64_Shdr section;
    void* ptr;

    struct vector relocation_database;
};

extern struct vector symbol_database;
extern struct vector section_database;

int register_elf_buffer(uint8_t* buffer, const char* filename);
struct symbol_database_entry* find_symbol(const char* symbol_name);
struct section_database_entry* associated_section(struct symbol_database_entry symbol);

void dump_symbol_database();
void dump_section_database();
void dump_relocation_database();

#endif // DATABASE_H