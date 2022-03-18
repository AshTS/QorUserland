#include <libc/stdio.h>
#include <libc/stdint.h>
#include <libc/stdlib.h>

#include <elf/elf.h>
#include <elf/libelf.h>
#include <elf/symbols.h>

#include <riscv/riscv.h>

#include "settings.h"

static Elf64_Ehdr* symbol_header;
static uint16_t symbol_section;

void dump_section(Elf64_Ehdr* header, Elf64_Shdr* section);

void dump_ptr(void* ptr, uint64_t length)
{
    Elf64_Ehdr* header = ptr;

    // Verify the elf header
    int error = elf_verify(header);
    if (error)
    {
        printf("objdump: file is not an elf: %s\n", elf_strerror(error));
        exit(1);
    }

    // Verify that the file is for the right abi and architecture
    if (header->e_ident[EI_OSABI] != ELFOSABI_SYSV)
    {
        printf("objdump: elf file not for abi 0\n");
        exit(1);
    }
    if (header->e_machine != EM_RISCV)
    {
        printf("objdump: elf file not for riscv\n");
        exit(1);
    }

    for (int i = 0; i < header->e_shnum; i++)
    {
        Elf64_Shdr* sh = elf_get_sh(header, i);

        if (sh == NULL)
        {
            printf("objdump: bad section header index %i\n", i);
            exit(1);
        }

        symbol_header = header;
        symbol_section = i;

        dump_section(header, sh);
    }
}

void hex_dump(void* buf, size_t length)
{
    int line_num_length = 7;
    int sub_grouping = 1;
    int super_grouping = 8;
    char show_ascii = 1;
    
    char* buffer = (char*)buf;

    size_t line = 0;

    while (line < length)
    {
        printf("%0*x ", line_num_length, line);

        for (size_t i = 0; i < 16; i++)
        {
            if (line + i < length)
            {
                printf("%02x", buffer[line + i]);
            }
            else
            {
                printf("  ");
            }

            if ((i + 1) % sub_grouping == 0)
            {
                printf(" ");
            }

            if ((i + 1) % super_grouping == 0)
            {
                printf(" ");
            }
        }

        if (show_ascii)
        {
            printf("|");

            for (size_t i = 0; i < 16; i++)
            {
                if (line + i < length)
                {
                    char c = buffer[line + i];

                    if (c >= 0x20 && c < 0x80)
                    {
                        printf("%c", c);
                    }
                    else
                    {
                        printf(".");
                    }
                }
                else
                {
                    printf(" ");
                }
            }

            printf("|");
        }

        printf("\n");

        line += 16;
    }
}

int render_addr(char* buf, uint64_t addr)
{
    elf_get_symbol(symbol_header, buf, addr, symbol_section);
}

void dump_section(Elf64_Ehdr* header, Elf64_Shdr* section)
{
    void* ptr = (void*)header + section->sh_offset;

    if (output_settings.disassemble && section->sh_size && section->sh_flags & SHF_EXECINSTR)
    {
        printf("Dissassembly of section %s:\n", elf_get_section_name(header, section->sh_name));

        uint32_t* inst_ptr = ptr;

        while ((uint64_t)inst_ptr - (uint64_t)ptr < section->sh_size)
        {   
            struct riscv_inst_repr inst;

            riscv_parse_inst(*(inst_ptr), &inst);

            size_t addr = section->sh_addr + (uint64_t)inst_ptr - (uint64_t)ptr;

            char s[128];

            riscv_render_inst_symbols(s, &inst, addr, render_addr);

            // Check to see if there is a symbol registered for this address
            Elf64_Sym* sym = elf_check_for_symbol(header, addr, symbol_section);

            if (sym != NULL)
            {
                char* name = elf_get_string(header, sym->st_name);

                if (name == NULL)
                {
                    name = "NULL";
                }

                printf("\n%016lx <%s>:\n", addr, name);
            }

            printf("    %07x  %s\n", addr, s);
        
            if (inst.width == 4)
            {
                inst_ptr += 1;
            }
            else
            {
                inst_ptr = ((uint64_t)inst_ptr + 2);
            }
        }
    }
    else if (output_settings.show_symbols && section->sh_size && section->sh_type == SHT_SYMTAB)
    {
        size_t count = section->sh_size / sizeof(Elf64_Sym);
        Elf64_Sym* array = ptr;

        for (int i = 0; i < count; i++)
        {
            Elf64_Sym symbol = array[i];

            char* name = elf_get_string(header, symbol.st_name);

            if (name == NULL)
            {
                name = "";
            }

            printf("%i: `%s`\n", i, name);
        }

        hex_dump(ptr, section->sh_size);

    }
    else if (output_settings.show_data)
    {
        hex_dump(ptr, section->sh_size);
    }
}