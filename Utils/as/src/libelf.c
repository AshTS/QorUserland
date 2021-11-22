#include "libelf.h"

#include <libc/assert.h>
#include <libc/errno.h>
#include <libc/stddef.h>
#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <libc/sys/syscalls.h>

struct ElfFile* elf_file_alloc_new()
{
    struct ElfFile* buffer = malloc(sizeof(struct ElfFile));

    return buffer;
}

void elf_file_alloc_free(struct ElfFile* elf)
{
    free(elf);
}

int elf_file_open(char* name)
{
    errno = 0;
    FILE* file = fopen(name, "rb");

    if (!file || errno)
    {
        return -1;
    }

    struct ElfFile* buf = elf_file_alloc_new();

    buf->backing = file;

    // TODO: This should be dynamicly found, not just always 32KiB
    buf->buffer = mmap(0, 4096*8, PROT_READ, MAP_PRIVATE, file->fd, 0);

    buf->header = buf->buffer;
    buf->prog_headers = buf->buffer + buf->header->e_phoff;
    buf->sect_headers = buf->buffer + buf->header->e_shoff;

    // Convert the offsets in the program and section headers into raw addresses
    for (size_t i = 0; i < buf->header->e_phnum; i++)
    {
        buf->prog_headers[i].p_offset += (uint64_t)(buf->buffer);
    }

    for (size_t i = 0; i < buf->header->e_shnum; i++)
    {
        buf->sect_headers[i].sh_offset += (uint64_t)(buf->buffer);
    }

    return 0;
}

int elf_file_write(struct ElfFile* elf, char* name)
{
    /*
    errno = 0;
    FILE* file = fopen(name, "wb");

    if (!file || errno)
    {
        return -1;
    }
    
    uint64_t* ph_offsets_old = malloc(sizeof(uint64_t) * elf->header->e_phnum);
    uint64_t* sh_offsets_old = malloc(sizeof(uint64_t) * elf->header->e_shnum);

    size_t cumulative = elf->header->e_ehsize + sizeof(struct ProgramHeader) * elf->header->e_phnum + sizeof(struct SectionHeader) * elf->header->e_shnum;

    for (size_t i = 0; i < elf->header->e_phnum; i++)
    {
        ph_offsets_old[i] = elf->prog_headers[i].p_offset;
        elf->prog_headers[i].p_offset = cumulative;

        cumulative += elf->prog_headers[i]
    }

    for (size_t i = 0; i < elf->header->e_shnum; i++)
    {
        sh_offsets_old[i] = elf->sect_headers[i].sh_offset;
        elf->sect_headers[i].sh_offset = cumulative;
    }

    write(file->fd, elf->header, sizeof(struct ElfHeader));
    write(file->fd, elf->prog_headers, sizeof(struct ProgramHeader) * elf->header->e_phnum);
    write(file->fd, elf->sect_headers, sizeof(struct SectionHeader) * elf->header->e_shnum);

    return 0;*/

    assert(0 && "Not yet implemented");
}

void elf_file_close(struct ElfFile* elf)
{
    if (elf->backing)
    {
        munmap(elf->buffer, 4096*8);
        fclose(elf->backing);
    }

    elf_file_alloc_free(elf);
}