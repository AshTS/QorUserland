#include "codegen.h"

#include <libc/assert.h>
#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <libc/string.h>

#include "parser.h"
#include "riscv.h"

void dump_section(struct OutputSection* sect)
{
    printf("Section %s @ %p:\n", sect->name, (void*)sect->vaddr);
    printf("        0  1  2  3  4  5  6  7   8  9  A  B  C  D  E  F\n");

    for (size_t row = 0; row < (sect->length + 15) / 16; row++)
    {
        printf("  %4x", (int)row);

        for (size_t i = 0; i < 16; i++)
        {
            if (i % 8 == 0)
            {
                printf(" ");
            }

            size_t addr = (row * 16) | i;

            if (addr < sect->length)
            {
                printf(" %02x", ((char*)sect->buffer)[addr]);
            }
            else
            {
                printf("   ");
            }
        }

        printf("     ");

        for (size_t i = 0; i < 16; i++)
        {
            size_t addr = (row * 16) | i;

            if (addr < sect->length)
            {
                char data = ((char*)sect->buffer)[addr];
                if (data >= ' ' && data <= 128)
                {
                    printf("%c", data);
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

        printf("\n");
    }
}

struct GenerationSettings* settings_alloc_new()
{
    struct GenerationSettings* settings = malloc(sizeof(struct GenerationSettings));

    settings->sections = 0;
    settings->allocated_sections = 0;

    settings->labels = 0;
    settings->allocated_labels = 0;

    settings->sections_i = 0;
    settings->labels_i = 0;

    settings->current_addr = 0x8000000;

    settings->linking = linking_alloc_new();

    return settings;
}

void settings_expand_sections(struct GenerationSettings* settings)
{
    size_t next = 16;
    if (settings->allocated_sections > 0)
    {
        next = settings->allocated_sections * 2;
    }

    struct OutputSection* new_buf = malloc(sizeof(struct OutputSection) * next);

    if (settings->allocated_sections > 0)
        memcpy(new_buf, settings->sections, sizeof(struct OutputSection) * settings->allocated_sections);
        free(settings->sections);

    settings->sections = new_buf;
    settings->allocated_sections = next;
}

void settings_expand_labels(struct GenerationSettings* settings)
{
    size_t next = 16;
    if (settings->allocated_labels > 0)
    {
        next = settings->allocated_labels * 2;
    }

    struct LabelAddress* new_buf = malloc(sizeof(struct LabelAddress) * next);

    if (settings->allocated_labels > 0)
    {
        memcpy(new_buf, settings->labels, sizeof(struct LabelAddress) * settings->allocated_labels);
        free(settings->labels);
    }

    settings->labels = new_buf;
    settings->allocated_labels = next;
}

void settings_alloc_free(struct GenerationSettings* settings)
{
    if (settings->labels != 0)
        free(settings->labels);

    if (settings->sections != 0)
        free(settings->sections);

    linking_alloc_free(settings->linking);

    free(settings);
}

void settings_add_section(struct GenerationSettings* settings, char* name)
{
    if (settings->sections_i == settings->allocated_sections)
    {
        settings_expand_sections(settings);
    }

    settings->sections[settings->sections_i].buffer = 0;
    settings->sections[settings->sections_i].buffer_len = 0;
    settings->sections[settings->sections_i].vaddr = settings->current_addr;
    settings->sections[settings->sections_i].length = 0;

    settings->sections[settings->sections_i++].name = name;
}

void settings_add_label(struct GenerationSettings* settings, char* name, size_t addr)
{
    if (settings->labels_i == settings->allocated_labels)
    {
        settings_expand_labels(settings);
    }

    settings->labels[settings->labels_i].addr = addr;
    settings->labels[settings->labels_i++].label = name;
}

void section_expand(struct OutputSection* section)
{
    size_t next = 64;
    if (section->buffer_len != 0)
    {
        next = section->buffer_len * 2;
    }

    void* next_buf = malloc(next);

    if (section->buffer_len != 0)
    {
        memcpy(next_buf, section->buffer, section->buffer_len);
        free(section->buffer);
    }

    section->buffer = next_buf;
    section->buffer_len = next;
}


void add_to_section(struct OutputSection* section, void* data, size_t len)
{
    while (section->length + len >= section->buffer_len)
    {
        section_expand(section);
    }

    memcpy(section->buffer + section->length, data, len);

    section->length += len;
}


bool settings_add_to_current(struct GenerationSettings* settings, void* data, size_t length)
{
    if (settings->sections_i == 0)
    {
        return false;
    }

    add_to_section(&settings->sections[settings->sections_i - 1], data, length);
    settings->current_addr += length;
    return true;
}

bool settings_add_instruction(struct GenerationSettings* settings, struct Instruction* inst, Location loc)
{
    struct OutputSection sect = settings->sections[settings->sections_i - 1];

    if (inst->j_link)
    {
        linking_add_link(settings->linking, sect.name, inst->link, loc, sect.length, JUMP_LINK);
    }
    else if (inst->b_link)
    {
        linking_add_link(settings->linking, sect.name, inst->link, loc, sect.length, BRANCH_LINK);
    }

    uint32_t compiled = compile_instruction(inst);

    return settings_add_to_current(settings, &compiled, 4);
}


bool settings_get_label(struct GenerationSettings* settings, char* name, size_t* addr)
{
    for (size_t i = 0; i < settings->labels_i; i++)
    {
        if (strcmp(settings->labels[i].label, name) == 0)
        {
            *addr = settings->labels[i].addr;
            return true;
        }
    }

    return false;
}

bool settings_get_section(struct GenerationSettings* settings, char* name, size_t* index)
{
    for (size_t i = 0; i < settings->sections_i; i++)
    {
        if (strcmp(settings->sections[i].name, name) == 0)
        {
            *index = i;
            return true;
        }
    }

    return false;
}