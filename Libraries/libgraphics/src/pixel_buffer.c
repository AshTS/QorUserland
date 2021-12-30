#include "graphics.h"

#include <libc/stdio.h>
#include <libc/string.h>

static char* format_to_string(pixel_format fmt)
{
    switch (fmt)
    {
    case GRAY1:
        return "GRAY1";
    case GRAY4:
        return "GRAY4";
    case GRAY8:
        return "GRAY8";
    case RGB12:
        return "RGB12";
    case BGR12:
        return "BGR12";
    case RGBA16:
        return "RGBA16";
    case BGRA16:
        return "BGRA16";
    case RGB24:
        return "RGB24";
    case BGR24:
        return "BGR24";
    case RGBA32:
        return "RGBA32";
    case BGRA32:
        return "BGRA32";
    default:
        return "Unknown Format";
    }
}

// Free the pixel buffer
void free_pixel_buffer(struct pixel_buffer buffer)
{
    // Free the raw buffer stored within
    free(buffer.raw_buffer);
}

// Allocate a new pixel buffer with the given format, returns null on failure
struct pixel_buffer alloc_pixel_buffer(pixel_format fmt, size_t width, size_t height)
{
    void* buffer = malloc((GET_BITS_PER_PIXEL(fmt) * width * height + 7) / 8);

    struct pixel_buffer pixel_buf = (struct pixel_buffer){.fmt = fmt, .width = width, .height = height, .raw_buffer = buffer, .line_length = GET_BITS_PER_PIXEL(fmt) * width};
    return pixel_buf;
}

// Attempts to convert the format of the pixel_buffer. Note that this function allocates a new buffer, meaning the original buffer must be freed seperately.
int convert_pixel_buffer(pixel_format dest_format, struct pixel_buffer* dest, struct pixel_buffer* src)
{
    *dest = alloc_pixel_buffer(dest_format, src->width, src->height);

    if (dest_format == src->fmt)
    {
        memcpy(dest->raw_buffer, src->raw_buffer, (GET_BITS_PER_PIXEL(dest_format) * src->width * src->height + 7) / 8);
    }
    else
    {
        printf("Unable to convert from format %s to format %s, not yet implemented.\n", format_to_string(src->fmt), format_to_string(dest_format));
        exit(1);
    }

    return 0;
}


// Blit a subset of one pixel buffer to a subset of another, returns 0 on success, nonzero on failure
int blit_buffer(struct pixel_buffer* dest, struct pixel_buffer* src, size_t dest_x, size_t dest_y, size_t src_x, size_t src_y, size_t width, size_t height)
{
    // Ensure the same format is used for the destination and source buffers
    if (dest->fmt != src->fmt)
    {
        printf("Unable to blit from format %s to format %s.\n", format_to_string(src->fmt), format_to_string(dest->fmt));
        exit(1);
    }

    // Ensure the buffers are of the proper size to allow the blit to occur
    // TODO
    printf("TODO!\n");

    // Get the number of bits per pixel
    size_t bpp = GET_BITS_PER_PIXEL(dest->fmt);

    if (bpp % 8 != 0)
    {
        printf("Blitting non-byte aligned pixel data is not yet supported.\n");
        exit(1);
    }

    for (size_t y = 0; y < height; y++)
    {
        void* src_line = src->raw_buffer + (src_y + y) * src->line_length / 8;
        void* dest_line = dest->raw_buffer + (dest_y + y) * dest->line_length / 8;

        memcpy(dest_line + dest_x * bpp / 8, src_line + src_x * bpp / 8, width * bpp / 8);
    }

    return 0;
}