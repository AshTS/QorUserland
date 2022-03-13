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