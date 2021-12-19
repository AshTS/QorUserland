#include <libc/stdbool.h>
#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <libc/sys/syscalls.h>

#include "graphics.h"

#define SCALE 8
#define WIDTH (640 / SCALE)
#define HEIGHT (480 / SCALE)

bool grid0[WIDTH * HEIGHT];
bool grid1[WIDTH * HEIGHT];

bool* grid = 0;
bool* backup = 0;

struct Pixel shader(int, int);

void tick_and_swap();

int main(int argc, char** argv)
{
    init_framebuffer();

    struct Pixel* fb = get_framebuffer();

    for (int x = 0; x < 256; x++)
    {
        for (int y = 0; y < 256; y++)
        {
            fb[compute_location(320 - 128 + x, 240 - 128 + y)] = COLOR_WHITE;
        }
    }

    flush_framebuffer();

    close_framebuffer();

    return 0;
}