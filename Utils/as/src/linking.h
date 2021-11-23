#ifndef LINKING_H
#define LINKING_H

#include <libc/stddef.h>
#include <libc/stdint.h>

enum LinkingType
{
    JUMP_LINK,
    BRANCH_LINK
};

struct Link
{
    char* section;
    char* symbol;
    size_t offset;
    enum LinkingType type;
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
void linking_add_link(struct LinkingObject*, char*, char*, size_t, enum LinkingType);

#endif // LINKING_H