#include "png.h"

#include <libc/assert.h>
#include <libc/stdio.h>

#include "libimg.h"

// Load a portable network graphic image from a buffer into an image data buffer
int image_backend_png(void* buffer, struct image_data* data)
{
    printf("Hey, that's a png!\n");
    assert(0);
}