#include "bitstream.h"

#include <libc/assert.h>

// Read a single bit from the bitstream
uint8_t read_bit(struct bitstream* s)
{
    uint8_t bit = ((*((uint8_t*)s->ptr + s->byte)) >> s->bit) & 1;

    s->bit += 1;

    if (s->bit == 8) 
    {
        s->byte++;
        s->bit = 0;
    }

    return bit;
}

// Read multiple bits from the bitstream, placing the most recent read into the most significant bit (maximum of 8 bits can be read with this function)
uint8_t read_bits(struct bitstream* s, size_t count)
{
    assert(count <= 8);

    uint8_t result = 0;

    for (size_t i = 0; i < count; i++)
    {
        result |= read_bit(s) << i;
    }

    return result;
}

// Read multiple bits from the bitstream, placing the most recent read into the most significant bit (maximum of 16 bits can be read with this function)
uint16_t read_bits16(struct bitstream* s, size_t count)
{
    assert(count <= 8);

    uint16_t result = 0;

    for (size_t i = 0; i < count; i++)
    {
        result |= (uint16_t)read_bit(s) << i;
    }

    return result;
}

// Flush the bit stream such that it is pointing to a byte boundary
void flush_to_next_byte(struct bitstream* s)
{
    if (s->bit != 0)
    {
        s->bit = 0;
        s->byte++;
    }
}

// Read a byte from the bitstream
uint8_t read_byte(struct bitstream* s)
{
    return ((uint8_t*)s->ptr)[s->byte++];
}

// Read a short from the bitstream
uint16_t read_short(struct bitstream* s)
{
    size_t offset = s->byte;
    s->byte += 2;
    return ((uint8_t*)s->ptr)[offset];
}