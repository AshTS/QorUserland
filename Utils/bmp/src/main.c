#include <libc/errno.h>
#include <libc/stdio.h>
#include <libc/string.h>

#include <graphics.h>

#include "bmp.h"

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("bmp requires atleast one argument\n");
        return 1;
    }
    
    FILE* f = fopen(argv[1], "rb");
    if (errno != 0)
    {
        printf("Unable to open image `%s`: %s\n", argv[1], strerror(errno));
        return 1;
    }

    char* buffer = malloc(128 * 1024);

    int length = fread(buffer, 1, 128 * 1024, f);
    if (errno != 0)
    {
        printf("Unable to read from image `%s`: %s\n", argv[1], strerror(errno));
        fclose(f);
        return 1;
    }

    BitmapHeader* header = buffer;

    int width = header->width;
    int height = header->height;

    init_framebuffer();

    struct Pixel* fb = get_framebuffer();

    char* ptr = buffer + header->pixel_data_offset;

    for (int y = height - 1; y >= 0; y--)
    {
        for (int x = 0; x < width; x++)
        {
            struct Pixel color;
            color.a = 255;
            color.g = ptr[0];
            color.b = ptr[1];
            color.r = ptr[2];
            ptr += 3;
            fb[compute_location(320 - width / 2 + x, 240 - height / 2 + y)] = color;
        }

        ptr += (4 - ((int)(ptr - header->pixel_data_offset - buffer) % 4)) % 4;
    }

    flush_framebuffer();
    close_framebuffer();

    fclose(f);

    return 0;
}