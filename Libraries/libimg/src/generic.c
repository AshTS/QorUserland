#include "libimg.h"

#include <libc/errno.h>
#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <libc/string.h>

#include "bmp.h"
#include "png.h"

// Load an image with a generic backend
int load_image_from_backend(int (*backend)(void*, struct image_data*), void* buffer, struct image_data* data)
{
    return backend(buffer, data);
}

// Load an image from file into a buffer which can later be free()ed. Returns
// -1 on failure, and 0 on success.
int load_image(const char* filename, struct image_data* image_data_ptr)
{
    const char* walk = filename;
    const char* suffix = strchr(walk, '.');

    while (suffix != NULL)
    {
        walk = suffix + 1;
        suffix = strchr(walk, '.');
    }

    int (*backend)(void*, struct image_data*);

    if (strcmp(walk, "bmp") == 0)
    {
        backend = image_backend_bmp;
    }
    else if (strcmp(walk, "png") == 0)
    {
        backend = image_backend_png;
    }
    else
    {
        return -1;
    }

    FILE* file = fopen(filename, "rb");
    if (file == NULL || errno != 0)
    {
        return -1;
    }

    void* buffer = malloc(1024 * 128);
    size_t length = fread(buffer, 1, 1024 * 128, file);
    if (errno != 0)
    {
        return -1;
    }

    fclose(file);
    if (errno != 0)
    {
        return -1;
    }

    if (backend(buffer, image_data_ptr) != 0)
    {
        return -1;
    }

    free(buffer);

    return 0;
}