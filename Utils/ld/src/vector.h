#ifndef VECTOR_H
#define VECTOR_H

#include <libc/stddef.h>

// Generic vector structure
struct vector
{
    size_t element_size;
    size_t length;
    size_t max_length;

    void* ptr;
};

// Macro for creating a new vector
#define VECTOR(type) (struct vector){.element_size = sizeof(type), .length = 0, .max_length = 0, .ptr = NULL}

// Macro for converting a vector to a c style array
#define VEC_TO_ARRAY(vec, type) ((type*)vec.ptr)

// Method to append an object to a vector
void vector_append_ptr(struct vector* vec, void* obj);

// Method to append an object less than 8 bytes to an array
void vector_append(struct vector* vec, uint64_t obj);

// Method to append a buffer to a byte vector
void vector_append_buffer(struct vector* vec, void* obj, size_t length);

// Free a vector
void vector_free(struct vector vec);

#endif // VECTOR_H