#include "elf.h"

#include "database.h"

static uint64_t TEXT_START_ADDR = 0x8000000;
static uint64_t RODATA_START_ADDR = 0x1000000;

struct symbol_data* find_exec_symbol(struct vector* symbols, const char* symbol_name);

struct section_data construct_final_link_section(const char* name, uint32_t type, uint32_t flags, uint64_t address, uint32_t addralign)
{
    struct section_data section = create_section_data(name);

    section.type = type;
    section.flags = flags;
    section.address = address;
    section.addralign = addralign;

    return section;
}

struct linking_context_data
{
    struct section_data* text_section;
    struct section_data* rodata_section;
};

int append_section(struct linking_context_data context_data, struct section_database_entry* src_section, struct vector* symbols);

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

    struct section_data text_section = construct_final_link_section(".text",
                                                SHT_PROGBITS,
                                                SHF_ALLOC | SHF_EXECINSTR,
                                                TEXT_START_ADDR, 4);

    struct section_data rodata_section = construct_final_link_section(".rodata",
                                                SHT_PROGBITS,
                                                SHF_ALLOC | SHF_WRITE,
                                                RODATA_START_ADDR, 64);

    struct linking_context_data context_data = (struct linking_context_data){
        .rodata_section = &rodata_section,
        .text_section = &text_section
    };

    int result = append_section(context_data, start_section, &symbols);

    if (result)
    {
        return result;
    }

    vector_append_ptr(&sections, &text_section);
    vector_append_ptr(&sections, &rodata_section);

    *elf_buffer = construct_elf_file(sections, symbols, start_symbol_offset);

    return 0;
}

int append_section(struct linking_context_data context_data, struct section_database_entry* src_section, struct vector* symbols)
{
    struct section_data* dest_section;
    uint32_t section_index;

    if (src_section->section.sh_flags & SHF_EXECINSTR)
    {
        dest_section = context_data.text_section;
        section_index = 3;
    }
    else
    {
        dest_section = context_data.rodata_section;
        section_index = 4;
    }

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
            .expanded_value = dest_section->address + offset + symbol_array[i].symbol.st_value
        };

        symbol.data.st_shndx = section_index;
        symbol.data.st_value += offset;

        printf("Adding symbol %s with value %lx\n", symbol.name, symbol.expanded_value);

        vector_append_ptr(symbols, &symbol);
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
            // If we didn't find the symbol already included, then we must include it by bringing in the section which contains it
            struct symbol_database_entry* symbol_reference = find_symbol(reloc_array[i].symbol_name);
            if (reloc_array[i].symbol_name == NULL || !symbol_reference->defined)
            {
                printf("ld: Symbol %s not defined\n", reloc_array[i].symbol_name);
                return 1;
            }

            // Get the section associated with the symbol
            struct section_database_entry* symbol_section = associated_section(*symbol_reference);
            if (symbol_section == NULL)
            {
                printf("ld: No section associated with symbol %s\n", reloc_array[i].symbol_name);
                return 1;
            }

            // Now, append that section to the current one
            int result = append_section(context_data, symbol_section, symbols);
            if (result)
            {
                return result;
            }

            // Try to get the symbol again
            symbol = find_exec_symbol(symbols, reloc_array[i].symbol_name);

            if (symbol == NULL)
            {
                printf("ld: Successfully imported section for symbol %s, but no such symbol was defined\n", reloc_array[i].symbol_name);
                return 1;
            }
        }
        else
        {
            printf("Found symbol: %s\n", reloc_array[i].symbol_name);
        }

        // Finally, apply the given relocation
        int result = apply_relocation((void*)dest_section->buffer.ptr + offset, symbol, reloc_array[i], dest_section->address + offset, &relocations);

        if (result)
        {
            return result;
        }
    }

    return 0;
}

struct symbol_data* find_exec_symbol(struct vector* symbols, const char* symbol_name)
{
    printf("Searching for exec symbol %s\n", symbol_name);

    if (symbols == NULL || symbols->ptr == NULL)
    {
        printf("No Symbols Found\n");
        return NULL;
    }

    struct symbol_data* array = VEC_TO_ARRAY((*symbols), struct symbol_data);

    for (int i = 0; i < symbols->length; i++)
    {
        printf("Checking symbol %s\n", array[i].name);
        printf("    Value: %lx\n", array[i].expanded_value);
        if (strcmp(array[i].name, symbol_name) == 0)
        {
            return &array[i];
        }
    }

    return NULL;
}