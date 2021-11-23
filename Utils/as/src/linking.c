#include "linking.h"

#include <libc/stdlib.h>
#include <libc/string.h>

struct LinkingObject* linking_alloc_new()
{
    struct LinkingObject* result = malloc(sizeof(struct LinkingObject));

    result->links = NULL;
    result->allocated_links = 0;

    result->link_i = 0;

    return result;
}

void linking_alloc_free(struct LinkingObject* obj)
{
    if (obj->allocated_links)
    {
        free(obj->links);
    }
    
    free(obj);
}

void linking_expand_links(struct LinkingObject* obj)
{
    size_t next = 4;
    if (obj->allocated_links)
    {
        next = obj->allocated_links * 2;
    }

    struct Link* new_buf = malloc(sizeof(struct Link) * next);

    if (obj->allocated_links)
    {
        memcpy(new_buf, obj->links, sizeof(struct Link) * obj->allocated_links);
        free(obj->links);
    }

    obj->links = new_buf;
    obj->allocated_links = next;
}

void linking_add_link(struct LinkingObject* obj, char* section, char* symbol, Location loc, size_t offset, enum LinkingType type)
{
    if (obj->link_i == obj->allocated_links)
    {
        linking_expand_links(obj);
    }

    obj->links[obj->link_i++] = (struct Link){.section = section, .symbol = symbol, .offset = offset, .type = type, .loc = loc};
}