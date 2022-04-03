#include "vector.h"

#include <libc/assert.h>
#include <libc/stdlib.h>
#include <libc/string.h>

// Method to append an object to a vector
void vector_append_ptr(struct vector* vec, void* obj)
{
    // If there is enough space, we can just insert the token
    if (vec->length < vec->max_length)
    {
        memcpy(vec->ptr + vec->element_size * vec->length, (const void*)obj, vec->element_size);
        vec->length++;
        return;
    }

    // But if not, we need to allocate more space
    size_t new_size = vec->max_length ? vec->max_length * 2 : 16;

    // Allocate the new buffer
    void* new_buffer = malloc(new_size * vec->element_size);

    // If there was data in the previous buffer, copy it over
    if (vec->max_length)
    {
        memcpy(new_buffer, vec->ptr, vec->max_length * vec->element_size);
    }

    // If the previous buffer was not null, free it
    if (vec->ptr)
    {
        free(vec->ptr);
    }

    // Update the new max_length and return the new buffer
    vec->max_length = new_size;

    // Update the ptr
    vec->ptr = new_buffer;

    // Finally, perform the add with the new buffer
    vector_append_ptr(vec, obj);
}


// Method to append an object less than 8 bytes to an array
void vector_append(struct vector* vec, uint64_t obj)
{
    vector_append_ptr(vec, &obj);
}

// Method to append a buffer to a byte vector
void vector_append_buffer(struct vector* vec, void* obj, size_t length)
{   
    assert(vec->element_size == 1);

    // If there is enough space, we can just insert the token
    if (vec->length + length <= vec->max_length)
    {
        memcpy(vec->ptr + vec->element_size * vec->length, (const void*)obj, length);
        vec->length += length;
        return;
    }

    // But if not, we need to allocate more space
    size_t new_size = vec->max_length ? vec->max_length * 2 : 16;

    // Allocate the new buffer
    void* new_buffer = malloc(new_size * vec->element_size);

    // If there was data in the previous buffer, copy it over
    if (vec->max_length)
    {
        memcpy(new_buffer, vec->ptr, vec->max_length * vec->element_size);
    }

    // If the previous buffer was not null, free it
    if (vec->ptr)
    {
        free(vec->ptr);
    }

    // Update the new max_length and return the new buffer
    vec->max_length = new_size;

    // Update the ptr
    vec->ptr = new_buffer;

    // Finally, perform the add with the new buffer
    vector_append_buffer(vec, obj, length);
}

void vector_free(struct vector vec)
{
    if (vec.ptr)
    {
        free(vec.ptr);
    }
}