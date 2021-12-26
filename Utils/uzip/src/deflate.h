#ifndef DEFLATE_H
#define DEFLATE_H

#include <libc/stdint.h>
#include <libc/stddef.h>

// Decompress data stored in the DEFLATE format, this will return a buffer
// which needs to be free()ed at a later point to avoid a memory leak, this
// function will return a null pointer if the decompression fails.
uint8_t* deflate_decompress(void* data, size_t* length);

#endif // DEFLATE_H