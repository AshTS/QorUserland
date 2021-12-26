#ifndef LIBIMG_H
#define LIBIMG_H

#include <libc/stddef.h>

struct image_data
{
    size_t width;
    size_t height;
    void* buffer;
};

// Load an image from file into a buffer which can later be free()ed. Returns
// -1 on failure, and 0 on success.
int load_image(const char* filename, struct image_data* data);

#endif // LIBIMG_H