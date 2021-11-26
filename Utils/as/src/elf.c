#include "elf.h"

#include <libc/assert.h>
#include <libc/errno.h>
#include <libc/stdio.h>
#include <libc/string.h>

#include "generic.h"
#include "linking.h"

bool link(struct GenerationSettings* settings)
{
    DEBUG_PRINTF("\nLinks: %ld\n", settings->linking->link_i);
    
    for (size_t i = 0; i < settings->linking->link_i; i++)
    {
        struct Link link = settings->linking->links[i];

        DEBUG_PRINTF("Link: %10s+%p : %s (%s)\n", link.section, (void*)link.offset, link.symbol, linking_type_str(link.type));

        size_t addr;
        size_t index;

        if (!settings_get_label(settings, link.symbol, &addr))
        {
            printf("ERROR: Unable to locate symbol `%s` for linking at %s\n", link.symbol, render_location(&link.loc));
            return false;
        }

        if (!settings_get_section(settings, link.section, &index))
        {
            printf("ERROR: Unable to locate section `%s` for linking\n", link.section);
            return false;
        }

        size_t offset = addr - (settings->sections[index].vaddr + link.offset);
        uint32_t* ptr = settings->sections[index].buffer + link.offset;

        uint32_t value = 0;

        if (link.type == JUMP_LINK)
        {
            offset &= ~1;
            
            value |= ((offset >> 12) & 0b11111111) << 12;
            value |= ((offset >> 11) & 0b1) << 20;
            value |= ((offset >> 1) & 0b1111111111) << 21;
            value |= ((offset >> 20) & 0b1) << 31;
        }
        else if (link.type == IMMEDIATE_LINK)
        {
            value |= (addr & 0b111111111111) << 20;
        }
        else if (link.type == UPPER_IMMEDIATE_LINK)
        {
            value |= addr & 0b11111111111111111111000000000000;
        }
        else if (link.type == BRANCH_LINK)
        {
            offset &= ~1;
            
            value |= ((offset >> 5) & 0b111111) << 25;
            value |= ((offset >> 11) & 0b1) << 7;
            value |= ((offset >> 1) & 0b1111) << 8;
            value |= ((offset >> 12) & 0b1) << 31;
        }
        else
        {
            assert(0 && "Not yet implemented");
        }

        *ptr |= value;
    }

    return true;
}

bool write_to_elf(struct GenerationSettings* settings, char* name)
{
    if (SHOW_DEBUG)
    {
        printf("\nLabels: %ld\n", settings->labels_i);
        printf("Sections: %ld\n", settings->sections_i);

        for (size_t i = 0; i < settings->labels_i; i++)
        {
            printf("Label: %15s: %p\n", settings->labels[i].label, settings->labels[i].addr);
        }

        for (size_t i = 0; i < settings->sections_i; i++)
        {
            printf("\n");

            dump_section(settings->sections + i);
        }
    }

    struct ElfHeader header;

    header.ei_mag = 0x464c457F;
    header.ei_class = 2;
    header.ei_data = 1;
    header.ei_version = 1;

    header.ei_osabi = 0;
    header.ei_absversion = 0;
    header.e_type = 2;
    header.e_machine = 0xF3;
    header.e_version = 1;
    
    if (!settings_get_label(settings, "_start", &header.e_entry))
    {
        printf("ERROR: No label _start defined\n");
        return false;
    }

    header.e_phoff = 0x40;

    header.e_flags = 0;
    header.e_ehsize = 64;
    header.e_phentsize = 0x38;

    header.e_phnum = settings->sections_i;

    header.e_shentsize = 0x40;

    header.e_shnum = 2 + settings->sections_i;

    header.e_shstrndx = 1;

    
    FILE* f = fopen(name, "wb");

    if (f == 0 || errno > 0)
    {
        printf("%i\n", errno);
        printf("Unable to open file `%s`: %s\n", name, strerror(errno));
        return false;
    }

    size_t after_prog_headers = header.e_ehsize + header.e_phnum * header.e_phentsize;
    size_t walk = after_prog_headers;

    for (size_t i = 0; i < settings->sections_i; i++)
    {
        struct OutputSection* sect = settings->sections + i;

        walk += (sect->align - (walk % sect->align)) % sect->align;

        walk += sect->length;
    }

    size_t str_seg = walk;

    header.e_shoff = str_seg + 11;

    for (size_t i = 0; i < settings->sections_i; i++)
    {
        struct OutputSection* sect = settings->sections + i;

        header.e_shoff += strlen(sect->name) + 1;
    }

    size_t result = fwrite(&header, sizeof(struct ElfHeader), 1, f);
    if (result == 0 || errno > 0)
    {
        printf("%i\n", errno);
        printf("Unable to write to file `%s`: %s\n", name, strerror(errno));
        return false;
    }

    walk = after_prog_headers;

    for (size_t i = 0; i < settings->sections_i; i++)
    {
        struct OutputSection* sect = settings->sections + i;

        struct ProgramHeader prog;

        walk += (sect->align - (walk % sect->align)) % sect->align;

        prog.p_type = 0x00000001;
        prog.p_flags = 0b101;
        prog.p_offset = walk;
        prog.p_vaddr = sect->vaddr;
        prog.p_paddr = sect->vaddr;
        prog.p_filesz = sect->length;
        prog.p_memsz = sect->length;
        prog.p_align = 4;

        size_t result = fwrite(&prog, 1, sizeof(struct ProgramHeader), f);
        if (result == 0 || errno > 0)
        {
            printf("%i\n", errno);
            printf("Unable to write to file `%s`: %s\n", name, strerror(errno));
            return false;
        }

        walk += sect->length;
    }

    walk = after_prog_headers;
    for (size_t i = 0; i < settings->sections_i; i++)
    {
        struct OutputSection* sect = settings->sections + i;

        size_t align_offset = (sect->align - (walk % sect->align)) % sect->align;
        walk += align_offset;

        if (align_offset != 0)
        {
            size_t result = fwrite("\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 1, align_offset, f);
            if (result == 0 || errno > 0)
            {
                printf("%i\n", errno);
                printf("Unable to write to file `%s`: %s\n", name, strerror(errno));
                return false;
            }
        }

        size_t result = fwrite(sect->buffer, 1, sect->length, f);
        if (result == 0 || errno > 0)
        {
            printf("%i\n", errno);
            printf("Unable to write to file `%s`: %s\n", name, strerror(errno));
            return false;
        }

        walk += sect->length;
    }
    
    result = fwrite("\0.shstrtab", 1, 11, f);
    if (result == 0 || errno > 0)
        {
            printf("%i\n", errno);
            printf("Unable to write to file `%s`: %s\n", name, strerror(errno));
            return false;
        }

    for (size_t i = 0; i < settings->sections_i; i++)
    {
        struct OutputSection* sect = settings->sections + i;

        size_t result = fwrite(sect->name, 1, strlen(sect->name) + 1, f);
        if (result == 0 || errno > 0)
        {
            printf("%i\n", errno);
            printf("Unable to write to file `%s`: %s\n", name, strerror(errno));
            return false;
        }
    }

    struct SectionHeader secth;

    secth.sh_name = 0;
    secth.sh_type = 0;
    secth.sh_flags = 0;
    secth.sh_addr = 0;
    secth.sh_offset = 0;
    secth.sh_size = 0;
    secth.sh_link = 0;
    secth.sh_info = 0;
    secth.sh_addralign = 0;
    secth.sh_entsize = 0;

    result = fwrite(&secth, 1, sizeof(struct SectionHeader), f);
    if (result == 0 || errno > 0)
    {
        printf("%i\n", errno);
        printf("Unable to write to file `%s`: %s\n", name, strerror(errno));
        return false;
    }
    
    secth.sh_name = 1;
    secth.sh_type = 3;
    secth.sh_flags = 20;
    secth.sh_addr = 0;
    secth.sh_offset = str_seg;
    secth.sh_size = header.e_shoff - str_seg;
    secth.sh_link = 0;
    secth.sh_info = 0;
    secth.sh_addralign = 0;
    secth.sh_entsize = 0;

    result = fwrite(&secth, 1, sizeof(struct SectionHeader), f);
    if (result == 0 || errno > 0)
    {
        printf("%i\n", errno);
        printf("Unable to write to file `%s`: %s\n", name, strerror(errno));
        return false;
    }

    size_t name_offset = 11;
    walk = after_prog_headers;

    for (size_t i = 0; i < settings->sections_i; i++)
    {
        struct OutputSection* sect = settings->sections + i;

        struct SectionHeader secth;

        walk += (sect->align - (walk % sect->align)) % sect->align;
    
        secth.sh_name = name_offset;
        secth.sh_type = 1;
        secth.sh_flags = 6;
        secth.sh_addr = sect->vaddr;
        secth.sh_offset = walk;
        secth.sh_size = sect->length;
        secth.sh_link = 0;
        secth.sh_info = 0;
        secth.sh_addralign = 4;
        secth.sh_entsize = 0;

        size_t result = fwrite(&secth, 1, sizeof(struct SectionHeader), f);
        if (result == 0 || errno > 0)
        {
            printf("%i\n", errno);
            printf("Unable to write to file `%s`: %s\n", name, strerror(errno));
            return false;
        }

        walk += sect->length;
        name_offset += 1 + strlen(sect->name);
    }

    fclose(f);
    if (errno > 0)
    {
        printf("Unable to close file `%s`: %s\n", name, strerror(errno));
        return false;
    }


    return true;
}