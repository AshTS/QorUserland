#ifndef _LIBGRAPHICS_H
#define _LIBGRAPHICS_H

#define LIBGRAPHICS_UNINITIALIZED_FRAMEBUFFER 1
#define LIBGRAPHICS_UNMAPPED_FRAMEBUFFER 2
#define LIBGRAPHICS_UNABLE_TO_OPEN_FRAMEBUFFER 3
#define LIBGRAPHICS_UNABLE_TO_MAP_FRAMEBUFFER 4

#define COLOR_BLACK (struct Pixel){.r=0, .g=0, .b=0, .a=255}
#define COLOR_WHITE (struct Pixel){.r=255, .g=255, .b=255, .a=255}
#define COLOR_GREY (struct Pixel){.r=128, .g=128, .b=128, .a=255}
#define COLOR_RED (struct Pixel){.r=255, .g=0, .b=0, .a=255}
#define COLOR_GREEN (struct Pixel){.r=0, .g=255, .b=0, .a=255}
#define COLOR_BLUE (struct Pixel){.r=0, .g=0, .b=255, .a=255}
#define COLOR_MAGENTA (struct Pixel){.r=255, .g=0, .b=255, .a=255}
#define COLOR_YELLOW (struct Pixel){.r=255, .g=255, .b=0, .a=255}
#define COLOR_CYAN (struct Pixel){.r=0, .g=255, .b=255, .a=255}
#define COLOR_LIGHT_RED (struct Pixel){.r=255, .g=128, .b=128, .a=255}
#define COLOR_LIGHT_GREEN (struct Pixel){.r=128, .g=255, .b=128, .a=255}
#define COLOR_LIGHT_BLUE (struct Pixel){.r=128, .g=128, .b=255, .a=255}
#define COLOR_LIGHT_MAGENTA (struct Pixel){.r=255, .g=128, .b=255, .a=255}
#define COLOR_LIGHT_YELLOW (struct Pixel){.r=255, .g=255, .b=128, .a=255}
#define COLOR_LIGHT_CYAN (struct Pixel){.r=128, .g=255, .b=255, .a=255}

#include <libc/stdlib.h>
#include <libc/stddef.h>
#include "libimg.h"

#include "pixelbuffer.h"

extern int LIBGRAPHICS_ERROR;

struct Pixel
{
    char r;
    char g;
    char b;
    char a;
};

int init_framebuffer();
int close_framebuffer();

struct Pixel* get_framebuffer();
struct pixel_buffer get_pixel_framebuffer();

int compute_location(int x, int y);

int run_shader(struct Pixel (shader)(int, int));
int run_individual_shader(struct Pixel (shader)(int, int));
int flush_framebuffer();

char* graphics_strerror(int error);
void graphics_perror();

int blit(struct pixel_buffer* data, size_t x, size_t y);


// Free the pixel buffer
void free_pixel_buffer(struct pixel_buffer buffer);

// Allocate a new pixel buffer with the given format, returns null on failure
struct pixel_buffer alloc_pixel_buffer(pixel_format fmt, size_t width, size_t height);

// Attempts to convert the format of the pixel_buffer. Note that this function allocates a new buffer, meaning the original buffer must be freed seperately.
int convert_pixel_buffer(pixel_format dest_format, struct pixel_buffer* dest, struct pixel_buffer* src);

// Blit a subset of one pixel buffer to a subset of another, returns 0 on success, nonzero on failure
int blit_buffer(struct pixel_buffer* dest, struct pixel_buffer* src, size_t dest_x, size_t dest_y, size_t src_x, size_t src_y, size_t width, size_t height);


#endif // _LIBGRAPHICS_H