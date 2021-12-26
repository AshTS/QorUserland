#include <libc/errno.h>
#include <libc/stdio.h>
#include <libc/string.h>
#include <libc/stdlib.h>

#include <graphics.h>
#include <libimg.h>

#include "bmp.h"

void blit(struct image_data* data, size_t x, size_t y);

int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("bmp requires atleast one argument\n");
        return 1;
    }
    
    struct image_data data;

    load_image(argv[1], &data);

    blit(&data, 320 - data.width / 2, 240 - data.height / 2);

    return 0;
}


void blit(struct image_data* data, size_t x, size_t y)
{
    init_framebuffer();

    struct Pixel* fb = get_framebuffer();

    size_t row_length = sizeof(struct Pixel) * data->width;

    for (size_t i = 0; i < data->height; i++)
    {
        memcpy(&fb[compute_location(x, y + i)], data->buffer + row_length * i, row_length);
    }

    flush_framebuffer();
    close_framebuffer();
}