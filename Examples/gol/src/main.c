#include <libc/stdbool.h>
#include <libc/stdio.h>
#include <libc/stdlib.h>

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
    printf("Random: %i\n", rand());

    for (int x = 0; x < WIDTH; x++)
    {
        for (int y = 0; y < HEIGHT; y++)
        {
            grid0[x + y * WIDTH] = (rand() & 1);
        }
    }

    /*
    grid0[500] = true;
    grid0[500 + WIDTH + 1] = true;
    grid0[500 + WIDTH * 2 - 1] = true;
    grid0[500 + WIDTH * 2] = true;
    grid0[500 + WIDTH * 2 + 1] = true;*/

    grid = grid0;
    backup = grid1;

    for (int i = 0; i < 512; i++)
    {
        if (run_shader(shader) != 0)
        {
            graphics_perror();
        }
        tick_and_swap();
    }

    return 0;
}


struct Pixel shader(int x, int y)
{
    if (grid[(x / SCALE) + (y / SCALE)* WIDTH])
    {
        return COLOR_WHITE;
    }
    else
    {
        return COLOR_BLACK;
    }
}


void tick_and_swap()
{
    for (int x = 0; x < WIDTH; x++)
    {
        for (int y = 0; y < HEIGHT; y++)
        {
            int count = 0;

            for (int dx = -1; dx <= 1; dx++)
            {
                for (int dy = -1; dy <= 1; dy++)
                {
                    if (dx == 0 && dy == 0) continue;

                    int this_x = (x + dx + WIDTH) % WIDTH;
                    int this_y = (y + dy + HEIGHT) % HEIGHT;
                    
                    if (grid[this_x + this_y * WIDTH]) count++;
                }
            }
            
            if (grid[x + y * WIDTH])
            {
                backup[x + y*WIDTH] = count == 2 || count == 3;
            }
            else
            {
                backup[x + y*WIDTH] = count == 3;
            }
        }
    }

    bool* t = backup;

    backup = grid;
    grid = t;
}