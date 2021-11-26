#ifndef LINKING_H
#define LINKING_H

#include <libc/stddef.h>
#include <libc/stdint.h>

#include "parser.h"

enum LinkingType
{
    NONE,
    JUMP_LINK,
    BRANCH_LINK,
    IMMEDIATE_LINK,
    UPPER_IMMEDIATE_LINK
};

struct Link
{
    char* section;
    char* symbol;
    size_t offset;
    enum LinkingType type;
    Location loc;
};

struct LinkingObject
{
    struct Link* links;
    size_t allocated_links;
    
    size_t link_i;
};

struct LinkingObject* linking_alloc_new();
void linking_alloc_free(struct LinkingObject*);
void linking_expand_links(struct LinkingObject*);
void linking_add_link(struct LinkingObject*, char*, char*, Location, size_t, enum LinkingType);

char* linking_type_str(enum LinkingType);

#endif // LINKING_H