#include "png.h"

#include <libc/assert.h>
#include <libc/stdio.h>
#include <libc/string.h>

#include "libimg.h"
#include "graphics.h"
#include "libzip.h"

#define BIG_ENDIAN32(v) (((v & 0xFF) << 24) | ((v & 0xFF00) << 8) | ((v & 0xFF0000) >> 8) | ((v & 0xFF000000) >> 24))
#define ABS(x) ((x < 0) ? -(x) : (x))
typedef struct png_chunk_header
{
    uint32_t length;
    const char type[4];
    const char data;
} __attribute__((packed)) png_chunk_header;

struct png_metadata_chunk
{
    uint32_t width;
    uint32_t height;
    uint8_t bit_depth;
    uint8_t color_type;
    uint8_t compression;
    uint8_t filter_method;
    uint8_t interlacing;
};

struct png_24bpp
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

// Process an individual png chunk
int handle_png_chunk(void** buffer, struct image_data* data, void** compressed_buffer, size_t* compressed_buffer_size);

uint8_t paeth_byte(uint8_t a, uint8_t b, uint8_t c)
{
    /*
     ; a = left, b = above, c = upper left
        p := a + b - c        ; initial estimate
        pa := abs(p - a)      ; distances to a, b, c
        pb := abs(p - b)
        pc := abs(p - c)
        ; return nearest of a,b,c,
        ; breaking ties in order a,b,c.
        if pa <= pb AND pa <= pc then return a
        else if pb <= pc then return b
        else return c
   end*/

   int32_t p = (int32_t)a + (int32_t)b - (int32_t)c;

   int32_t pa = ABS(p - (int32_t)a);
   int32_t pb = ABS(p - (int32_t)b);
   int32_t pc = ABS(p - (int32_t)c);

   if ((pa <= pb) && (pa <= pc))
   {
       return a;
   }
   else if (pb <= pc)
   {
       return b;
   }

   return c;
}

struct Pixel paeth(struct Pixel a, struct Pixel b, struct Pixel c)
{
    return (struct Pixel){.r=paeth_byte(a.r, b.r, c.r), .g=paeth_byte(a.g, b.g, c.g), .b=paeth_byte(a.b, b.b, c.b), .a=255};
}

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

    void* compressed_buffer = 0;
    size_t compressed_buffer_size = 0;

    while (1)
    {
        int result = handle_png_chunk(&walk, data, &compressed_buffer, &compressed_buffer_size);
        
        if (result < 0)
        {
            printf("Bad Chunk\n");
            return 1;
        }
        if (result == 0)
        {
            break;
        }
    }

    if (compressed_buffer_size)
    {
        size_t decompressed_size = 0;
        void* decompressed = deflate_decompress(compressed_buffer + 2, &decompressed_size);
        free(compressed_buffer);

        for (size_t y = 0; y < data->height; y++)
        {
            uint8_t* line_pointer = decompressed + y * (1 + 3 * data->width);
            uint8_t filter_type = *line_pointer;
            struct png_24bpp* pixels = (struct png_24bpp*)(line_pointer + 1);

            for (size_t x = 0; x < data->width; x++)
            {
                struct png_24bpp color = pixels[x];
                
                
                if (filter_type == 1)
                {
                    struct Pixel last = (struct Pixel){.r = 0, .g = 0, .b = 0, .a = 0};

                    if (x > 0)
                    {
                        last = ((struct Pixel*)data->buffer)[y * data->width + x - 1];
                    }

                    ((struct Pixel*)data->buffer)[y * data->width + x] = (struct Pixel){.r = last.r + color.r, .g = last.g + color.g, .b = last.b + color.b, .a = 255};
                }
                else if (filter_type == 2)
                {
                    struct Pixel last = (struct Pixel){.r = 0, .g = 0, .b = 0, .a = 0};

                    if (y > 0)
                    {
                        last = ((struct Pixel*)data->buffer)[(y - 1) * data->width + x];
                    }

                    ((struct Pixel*)data->buffer)[y * data->width + x] = (struct Pixel){.r = last.r + color.r, .g = last.g + color.g, .b = last.b + color.b, .a = 255};
                }
                else if (filter_type == 3)
                {
                    struct Pixel up = (struct Pixel){.r = 0, .g = 0, .b = 0, .a = 0};
                    struct Pixel left = (struct Pixel){.r = 0, .g = 0, .b = 0, .a = 0};

                    if (x > 0)
                    {
                        left = ((struct Pixel*)data->buffer)[y * data->width + x - 1];
                    }
                    if (y > 0)
                    {
                        up = ((struct Pixel*)data->buffer)[(y - 1) * data->width + x];
                    }

                    struct Pixel last = (struct Pixel){.r = ((size_t)left.r + (size_t)up.r) / 2, .g = ((size_t)left.g + (size_t)up.g) / 2, .b = ((size_t)left.b + (size_t)up.b) / 2};

                    ((struct Pixel*)data->buffer)[y * data->width + x] = (struct Pixel){.r = last.r + color.r, .g = last.g + color.g, .b = last.b + color.b, .a = 255};
                }
                else if (filter_type == 4)
                {
                    struct Pixel up = (struct Pixel){.r = 0, .g = 0, .b = 0, .a = 0};
                    struct Pixel left = (struct Pixel){.r = 0, .g = 0, .b = 0, .a = 0};
                    struct Pixel ul = (struct Pixel){.r = 0, .g = 0, .b = 0, .a = 0};

                    if (x > 0)
                    {
                        left = ((struct Pixel*)data->buffer)[y * data->width + x - 1];
                    }
                    if (y > 0)
                    {
                        up = ((struct Pixel*)data->buffer)[(y - 1) * data->width + x];
                    }
                    if (x > 0 && y > 0)
                    {
                        ul = ((struct Pixel*)data->buffer)[(y - 1) * data->width + x - 1];
                    }

                    struct Pixel last = paeth(left, up, ul);

                    ((struct Pixel*)data->buffer)[y * data->width + x] = (struct Pixel){.r = last.r + color.r, .g = last.g + color.g, .b = last.b + color.b, .a = 255};
                }
                else
                {
                    ((struct Pixel*)data->buffer)[y * data->width + x] = (struct Pixel){.r = color.r, .g = color.g, .b = color.b, .a = 255};
                }
            }
        }
    }

    return 0;
}

// Handle a metadata chunk
int handle_metadata_chunk(void* buffer, struct image_data* data)
{
    struct png_metadata_chunk* chunk_data = buffer;

    // Copy the width and height
    data->width = (size_t)BIG_ENDIAN32(chunk_data->width);
    data->height = (size_t)BIG_ENDIAN32(chunk_data->height);

    // Make sure we are in a supported format, for right now just a bitDepth of 8, true color, filter set 0, and no interlacing
    if (chunk_data->bit_depth != 8)
    {
        printf("Invalid bit depth of %i \n", chunk_data->bit_depth);
        assert(0);
    }

    if (chunk_data->color_type != 2)
    {
        printf("Invalid color type of %i \n", chunk_data->color_type);
        assert(0);
    }

    if (chunk_data->filter_method != 0)
    {
        printf("Invalid filter set of %i \n", chunk_data->filter_method);
        assert(0);
    }

    if (chunk_data->interlacing != 0)
    {
        printf("Invalid interlacing value %i \n", chunk_data->interlacing);
        assert(0);
    }

    struct Pixel* pixels = malloc(4 * data->width * data->height);

    data->buffer = pixels;

    for (size_t i = 0; i < data->width * data->height; i++)
    {
        pixels[i] = (struct Pixel){.r = 128, .g = 128, .b = 128, .a = 255};
    }

    return 0;
}

// Handle a data chunk
int handle_data_chunk(void* buffer, struct image_data* data, size_t length, void** compressed_buffer, size_t* compressed_buffer_size)
{
    void* new_buffer = malloc(*compressed_buffer_size + length);
    if (*compressed_buffer_size != 0)
    {
        memcpy(new_buffer, *compressed_buffer, *compressed_buffer_size);
    }

    memcpy(new_buffer + *compressed_buffer_size, buffer, length);

    if (*compressed_buffer_size != 0)
    {
        free(*compressed_buffer);
    }

    *compressed_buffer = new_buffer;
    *compressed_buffer_size += length;

    return 1;
}

// Process an individual png chunk
int handle_png_chunk(void** buffer, struct image_data* data, void** compressed_buffer, size_t* compressed_buffer_size)
{
    // Extract the header and data
    png_chunk_header* header = *buffer;
    void* buffer_data = *buffer + 8;

    // Get the size of the chunk (properly)
    size_t len = BIG_ENDIAN32(header->length);
    // Handle the metadata chunk
    if (memcmp(header->type, "IHDR", 4) == 0)
    {
        handle_metadata_chunk(buffer_data, data);
    }
    else if (memcmp(header->type, "IDAT", 4) == 0)
    {
        handle_data_chunk(buffer_data, data, len, compressed_buffer, compressed_buffer_size);
    }

    // Move the buffer along to ahead of the next chunk
    *buffer = buffer_data + len;
    *buffer += 4;
    return memcmp(header->type, "IEND", 4) == 0 ? 0 : 1;
}