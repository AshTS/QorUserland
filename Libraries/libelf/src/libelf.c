#include <elf/libelf.h>

#include <libc/assert.h>
#include <libc/stdlib.h>
#include <libc/string.h>

int elf_verify(Elf64_Ehdr* header)
{
    // Check the magic
    if (header->e_ident[EI_MAG0] != ELFMAG0 ||
        header->e_ident[EI_MAG1] != ELFMAG1 ||
        header->e_ident[EI_MAG2] != ELFMAG2 ||
        header->e_ident[EI_MAG3] != ELFMAG3)
    {
        return LIB_ELF_BADMAGIC;
    }

    // Make sure it is 64 bit, little endian, and the current version
    if (header->e_ident[EI_CLASS] != ELFCLASS64)
    {
        return LIB_ELF_BADWIDTH;
    }
    if (header->e_ident[EI_DATA] != ELFDATA2LSB)
    {
        return LIB_ELF_BADENDIAN;
    }
    if (header->e_ident[EI_VERSION] != EV_CURRENT)
    {
        return LIB_ELF_BADVERSION;
    }

    return 0;
}

Elf64_Shdr* elf_get_sh(Elf64_Ehdr* header, int i)
{
    if (i >= header->e_shnum)
    {
        return NULL;
    }

    void* ptr = header;

    Elf64_Shdr* sh_ptr = ptr + header->e_shoff;

    return &sh_ptr[i];
}

char* elf_get_section_name(Elf64_Ehdr* header, int offset)
{
    int index = header->e_shstrndx;

    Elf64_Shdr* sh = elf_get_sh(header, index);

    char* ptr = (char*)header + sh->sh_offset + offset;

    return ptr;
}

// Get a string from an offset into the string header
char* elf_get_string(Elf64_Ehdr* header, int offset)
{
    Elf64_Shdr* sh;

    // We need to find the string table, this is super inefficent and could be made much more efficent, but for now this will do
    // TODO: Optimize this
    for (int i = 1; i < header->e_shentsize; i++)
    {
        Elf64_Shdr* sh = elf_get_sh(header, i);

        char* name = elf_get_section_name(header, sh->sh_name);

        if (strcmp(name, ".strtab") == 0)
        {
            char* ptr = (char*)header + sh->sh_offset + offset;

            return ptr;
        }
    }

    return NULL;
}

Elf64_Sym* elf_check_for_symbol(Elf64_Ehdr* header, uint64_t addr, uint16_t section_index)
{
    // Get the base address of the given section index
    size_t base = 0;
    Elf64_Shdr* pointed_section = elf_get_sh(header, section_index);

    if (pointed_section != NULL)
    {
        base = pointed_section->sh_addr & ~(0xFFFF);
    }

    // First, we need to find the symbol table (assuming it is there, otherwise we will output the address)
    Elf64_Shdr* sh = NULL;

    for (int i = 1; i < header->e_shentsize; i++)
    {
        sh = elf_get_sh(header, i);

        if (sh->sh_type == SHT_SYMTAB)
        {
            break;
        }
    }

    // If we didn't find the symbol table, just return the pretty printed address
    if (sh == NULL)
    {
        return NULL;
    }

    // If we did find the symbol table, loop over the known symbols to find the one closest to the address we are trying to symbolicate
    size_t count = sh->sh_size / sizeof(Elf64_Sym);
    uint64_t best = 0xFFFFFFFFFFFFFFFF;

    Elf64_Sym* array = (void*)header + sh->sh_offset;
    Elf64_Sym*result = NULL;

    for (int i = 0; i < count; i++)
    {
        Elf64_Sym symbol = array[i];

        char* name = elf_get_string(header, symbol.st_name);

        if (name == NULL)
        {
            continue;
        }

        size_t value = symbol.st_value + base;

        if (symbol.st_shndx == section_index)
        {
            if ((symbol.st_info & 0xf) == 0 || (symbol.st_info & 0xf) == STT_FUNC)
            {
                if (value <= addr)
                {
                    uint64_t offset = addr - value;

                    if (offset == 0)
                    {
                        result = &array[i];
                    }

                    if (offset <= best)
                    {
                        best = offset;
                    }
                }
            }
        }
    }

    return result;
}

char* elf_get_symbol(Elf64_Ehdr* header, char* buffer, uint64_t addr, uint16_t section_index)
{
    // The default output should just be the address pretty printed
    sprintf(buffer, "<%#lx>", addr);

    // Get the base address of the given section index
    size_t base = 0;
    Elf64_Shdr* pointed_section = elf_get_sh(header, section_index);

    if (pointed_section != NULL)
    {
        base = pointed_section->sh_addr & ~(0xFFFF);
    }

    // First, we need to find the symbol table (assuming it is there, otherwise we will output the address)
    Elf64_Shdr* sh = NULL;

    for (int i = 1; i < header->e_shentsize; i++)
    {
        sh = elf_get_sh(header, i);

        if (sh->sh_type == SHT_SYMTAB)
        {
            break;
        }
    }

    // If we didn't find the symbol table, just return the pretty printed address
    if (sh == NULL)
    {
        return 0;
    }

    // If we did find the symbol table, loop over the known symbols to find the one closest to the address we are trying to symbolicate
    size_t count = sh->sh_size / sizeof(Elf64_Sym);
    uint64_t best = 0xFFFFFFFFFFFFFFFF;

    Elf64_Sym* array = (void*)header + sh->sh_offset;

    for (int i = 0; i < count; i++)
    {
        Elf64_Sym symbol = array[i];

        char* name = elf_get_string(header, symbol.st_name);

        if (name == NULL)
        {
            continue;
        }

        size_t value = symbol.st_value + base;

        if (symbol.st_shndx == section_index)
        {
            if ((symbol.st_info & 0xf) == 0 || (symbol.st_info & 0xf) == STT_FUNC)
            {
                if (value <= addr)
                {
                    uint64_t offset = addr - value;
                    if (offset == 0)
                    {
                        sprintf(buffer, "<%s>", name);
                        best = offset;
                    }
                    else if (offset <= best)
                    {
                        sprintf(buffer, "<%s + %#lx>", name, offset);
                        best = offset;
                    }
                }
            }
        }
    }

    return 0;
}