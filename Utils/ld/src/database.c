#include "database.h"

#include <libc/stdio.h>
#include <elf/libelf.h>

#include "as.h"

struct vector symbol_database = VECTOR(struct symbol_database_entry);
struct vector section_database = VECTOR(struct section_database_entry);

int register_elf_symbols(uint8_t* buffer, const char* filename);
int register_elf_sections(uint8_t* buffer, const char* filename);
int register_elf_relocations(uint8_t* buffer, const char* filename);

int register_elf_buffer(uint8_t* buffer, const char* filename)
{
    int r = register_elf_sections(buffer, filename);
    if (r) return r;

    r = register_elf_symbols(buffer, filename);
    if (r) return r;

    r = register_elf_relocations(buffer, filename);
    if (r) return r;
}

int register_elf_symbols(uint8_t* buffer, const char* filename)
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

            printf("Symbol %s in section %s\n", name, section_name);
            
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


int register_elf_sections(uint8_t* buffer, const char* filename)
{
    LOG("Registering sections from elf file in buffer for file %s\n", filename);

    Elf64_Ehdr* header = buffer;

    // Verify the ELF header
    int elf_verify_result = elf_verify(header);
    if (elf_verify_result)
    {
        printf("ld: %s is not an ELF file: %s\n", filename, elf_strerror(elf_verify_result));
        return 1;
    }

    // Iterate over all of the sections
    for (int i = 1; i < header->e_shnum; i++)
    {
        Elf64_Shdr* section_header = elf_get_sh(header, i);

        if (section_header == NULL)
        {
            printf("ld: %s is malformed: not enough space for section headers\n", filename);
            return 1;
        }

        // Ignore any non SHT_PROGBITS sections
        if (section_header->sh_type != SHT_PROGBITS)
        {
            continue;
        }

        char* name = elf_get_section_name(header, section_header->sh_name);

        struct section_database_entry entry = (struct section_database_entry){
            .file = filename,
            .name = name,
            .ptr = buffer + section_header->sh_offset,
            .relocation_database = VECTOR(struct relocation_database_entry),
            .section = *section_header,
            .index_in_elf = i,
        };

        vector_append_ptr(&section_database, &entry);
    }

    return 0;
}


int register_elf_relocations(uint8_t* buffer, const char* filename)
{
    LOG("Registering relocations from elf file in buffer for file %s\n", filename);

    Elf64_Ehdr* header = buffer;

    // Verify the ELF header
    int elf_verify_result = elf_verify(header);
    if (elf_verify_result)
    {
        printf("ld: %s is not an ELF file: %s\n", filename, elf_strerror(elf_verify_result));
        return 1;
    }

    // Iterate over all of the sections
    for (int i = 1; i < header->e_shnum; i++)
    {
        Elf64_Shdr* section_header = elf_get_sh(header, i);

        if (section_header == NULL)
        {
            printf("ld: %s is malformed: not enough space for section headers\n", filename);
            return 1;
        }

        // Ignore any sections which are not SHT_RELA
        if (section_header->sh_type != SHT_RELA)
        {
            continue;
        }

        // Find the section which this section has relocations to
        struct section_database_entry* sa = VEC_TO_ARRAY(section_database, struct section_database_entry);
        struct section_database_entry* referenced = NULL;
        for (int j = 0; j < section_database.length; j++)
        {
            if (sa[j].index_in_elf != section_header->sh_info)
            {
                continue;
            }

            if (strcmp(sa[j].file, filename) == 0)
            {
                referenced = &sa[j];
                break;
            }
        }

        char* name = elf_get_section_name(header, section_header->sh_name);

        // If the section refered to doesn't exist, we have a problem
        if (referenced == NULL)
        {
            printf("ld: Section %s in %s doesn't refer to a known section\n", name, filename);
            return 1;
        }

        // Get a reference to the section for symbols
        Elf64_Shdr* symbol_table = elf_get_sh(header, section_header->sh_link);
        if (symbol_table == NULL)
        {
            printf("ld: Section %s in %s doesn't refer to a known symbol table\n", name, filename);
            return 1;
        }

        Elf64_Sym* symbols = buffer + symbol_table->sh_offset;

        // Get the pointer to the array of relocations
        Elf64_Rela* relocation_array = buffer + section_header->sh_offset;

        // Otherwise, we will go through every relocation, and append it to the array
        for (int j = 0; j < section_header->sh_size / section_header->sh_entsize; j++)
        {
            Elf64_Sym symbol = symbols[relocation_array[j].r_info >> 32];
            char* symbol_name = elf_get_string(header, symbol.st_name);

            if (symbol_name == NULL)
            {
                symbol_table = "";
            }

            struct relocation_database_entry entry = (struct relocation_database_entry)
                {
                    .relocation = relocation_array[j],
                    .symbol_name = symbol_name
                };

            vector_append_ptr(&referenced->relocation_database, &entry);
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

struct section_database_entry* associated_section(struct symbol_database_entry symbol)
{
    struct section_database_entry* array = VEC_TO_ARRAY(section_database, struct section_database_entry);

    for (int i = 0; i < symbol_database.length; i++)
    {
        if (strcmp(array[i].name, symbol.section) == 0 && strcmp(array[i].file, symbol.file) == 0)
        {
            return &array[i];
        }
    }

    return NULL;
}

void dump_symbol_database()
{
    struct symbol_database_entry* array = VEC_TO_ARRAY(symbol_database, struct symbol_database_entry);

    printf("Symbols: \n");
    printf("  Index    Name            Defined Value              Other            Size  File      Section\n");

    for (int i = 0; i < symbol_database.length; i++)
    {
        printf("  %-5i    %-15s %c       0x%016lx 0x%016lx %-5ld %-10s%s\n", 
            i, array[i].name, 
            array[i].defined ? 'D' : 'U',
            array[i].symbol.st_value,
            array[i].symbol.st_other,
            array[i].symbol.st_size,
            array[i].file,
            array[i].section);
    }
}

void dump_section_database()
{
    struct section_database_entry* array = VEC_TO_ARRAY(section_database, struct section_database_entry);

    printf("Sections: \n");
    printf("  Index    File       Name\n");

    for (int i = 0; i < section_database.length; i++)
    {
        printf("  %-5i    %-10s  %-10s\n", 
            i, array[i].file, array[i].name);
    }
}

void dump_relocation_database()
{
    struct section_database_entry* array = VEC_TO_ARRAY(section_database, struct section_database_entry);

    printf("Relocations: \n");

    // First, iterate over all of the sections
    for (int i = 0; i < section_database.length; i++)
    {
        if (array[i].relocation_database.length == 0)
        {
            continue;
        }

        printf(" %s %s:\n", array[i].file, array[i].name);
        printf("    Name        Type  Value              Addend\n");

        struct relocation_database_entry* relocs = VEC_TO_ARRAY(array[i].relocation_database, struct relocation_database_entry);
        for (int j = 0; j < array[i].relocation_database.length; j++)
        {
            printf("    %-10s  %-4i  0x%015lx  0x%015lx\n", relocs[j].symbol_name, relocs[j].relocation.r_info & 0xFFFF, relocs[j].relocation.r_offset, relocs[j].relocation.r_addend);
        }
    }
}