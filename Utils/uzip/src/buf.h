#ifndef BUF_H
#define BUF_H

#include <libc/stdint.h>
#include <libc/stdlib.h>

struct exp_buffer
{
    uint8_t* buf;
    size_t size;
    size_t index;
};

void expand_buffer(struct exp_buffer* b);
void append_byte_to_buffer(struct exp_buffer* buf, uint8_t byte);
struct exp_buffer new_exp_buffer(size_t size);

#endif // BUF_H