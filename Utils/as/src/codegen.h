#ifndef _CODEGEN_H
#define _CODEGEN_H

#include <libc/stdbool.h>
#include <libc/stddef.h>
#include <libc/stdint.h>

#include "parser.h"
#include "riscv.h"
#include "linking.h"

struct OutputSection
{
    char* name;
    void* buffer;
    size_t buffer_len;

    uint64_t vaddr;

    size_t length;
};

struct LabelAddress
{
    char* label;
    size_t addr;
};

struct GenerationSettings
{
    struct OutputSection* sections;
    size_t allocated_sections;

    struct LabelAddress* labels;
    size_t allocated_labels;

    size_t sections_i;
    size_t labels_i;

    struct LinkingObject* linking;

    size_t current_addr;
};

void section_expand(struct OutputSection*);
void add_to_section(struct OutputSection*, void*, size_t);

struct GenerationSettings* settings_alloc_new();
void settings_expand_sections(struct GenerationSettings*);
void settings_expand_labels(struct GenerationSettings*);
void settings_add_section(struct GenerationSettings*, char* name);
void settings_add_label(struct GenerationSettings*, char* name, size_t addr);
bool settings_get_label(struct GenerationSettings*, char* name, size_t* addr);
bool settings_get_section(struct GenerationSettings*, char* name, size_t* index);
void settings_alloc_free(struct GenerationSettings*);
bool settings_add_to_current(struct GenerationSettings*, void*, size_t);
bool settings_add_instruction(struct GenerationSettings*, struct Instruction*);

void dump_section(struct OutputSection*);

bool parse_code(struct Token** tokens, struct GenerationSettings*, struct ParsingError*);

#endif // _CODEGEN_H