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

extern struct vector symbol_database;

int register_elf_buffer(uint8_t* buffer, const char* filename);
struct symbol_database_entry* find_symbol(const char* symbol_name);

void dump_symbol_database();

#endif // DATABASE_H