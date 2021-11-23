#ifndef _LIBELF_H
#define _LIBELF_H

#include <libc/stdbool.h>
#include <libc/stddef.h>
#include <libc/stdio.h>

#include "codegen.h"

#define ELF_WIDTH_64 uint64_t
#define ELF_WIDTH_32 uint32_t

#define ELF_WIDTH ELF_WIDTH_64

struct ElfHeader
{
    int ei_mag;
    char ei_class;
    char ei_data;
    char ei_version;
    char ei_osabi;
    char ei_absversion;
    char ei_pad[7];
    short e_type;
    short e_machine;
    int e_version;
    ELF_WIDTH e_entry;
    ELF_WIDTH e_phoff;
    ELF_WIDTH e_shoff;
    int e_flags;
    short e_ehsize;
    short e_phentsize;
    short e_phnum;
    short e_shentsize;
    short e_shnum;
    short e_shstrndx;
};

struct ProgramHeader
{
    int p_type;
    int p_flags;
    ELF_WIDTH p_offset;
    ELF_WIDTH p_vaddr;
    ELF_WIDTH p_paddr;
    ELF_WIDTH p_filesz;
    ELF_WIDTH p_memsz;
    ELF_WIDTH p_align;
};

struct SectionHeader
{
    int sh_name;
    int sh_type;
    ELF_WIDTH sh_flags;
    ELF_WIDTH sh_addr;
    ELF_WIDTH sh_offset;
    ELF_WIDTH sh_size;
    int sh_link;
    int sh_info;
    ELF_WIDTH sh_addralign;
    ELF_WIDTH sh_entsize;
};

bool link(struct GenerationSettings*);
bool write_to_elf(struct GenerationSettings* settings, char* name);

#endif // _LIBELF_H