#include "libimg.h"

#include <libc/stdlib.h>
#include <libc/stdio.h>
#include <graphics.h>

#include "bmp.h"

struct bmp_pixel
{
    uint8_t b;
    uint8_t g;
    uint8_t r;
};

// Load a bitmap image from a buffer into an image data buffer
int image_backend_bmp(void* buffer, struct image_data* data)
{
    // First load the buffer as a bitmap header
    BitmapHeader* header = (BitmapHeader*)buffer;

    // Verify that the buffer has the proper magic number
    if (header->magic0 != 'B' || header->magic1 != 'M')
    {
        return -1;
    }

    // Get the width and height of the image and load them into the data structure
    data->width = (size_t)header->width;
    data->height = (size_t)header->height;

    // Allocate the space for the image buffer
    data->buffer = malloc(4 * data->width * data->height);

    // Get a pointer into the bitmap file at the beginning of the pixel data
    uint8_t* pixel_data = (size_t)buffer + (size_t)header->pixel_data_offset;

    // Calculate the length of a line
    size_t line_length = 3 * data->width;
    line_length += (4 - (line_length % 4)) % 4;

    // Iterate over every line, copying the data into the buffer
    struct Pixel* output_buffer = data->buffer;
    for (size_t y = 0; y < data->height; y++)
    {
        size_t true_y = data->height - 1 - y;
        struct bmp_pixel* line = pixel_data + y * line_length;

        for (size_t x = 0; x < data->width; x++)
        {
            struct Pixel p = (struct Pixel){.r=line[x].r, .g=line[x].g, .b=line[x].b, .a=255};
            output_buffer[true_y * data->width + x] = p;
        }
    }

    return 0;
}