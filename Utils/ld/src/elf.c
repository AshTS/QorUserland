#include "elf.h"

#include "database.h"

#include <libc/string.h>

struct section_data create_section_data(char* name)
{
    return (struct section_data){.name = strdup(name), .buffer = VECTOR(char), .type = 0, .info=0, .addralign=1, .entsize=0, .address = 0};
}

// Convert symbols into a struct section_data
struct section_data convert_symbols(struct vector symbols)
{
    struct section_data sect = create_section_data(".symtab");
    sect.type = SHT_SYMTAB;
    sect.addralign = 8;
    sect.entsize = 0x18;
    sect.info = 1;

    struct symbol_data* syms = VEC_TO_ARRAY(symbols, struct symbol_data);

    for (int i = 0; i < symbols.length; i++)
    {
        vector_append_buffer(&sect.buffer, &(syms[i].data), sizeof(Elf64_Sym));
    }

    return sect;
}


struct vector construct_elf_file(struct vector sections, struct vector symbols, uint64_t start_symbol_offset, uint64_t vaddr_start)
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

        shs[3 + i].sh_addralign = 0x4;
        shs[3 + i].sh_offset = off;
        shs[3 + i].sh_size = section_array[i].buffer.length;
        shs[3 + i].sh_type = section_array[i].type;
        shs[3 + i].sh_addralign = section_array[i].addralign;
        shs[3 + i].sh_entsize = section_array[i].entsize;
        shs[3 + i].sh_info = section_array[i].info;
        shs[3 + i].sh_link = 1;
        shs[3 + i].sh_flags = section_array[i].flags;
        shs[3 + i].sh_addr = section_array[i].address;

        section_array[i].offset = off;

        if (memcmp(".rela", name, 5) == 0)
        {
            shs[3 + i].sh_link = 3 + sections.length - 1;
        }

        if (i == 0)
        {
            start_symbol_offset += section_array[i].address;
        }

        off += section_array[i].buffer.length;

        vector_append_buffer(&section_data_buffer, VEC_TO_ARRAY(section_array[i].buffer, uint8_t), section_array[i].buffer.length);
    }

    // Construct the program header entries
    struct vector program_header_entries = VECTOR(Elf64_Phdr);

    for (int i = 0; i < sections.length; i++)
    {
        // Only load segments which are marked "SHF_ALLOC"
        if (!(section_array[i].flags & SHF_ALLOC))
        {
            continue;
        }

        Elf64_Phdr prog_header = (Elf64_Phdr){
            .p_align = 0x4,
            .p_filesz = section_array[i].buffer.length,
            .p_flags = PF_R | PF_W | PF_X,
            .p_memsz = section_array[i].buffer.length,
            .p_offset = section_array[i].offset,
            .p_paddr = 0,
            .p_type = PT_LOAD,
            .p_vaddr = section_array[i].address
        };

        vector_append_ptr(&program_header_entries, &prog_header);
    }

    // Next, we will start building the file itself
    Elf64_Ehdr elf_header = (Elf64_Ehdr){
        .e_ident = {127,69,76,70,2,1,1,0,0,0,0,0,0,0,0,0},
        .e_type = ET_REL,
        .e_machine = 0xf3,
        .e_version = 0,
        .e_entry = start_symbol_offset,
        .e_phoff = 0,
        .e_shoff = 0x40,
        .e_flags = 5,
        .e_ehsize = 0x40,
        .e_phentsize = 0x40,
        .e_phnum = program_header_entries.length,
        .e_shentsize = 0x40,
        .e_shnum = 3 + sections.length,
        .e_shstrndx = 2
        };
    vector_append_buffer(&file_buffer, &elf_header, sizeof(Elf64_Ehdr));

    vector_append_buffer(&file_buffer, VEC_TO_ARRAY(section_headers, Elf64_Shdr), sizeof(Elf64_Shdr) * section_headers.length);
    vector_append_buffer(&file_buffer, VEC_TO_ARRAY(strtab_buffer, uint8_t), strtab_buffer.length);
    vector_append_buffer(&file_buffer, VEC_TO_ARRAY(shstrtab_buffer, uint8_t), shstrtab_buffer.length);
    vector_append_buffer(&file_buffer, VEC_TO_ARRAY(section_data_buffer, uint8_t), section_data_buffer.length);

    // Align against a 64 byte boundary
    for (;file_buffer.length % 64;)
        vector_append(&file_buffer, 0);

    ((Elf64_Ehdr*)file_buffer.ptr)->e_phoff = file_buffer.length;


    vector_append_buffer(&file_buffer, program_header_entries.ptr, sizeof(Elf64_Phdr) * program_header_entries.length);

    // Add 128 zeros to the end to make sure there is padding on the end for some parsers
    for (int i = 0;i < 128; i++)
        vector_append(&file_buffer, 0);

    return file_buffer;
}
