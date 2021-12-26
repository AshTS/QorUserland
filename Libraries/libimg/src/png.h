#ifndef PNG_H
#define PNG_H

#include "libimg.h"

// Load a portable network graphic image from a buffer into an image data buffer
int image_backend_png(void* buffer, struct image_data* data);

#endif