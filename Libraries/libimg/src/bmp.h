#ifndef BMP_H
#define BMP_H

#include "libimg.h"

typedef struct bitmap_header
{
    // Common Header
    char magic0;
    char magic1;

    int file_size;
    int res_0;
    int pixel_data_offset;

    // Just the windows header
    int header_size;
    int width;
    int height;
    short color_panes;
    short bits_per_pixel;
    int compression_method;
    int image_data_size;
    int horiz_res;
    int vert_res;
    int color_count;
    int important_colors;
} __attribute__((packed)) BitmapHeader;

// Load a bitmap image from a buffer into an image data buffer
int image_backend_bmp(void* buffer, struct image_data* data);

#endif // BMP_H