#include "buf.h"

#include <libc/string.h>

// Expand an expandable buffer
void expand_buffer(struct exp_buffer* b)
{
    // Allocate a new buffer with twice the size
    size_t new_size = b->size * 2;
    uint8_t* new_buf = malloc(new_size);

    // Move the old data over
    memcpy(new_buf, b->buf, b->index);

    // Free the old buffer
    free(b->buf);

    // Update the buffer pointer and size variables
    b->buf = new_buf;
    b->size = new_size;
}

// Add a byte to the expandable buffer
void append_byte_to_buffer(struct exp_buffer* buf, uint8_t byte)
{
    // Make sure there is enough space in the buffer
    while (buf->size <= buf->index)
    {
        expand_buffer(buf);
    }

    // Insert the new byte
    buf->buf[buf->index++] = byte;
}

// Create a new expandable buffer
struct exp_buffer new_exp_buffer(size_t size)
{
    struct exp_buffer buf = (struct exp_buffer){.buf = malloc(size), .size = size, .index = 0};

    return buf;
}
