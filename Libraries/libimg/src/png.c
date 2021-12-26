#include "png.h"

#include <libc/assert.h>
#include <libc/stdio.h>
#include <libc/string.h>

#include "libimg.h"

#define BIG_ENDIAN32(v) (((v & 0xFF) << 24) | ((v & 0xFF00) << 8) | ((v & 0xFF0000) >> 8) | ((v & 0xFF000000) >> 24))

typedef struct png_chunk_header
{
    uint32_t length;
    const char type[4];
    const char data;
} __attribute__((packed)) png_chunk_header;

// Process an individual png chunk
uint8_t handle_png_chunk(void** buffer, struct image_data* data);

// Load a portable network graphic image from a buffer into an image data buffer
int image_backend_png(void* buffer, struct image_data* data)
{
    // Verify the buffer is a png file
    if (memcmp(buffer, "\x89\x50\x4e\x47\x0d\x0a\x1a\x0a", 8) != 0)
    {
        printf("Bad Magic\n");
        return -1;
    }

    void* walk = buffer + 8;

    while (handle_png_chunk(&walk, data));

    return 0;
}

// Process an individual png chunk
uint8_t handle_png_chunk(void** buffer, struct image_data* data)
{
    // Extract the header and data
    png_chunk_header* header = *buffer;
    void* buffer_data = *buffer + 8;

    // Get the size of the chunk (properly)
    size_t len = BIG_ENDIAN32(header->length);

    printf("Got Header: `%c%c%c%c` with length %i\n", header->type[0], header->type[1], header->type[2], header->type[3], len);

    *buffer = buffer_data + len;
    *buffer += 4;

    return memcmp(header->type, "IEND", 4);
}