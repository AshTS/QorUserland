#include "elf.h"

#include "database.h"

static uint64_t START_ADDR = 0x1000000;

int append_section(struct section_data* dest_section, struct section_database_entry* src_section, struct vector* symbols, uint32_t section_index);
struct symbol_data* find_exec_symbol(struct vector* symbols, const char* symbol_name);

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

    int result = append_section(&start_section_data, start_section, &symbols, 3 + sections.length);

    if (result)
    {
        return result;
    }

    vector_append_ptr(&sections, &start_section_data);

    *elf_buffer = construct_elf_file(sections, symbols, start_symbol_offset, START_ADDR);

    return 0;
}

int append_section(struct section_data* dest_section, struct section_database_entry* src_section, struct vector* symbols, uint32_t section_index)
{
    printf("Appending Section %s(%s) to %s\n", src_section->file, src_section->name, dest_section->name);

    uint64_t offset = dest_section->buffer.length;

    vector_append_buffer(&(dest_section->buffer), src_section->ptr, src_section->section.sh_size);

    // For now, just take all of the symbols and move them over
    struct symbol_database_entry* symbol_array = VEC_TO_ARRAY(symbol_database, struct symbol_databaes_entry);
    for (int i = 0; i < symbol_database.length; i++)
    {
        if (strcmp(symbol_array[i].file, src_section->file) != 0)
        {
            continue;
        }

        if (strcmp(symbol_array[i].section, src_section->name) != 0)
        {
            continue;
        }

        // Append the symbol to the list of symbols to be inserted into the Elf file
        struct symbol_data symbol = (struct symbol_data)
        {
            .name = symbol_array[i].name,
            .data = symbol_array[i].symbol,
            .expanded_value = dest_section->address + offset + symbol.data.st_value
        };

        symbol.data.st_shndx = section_index;
        symbol.data.st_value += offset;

        vector_append_ptr(symbols, &symbol);

        struct symbol_data* a = VEC_TO_ARRAY((*symbols), struct symbol_data);
        printf("%lx\n", a[symbols->length - 1].expanded_value);
    }

    // Next, we want to apply any relocations which have been requested for this section
    struct vector relocations = src_section->relocation_database; // struct relocation_database_entry
    struct relocation_database_entry* reloc_array = VEC_TO_ARRAY(relocations, struct relocation_database_entry);

    for (int i = 0; i < relocations.length; i++)
    {
        printf("Processing relocation for %s\n", reloc_array[i].symbol_name);

        // Check if the symbol required for the relocation has already been included in the executable
        struct symbol_data* symbol = find_exec_symbol(symbols, reloc_array[i].symbol_name);

        if (symbol == NULL)
        {
            printf("Symbol %s not yet included\n", reloc_array[i].symbol_name);
            return 1;
        }
        else
        {
            printf("Found symbol: %s\n", reloc_array[i].symbol_name);
        }

        // Finally, apply the given relocation
        int result = apply_relocation((void*)dest_section->buffer.ptr + offset, symbol, reloc_array[i], dest_section->address + offset);

        if (result)
        {
            return result;
        }
    }

    return 0;
}

struct symbol_data* find_exec_symbol(struct vector* symbols, const char* symbol_name)
{
    struct symbol_data* array = VEC_TO_ARRAY((*symbols), struct symbol_data);

    for (int i = 0; i < symbol_database.length; i++)
    {
        if (strcmp(array[i].name, symbol_name) == 0)
        {
            return &array[i];
        }
    }

    return NULL;
}