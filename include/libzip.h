#ifndef LIBZIP_H
#define LIBZIP_H

#include <libc/stdint.h>
#include <libc/stddef.h>

#define GZIP_MAGIC0 0x1f
#define GZIP_MAGIC1 0x8b

#define CM_DEFLATE 8

#define FTEXT 1
#define FHCRC 2
#define FEXTRA 4
#define FNAME 8
#define FCOMMENT 16


// Decompress data stored in the DEFLATE format, this will return a buffer
// which needs to be free()ed at a later point to avoid a memory leak, this
// function will return a null pointer if the decompression fails.
uint8_t* deflate_decompress(void* data, size_t* length);

#endif // LIBZIP_H