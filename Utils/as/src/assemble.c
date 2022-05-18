#include "assemble.h"

#include "as.h"

#include <elf/symbols.h>
#include <riscv/riscv.h>

#include <libc/string.h>
#include <libc/stdlib.h>

struct section_data create_section_data(char* name)
{
    return (struct section_data){.name = name, .buffer = VECTOR(char), .type = 0, .info=0, .addralign=4, .entsize=0};
}

struct relocation_group create_relocation_group(char* name, uint32_t link)
{
    return (struct relocation_group){.name = name, .relocations = VECTOR(struct relocation_data), .link = link};
}

struct symbol_data create_symbol_data(char* name)
{
    return (struct symbol_data){
        .name = name, 
        .data = (Elf64_Sym){.st_info = 0, .st_name = 0, .st_other = 0, .st_shndx = 0, .st_size = 0, .st_value = 0}};
}

struct symbol_data* find_symbol_name(struct vector symbols, char* name)
{
    struct symbol_data* sa = VEC_TO_ARRAY(symbols, struct symbol_data);
    for (int i = 0; i < symbols.length; i++)
    {
        if (strcmp(sa[i].name, name) == 0)
        {
            return &sa[i];
        }
    }

    return NULL;
}

struct relocation_group* find_relocation_group_name(struct vector relocation_groups, char* name)
{
    struct symbol_data* rga = VEC_TO_ARRAY(relocation_groups, struct relocation_group);
    for (int i = 0; i < relocation_groups.length; i++)
    {
        if (strcmp(rga[i].name, name) == 0)
        {
            return &rga[i];
        }
    }

    return NULL;
}

void place_symbol_at(struct vector sections, size_t section_index, struct symbol_data* symbol)
{
    struct section_data* sa = VEC_TO_ARRAY(sections, struct section_data);

    symbol->data.st_shndx = section_index + 3;
    symbol->data.st_value = sa[section_index].buffer.length;
}

int symbol_compare_function(struct symbol_data* a, struct symbol_data* b)
{
    return (a->data.st_info >> 4) - (b->data.st_info >> 4);
}

void reorder_symbols(struct vector* symbols)
{
    LOG("Reordering Symbols\n");
    struct vector v = *symbols;
    struct symbol_data* symbol_array = VEC_TO_ARRAY(v, struct symbol_data);

    qsort(symbol_array, v.length, sizeof(struct symbol_data), symbol_compare_function);
}

struct vector construct_elf_file(struct vector sections, struct vector symbols);

// Assemble a file from the file handle
void assemble_file_handle(FILE* file, const char* input_name, const char* output_name)
{
    LOG("Assembling %s to %s\n", input_name, output_name);

    // Tokenize the stream
    struct token* tokens = tokenize(file, input_name);

    char buffer[128];
    char buffer2[128];

    // Vector of sections
    struct vector sections = VECTOR(struct section_data);

    // We will start by initializing an empty .text section, since if we just started writing code,
    // that is where it would end up
    struct section_data s = create_section_data(".text");
    s.type = SHT_PROGBITS;
    s.flags = SHF_ALLOC | SHF_EXECINSTR;
    vector_append_ptr(&sections, &s);
    size_t current_section_index = 0;

    // Next, we will initialize an empty vector of symbols, these will be placed in our elf file
    struct vector symbols = VECTOR(struct symbol_data);
    
    // Add a zeroed symbol as the null symbol
    {
        struct symbol_data s = create_symbol_data("");
        vector_append_ptr(&symbols, &s);
    }

    // Next, we will initialize an empty vector of relocations, these will be placed in our elf file
    struct vector relocations = VECTOR(struct relocation_group);

    struct relocation_group g = create_relocation_group(".text", 0);
    vector_append_ptr(&relocations, &g);

    // Iterate over all of the tokens, performing the parsing and translation
    for (int i = 0; tokens[i].type != TOK_NULL;)
    {
        // Check if the next token is a section directive
        if (tokens[i].type == TOK_DIRECTIVE && strcmp(tokens[i].data.string, ".section") == 0)
        {
            struct token next = tokens[++i];
            if (next.type != TOK_DIRECTIVE && next.type != TOK_IDENTIFIER)
            {
                printf("Expected identifier, got %s at %s\n", render_token(buffer, next), render_location(buffer, next.location));
                exit(1);
            }

            char* section_name = next.data.string;

            // Now we have to determine if a new section needs to be created, if so we will make a new one and switch to it, otherwise we can just switch to an already existing section
            struct section_data* sa = VEC_TO_ARRAY(sections, struct section_data);
            for (int i = 0; i < sections.length; i++)
            {
                if (strcmp(sa[i].name, section_name) == 0)
                {
                    current_section_index = i;
                    break;
                }
            }

            if (strcmp(sa[current_section_index].name, section_name))
            {
                LOG("Creating new section for %s at index %i\n", section_name, current_section_index);
                // Create a new section with the given name
                struct section_data this_sec = create_section_data(section_name);
                vector_append_ptr(&sections, &this_sec);

                current_section_index = sections.length - 1;

                // Create the section for relocations with that name
                struct relocation_group this_group = create_relocation_group(section_name, current_section_index);
                vector_append_ptr(&relocations, &this_group);
            }

            i++;
        }
        // Check if the next token is a .globl directive
        else if (tokens[i].type == TOK_DIRECTIVE && strcmp(tokens[i].data.string, ".globl") == 0)
        {
            struct token next = tokens[++i];
            if (next.type != TOK_IDENTIFIER)
            {
                printf("Expected identifier, got %s at %s\n", render_token(buffer, next), render_location(buffer, next.location));
                exit(1);
            }

            char* symbol_name = next.data.string;

            struct symbol_data* symbol = find_symbol_name(symbols, symbol_name);
            if (symbol == NULL)
            {
                struct symbol_data new_symbol = create_symbol_data(symbol_name);
                symbol = &new_symbol;

                symbol->data.st_info |= STB_GLOBAL << 4;
                vector_append_ptr(&symbols, symbol);
            }
            else
            {
                symbol->data.st_info |= STB_GLOBAL << 4;
            }

            i++;
        }
        // Check if the next token is an asciz directive
        else if (tokens[i].type == TOK_DIRECTIVE && strcmp(tokens[i].data.string, ".asciz") == 0)
        {
            struct token next = tokens[++i];
            if (next.type != TOK_STRING)
            {
                printf("Expected string, got %s at %s\n", render_token(buffer, next), render_location(buffer, next.location));
                exit(1);
            }

            char* string_data = next.data.string;

            struct section_data* sa = VEC_TO_ARRAY(sections, struct section_data);

            sa[current_section_index].type = SHT_PROGBITS;

            vector_append_buffer(&(sa[current_section_index].buffer), string_data, strlen(string_data) + 1);

            i++;
        }
        // Check if the next token is an identifier and the subsequent token is a ':'
        else if (tokens[i].type == TOK_IDENTIFIER && tokens[i + 1].type == TOK_SYMBOL && tokens[i + 1].data.character == ':')
        {
            // Get the name of the symbol
            char* symbol_name = tokens[i].data.string;

            // Step over the identifier
            i += 1;

            // Check if there is already a symbol with the given name
            struct symbol_data* symbol = find_symbol_name(symbols, symbol_name);
            if (symbol == NULL)
            {
                struct symbol_data new_symbol = create_symbol_data(symbol_name);
                symbol = &new_symbol;

                place_symbol_at(sections, current_section_index, symbol);
                vector_append_ptr(&symbols, symbol);
            }
            else
            {
                place_symbol_at(sections, current_section_index, symbol);
            }

            i++;
        }
        // If all else fails, it must be an instruction to be assembled, so at this point, pass it along to the instruction based assembler
        else
        {
            struct section_data* sa = VEC_TO_ARRAY(sections, struct section_data);
            struct relocation_group* ga = VEC_TO_ARRAY(relocations, struct relocation_group);
            assemble_instruction(tokens, &i, &(sa[current_section_index].buffer), &(ga[current_section_index].relocations), &symbols, sections, current_section_index);
        }
    }

    // Reorder the symbols so that local variables are put first
    reorder_symbols(&symbols);

    // Iterate over every symbol, printing out its name
    struct symbol_data* symbol_array = VEC_TO_ARRAY(symbols, struct symbol_data);
    for (int i = 0; i < symbols.length; i++)
    {
        printf("Symbol %s Value: %lx\n", symbol_array[i].name, symbol_array[i].data.st_value);
    }

    // Form all of the relocation sections
    struct section_data* sa = VEC_TO_ARRAY(sections, struct section_data);
    struct relocation_group* ga = VEC_TO_ARRAY(relocations, struct relocation_group);
    for (int i = 0; i < relocations.length; i++)
    {

        char rela_name_buffer[256] = ".rela";

        if (!(sa[ga[i].link].flags & SHF_EXECINSTR))
        {
            continue;
        }

        strcat(rela_name_buffer, ga[i].name);
        printf("Relocation Group: %s\n", rela_name_buffer);

        struct section_data rela_section = create_section_data(strdup(rela_name_buffer));
        rela_section.type = SHT_RELA;
        rela_section.buffer = VECTOR(uint8_t);
        rela_section.entsize = 0x18;
        rela_section.addralign = 8;
        rela_section.info = 3 + ga[i].link;

        struct relocation_data* ra = VEC_TO_ARRAY(ga[i].relocations, struct relocation_data);
        for (int j = 0; j < ga[i].relocations.length; j++)
        {
            Elf64_Rela data = ra[j].data;

            // Find the symbol for this relocation
            for (int k = 0; k < symbols.length; k++)
            {
                // printf("Searching for %s, found %s\n", ra[j].name, symbol_array[k].name, k);
                if (strcmp(symbol_array[k].name, ra[j].name) == 0)
                {
                    ra[j].data.r_info |= ((uint64_t)k << 32);
                    break;
                }
                // printf("Symbol %s Value: %lx\n", , symbol_array[i].data.st_value);
            }

            printf("Data: %#lx\n", ra[j].data.r_info);

            vector_append_buffer(&rela_section.buffer, &ra[j].data, sizeof(Elf64_Rela));
        }

        vector_append_ptr(&sections, &rela_section);
    }

    // Construct the ELF file
    struct vector elf_buffer = construct_elf_file(sections, symbols);
    FILE* f = fopen(output_name, "wb");
    fwrite(VEC_TO_ARRAY(elf_buffer, uint8_t), elf_buffer.length, 1, f);

    fclose(f);

    // Free the tokens, this is very important as the array can grow quite large
    free_tokens(tokens);
}

// Convert symbols into a struct section_data
struct section_data convert_symbols(struct vector symbols)
{
    struct section_data sect = create_section_data(".symtab");
    sect.type = SHT_SYMTAB;
    sect.addralign = 8;
    sect.entsize = 0x18;

    struct symbol_data* syms = VEC_TO_ARRAY(symbols, struct symbol_data);

    for (int i = 0; i < symbols.length; i++)
    {
        if ((syms[i].data.st_info >> 4) == STB_LOCAL)
        {
            vector_append_buffer(&sect.buffer, &(syms[i].data), sizeof(Elf64_Sym));
        }
    }

    sect.info = sect.buffer.length / sizeof(Elf64_Sym);

    for (int i = 0; i < symbols.length; i++)
    {
        if ((syms[i].data.st_info >> 4) != STB_LOCAL)
        {
            vector_append_buffer(&sect.buffer, &(syms[i].data), sizeof(Elf64_Sym));
        }
    }

    return sect;
}

struct vector construct_elf_file(struct vector sections, struct vector symbols)
{
    // First, we will create a number of buffers
    struct vector file_buffer = VECTOR(uint8_t);
    struct vector strtab_buffer = VECTOR(uint8_t);
    struct vector shstrtab_buffer = VECTOR(uint8_t);

    // Add null bytes to the beginning of both string buffers so a name of zero will be empty
    vector_append(&strtab_buffer, 0);
    vector_append(&shstrtab_buffer, 0);

    // Next, we will populate the strtab buffer and properly name the symbols
    struct symbol_data* symbol_array = VEC_TO_ARRAY(symbols, struct symbol_data);
    for (int i = 0; i < symbols.length; i++)
    {
        symbol_array[i].data.st_name = strtab_buffer.length;
        vector_append_buffer(&strtab_buffer, symbol_array[i].name, strlen(symbol_array[i].name) + 1);
    }

    // Convert the symbols to a section
    struct section_data symtab = convert_symbols(symbols);
    vector_append_ptr(&sections, &symtab);

    // Next, start constructing the section header objects
    struct vector section_headers = VECTOR(Elf64_Shdr);
    Elf64_Shdr section_header = (Elf64_Shdr)
    {
        .sh_name = 0,
        .sh_type = 0,
        .sh_flags = 0,
        .sh_addr = 0,
        .sh_offset = 0,
        .sh_size = 0,
        .sh_link = 0,
        .sh_info = 0,
        .sh_addralign = 0,
        .sh_entsize = 0
    };

    // Add this structure three times, once for the null, strtab, shstrtab each
    vector_append_ptr(&section_headers, &section_header);
    section_header.sh_name = 11;
    section_header.sh_type = SHT_STRTAB;
    section_header.sh_addralign = 1;
    vector_append_ptr(&section_headers, &section_header);
    section_header.sh_name = 1;
    vector_append_ptr(&section_headers, &section_header);

    // Clear the structure for future use
    section_header.sh_name = 0;
    section_header.sh_type = 0;
    section_header.sh_addralign = 0;

    // Then we will populate the shstrtab buffer 
    const char* names = ".shstrtab\0.strtab\0.symtab";
    vector_append_buffer(&shstrtab_buffer, names, 26);

    struct section_data* section_array = VEC_TO_ARRAY(sections, struct section_data);
    for (int i = 0; i < sections.length; i++)
    {
        char* name = section_array[i].name;

        section_header.sh_name = shstrtab_buffer.length;

        vector_append_buffer(&shstrtab_buffer, name, strlen(name) + 1);

        vector_append_ptr(&section_headers, &section_header);
    }

    // Now that the two string tables are complete, let's add the data about them
    Elf64_Shdr* shs = VEC_TO_ARRAY(section_headers, Elf64_Shdr);

    size_t off = 0x40 + sizeof(Elf64_Shdr) * section_headers.length;

    // .strtab
    shs[1].sh_size = strtab_buffer.length;
    shs[1].sh_offset = off;

    off += strtab_buffer.length;

    // .shstrtab
    shs[2].sh_size = shstrtab_buffer.length;
    shs[2].sh_offset = off;

    off += shstrtab_buffer.length;

    // Buffer for the rest of the section header data
    struct vector section_data_buffer = VECTOR(uint8_t);

    for (int i = 0; i < sections.length; i++)
    {
        char* name = section_array[i].name;

        // Add padding to the align size
        int padding = (section_array[i].addralign - (off % section_array[i].addralign)) % section_array[i].addralign;

        off += padding;

        for (int i = 0; i < padding; i++)
        {
            vector_append(&section_data_buffer, 0);
        }

        shs[3 + i].sh_addralign = section_array[i].addralign;
        shs[3 + i].sh_offset = off;
        shs[3 + i].sh_size = section_array[i].buffer.length;
        shs[3 + i].sh_type = section_array[i].type;
        shs[3 + i].sh_addralign = section_array[i].addralign;
        shs[3 + i].sh_entsize = section_array[i].entsize;
        shs[3 + i].sh_info = section_array[i].info;
        shs[3 + i].sh_link = 1;
        shs[3 + i].sh_flags = section_array[i].flags;

        if (memcmp(".rela", name, 5) == 0)
        {
            shs[3 + i].sh_link = 3 + sections.length - 1;
        }

        off += section_array[i].buffer.length;

        vector_append_buffer(&section_data_buffer, VEC_TO_ARRAY(section_array[i].buffer, uint8_t), section_array[i].buffer.length);
    }

    // Next, we will start building the file itself
    Elf64_Ehdr elf_header = (Elf64_Ehdr){
        .e_ident = {127,69,76,70,2,1,1,0,0,0,0,0,0,0,0,0},
        .e_type = ET_REL,
        .e_machine = 0xf3,
        .e_version = 0,
        .e_entry = 0,
        .e_phoff = 0,
        .e_shoff = 0x40,
        .e_flags = 5,
        .e_ehsize = 0x40,
        .e_phentsize = 0,
        .e_phnum = 0,
        .e_shentsize = 0x40,
        .e_shnum = 3 + sections.length,
        .e_shstrndx = 2
        };
    vector_append_buffer(&file_buffer, &elf_header, sizeof(Elf64_Ehdr));

    vector_append_buffer(&file_buffer, VEC_TO_ARRAY(section_headers, Elf64_Shdr), sizeof(Elf64_Shdr) * section_headers.length);
    vector_append_buffer(&file_buffer, VEC_TO_ARRAY(strtab_buffer, uint8_t), strtab_buffer.length);
    vector_append_buffer(&file_buffer, VEC_TO_ARRAY(shstrtab_buffer, uint8_t), shstrtab_buffer.length);
    vector_append_buffer(&file_buffer, VEC_TO_ARRAY(section_data_buffer, uint8_t), section_data_buffer.length);

    for (int i = 0; i < 100; i++)
        vector_append(&file_buffer, 0);

    return file_buffer;
}