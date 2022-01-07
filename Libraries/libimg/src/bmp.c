#include "libimg.h"

#include <libc/stdlib.h>
#include <libc/stdio.h>
#include <libc/string.h>
#include <graphics.h>

#include "bmp.h"

struct bmp_pixel
{
    uint8_t b;
    uint8_t g;
    uint8_t r;
};

// Load a bitmap image from a buffer into an image data buffer
int image_backend_bmp(void* buffer, struct pixel_buffer* data)
{
    // First load the buffer as a bitmap header
    BitmapHeader* header = (BitmapHeader*)buffer;

    // Verify that the buffer has the proper magic number
    if (header->magic0 != 'B' || header->magic1 != 'M')
    {
        return -1;
    }

    // Allocate the pixel buffer
    *data = alloc_pixel_buffer(BGR24, (size_t)header->width, (size_t)header->height);

    // Get a pointer into the bitmap file at the beginning of the pixel data
    uint8_t* pixel_data = (size_t)buffer + (size_t)header->pixel_data_offset;

    // Calculate the length of a line
    size_t line_length = 3 * data->width;
    line_length += (4 - (line_length % 4)) % 4;

    // Iterate over every line, copying the data into the buffer
    void* output_buffer = data->raw_buffer;
    for (size_t y = 0; y < data->height; y++)
    {
        memcpy(output_buffer, pixel_data + line_length * (data->height - 1 - y), line_length);

        // Step the output buffer forwards by the size of a line
        output_buffer += data->line_length / 8;
    }

    return 0;
}