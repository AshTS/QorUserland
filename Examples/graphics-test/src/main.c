#include <libc/stdbool.h>
#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <libc/string.h>
#include <libc/sys/syscalls.h>

#include "graphics.h"

int main(int argc, char** argv)
{
    init_framebuffer();

    struct Pixel* fb = get_framebuffer();

    if (fb == 0)
    {
        printf("Unable to access framebuffer\n");
    }

    struct pixel_buffer font;
    int result = load_image("/usr/share/font.png", &font);

    if (result)
    {
        printf("Unable to load font image.\n");
        return 1;
    }

    static const char* text = "Hello World!";
    struct pixel_buffer buf = get_pixel_framebuffer();

    size_t length = strlen(text);

    for (size_t i = 0; i < length; i++)
    {
        char c = text[i];
        size_t x = 3 + (c % 32) * 9;
        size_t y = 3 + (c / 32) * 16;

        blit_buffer(&buf, &font, 9*i, 0, x, y, 9, 16);
    }

    flush_framebuffer();

    close_framebuffer();

    return 0;
}
