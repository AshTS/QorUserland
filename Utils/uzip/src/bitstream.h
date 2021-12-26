#ifndef BITSTREAM_H
#define BITSTREAM_H

#include <libc/stdint.h>
#include <libc/stddef.h>

struct bitstream
{
    void* ptr;
    size_t byte;
    size_t bit;
};

uint8_t read_bit(struct bitstream* s);
uint8_t read_bits(struct bitstream* s, size_t count);
uint16_t read_bits16(struct bitstream* s, size_t count);
void flush_to_next_byte(struct bitstream* s);
uint8_t read_byte(struct bitstream* s);
uint16_t read_short(struct bitstream* s);


#endif // BITSTREAM