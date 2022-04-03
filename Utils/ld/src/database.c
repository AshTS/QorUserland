#include "database.h"

#include <libc/stdio.h>
#include <elf/libelf.h>

#include "as.h"

struct vector symbol_database = VECTOR(struct symbol_database_entry);

int register_elf_buffer(uint8_t* buffer, const char* filename)
{
    LOG("Registering symbols from elf file in buffer for file %s\n", filename);

    Elf64_Ehdr* header = buffer;

    // Verify the ELF header
    int elf_verify_result = elf_verify(header);
    if (elf_verify_result)
    {
        printf("ld: %s is not an ELF file: %s\n", filename, elf_strerror(elf_verify_result));
        return 1;
    }

    // Find the symbol table
    Elf64_Shdr* symtab;
    bool found_symbols = false;

    for (int i = 0; i < header->e_shnum; i++)
    {
        Elf64_Shdr* sh = elf_get_sh(header, i);

        if (sh->sh_type == SHT_SYMTAB)
        {
            symtab = sh;
            found_symbols = true;
            break;
        }
    }

    // If the symbol table was found, copy the symbols
    if (found_symbols)
    {
        Elf64_Sym* symbols = buffer + symtab->sh_offset;
        Elf64_Shdr* strheader = elf_get_sh(header, symtab->sh_link);
        for (int i = 1; i < symtab->sh_size / symtab->sh_entsize; i++)
        {
            char* name = elf_get_string(header, symbols[i].st_name);
            Elf64_Shdr* sh = elf_get_sh(header, symbols[i].st_shndx);
            char* section_name = elf_get_section_name(header, sh->sh_name);
            
            struct symbol_database_entry entry = (struct symbol_database_entry){
                .name = name, 
                .defined = symbols[i].st_shndx > 0, 
                .symbol=symbols[i],
                .file = filename,
                .section = section_name};

            struct symbol_database_entry* prev_entry = find_symbol((const char*)name);

            if (prev_entry == NULL)
            {
                vector_append_ptr(&symbol_database, &entry);
            }
            else
            {
                if (prev_entry->defined && entry.defined)
                {
                    printf("ld: Symbol %s redefined in %s\n", name, filename);
                    return 1;
                }
                else if ((prev_entry->defined == false) && (entry.defined == true))
                {
                    *prev_entry = entry;
                }
            }
        }
    }

    return 0;
}

struct symbol_database_entry* find_symbol(const char* symbol_name)
{
    struct symbol_database_entry* array = VEC_TO_ARRAY(symbol_database, struct symbol_database_entry);

    for (int i = 0; i < symbol_database.length; i++)
    {
        if (strcmp(array[i].name, symbol_name) == 0)
        {
            return &array[i];
        }
    }

    return NULL;
}

void dump_symbol_database()
{
    struct symbol_database_entry* array = VEC_TO_ARRAY(symbol_database, struct symbol_database_entry);

    printf("  Index    Name            Defined Value            Other            Size  File      Section\n");

    for (int i = 0; i < symbol_database.length; i++)
    {
        printf("  %-5i    %-15s %c       %#-016lx %#-016lx %-5ld %-10s%s\n", 
            i, array[i].name, 
            array[i].defined ? 'D' : 'U',
            array[i].symbol.st_value,
            array[i].symbol.st_other,
            array[i].symbol.st_size,
            array[i].file,
            array[i].section);
    }
}