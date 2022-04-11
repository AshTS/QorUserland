#include "elf.h"

#include "database.h"

static uint64_t START_ADDR = 0x1000000;

int link(struct vector* elf_buffer)
{
    struct vector sections = VECTOR(struct section_data);
    struct vector symbols = VECTOR(struct symbol_data);

    // Get the _start symbol
    struct symbol_database_entry* start_symbol = find_symbol("_start");
    if (start_symbol == NULL || !start_symbol->defined)
    {
        printf("ld: No _start symbol defined\n");
        return 1;
    }

    // Get the section associated with the _start symbol
    struct section_database_entry* start_section = associated_section(*start_symbol);
    if (start_section == NULL)
    {
        printf("ld: No section associated with _start symbol\n");
        return 1;
    }

    printf("Section: %s %s\n", start_section->file, start_section->name);

    // Store the offset for the _start symbol
    uint64_t start_symbol_offset = start_symbol->symbol.st_value;

    printf("_start Offset: 0x%lx\n", start_symbol_offset);

    struct section_data start_section_data = create_section_data(".text");

    start_section_data.type = SHT_PROGBITS;
    start_section_data.flags = SHF_ALLOC | SHF_EXECINSTR;
    start_section_data.address = START_ADDR;
    start_section_data.addralign = 4;

    vector_append_buffer(&start_section_data.buffer, start_section->ptr, start_section->section.sh_size);

    printf("%lu\n", start_section_data.buffer.length);

    vector_append_ptr(&sections, &start_section_data);

    *elf_buffer = construct_elf_file(sections, symbols, start_symbol_offset, START_ADDR);

    return 0;
}