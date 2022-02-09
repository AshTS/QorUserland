#ifndef LIBIMG_H
#define LIBIMG_H

#include <libc/stddef.h>

#include "pixelbuffer.h"

// Load an image from file into a buffer which can later be free()ed. Returns
// -1 on failure, and 0 on success.
int load_image(const char* filename, struct pixel_buffer* data);

// Load an image in the given format into a buffer which can later be free()ed.
// Returns -1 on failure and 0 on success.
int load_image_format(const char* filename, struct pixel_buffer* data, pixel_format fmt);

#endif // LIBIMG_H