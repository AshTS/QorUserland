#include <libc/stdbool.h>
#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <libc/string.h>
#include <libc/sys/syscalls.h>

#include "graphics.h"

void write_char(struct pixel_buffer* dest, struct pixel_buffer* src, char c, size_t x, size_t y);

int main(int argc, char** argv)
{
    init_framebuffer();

    struct Pixel* fb = get_framebuffer();

    if (fb == 0)
    {
        printf("Unable to access framebuffer\n");
    }

    struct pixel_buffer font;
    int result = load_image_format("/usr/share/font.png", &font, RGBA32);

    if (result)
    {
        printf("Unable to load font image.\n");
        return 1;
    }

    static const char* text = "Hello World!";
    struct pixel_buffer buf = get_pixel_framebuffer();

    size_t length = strlen(text);

    for (size_t i = 0; i < 100; i++)
    {
        write_char(&buf, &font, '0' + (i / 100) % 10, 2, 2);
        write_char(&buf, &font, '0' + (i / 10) % 10, 3, 2);
        write_char(&buf, &font, '0' + (i / 1) % 10, 4, 2);
        flush_framebuffer();

        struct time_repr t = (struct time_repr){.tv_nsec = 10000000};

        sys_nanosleep(&t, 0);
    }
    

    close_framebuffer();

    return 0;
}


void write_char(struct pixel_buffer* dest, struct pixel_buffer* src, char c, size_t x, size_t y)
{
        size_t x_v = 3 + (c % 32) * 9;
        size_t y_v = 3 + (c / 32) * 16;

        blit_buffer(dest, src, 9*x, 16*y, x_v, y_v, 9, 16);
}