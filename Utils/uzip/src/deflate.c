#include "deflate.h"

#include <libc/assert.h>
#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <libc/string.h>

#include "bitstream.h"
#include "buf.h"
#include "huffman.h"

// #define DEBUG_MSG(...) printf("  [DEBUG] "__VA_ARGS__)

#define DEBUG_MSG(...)

/*
    Much of this code has been adapted from this blog post https://pyokagan.name/blog/2019-10-18-zlibinflate/ and converted from python to C
*/

static size_t LengthExtraBits[29] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
static size_t LengthBase[29] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
static size_t DistanceExtraBits[30] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};
static size_t DistanceBase[30] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};

static struct huffman_node* default_lit_len_tree;
static struct huffman_node* default_dist_tree;

void init_default_trees()
{
    static uint8_t done = 0;
    if (!done)
    {
        uint8_t bl[288];
        uint16_t alphabet[288];
        
        size_t i = 0;

        for (; i < 144; i++)
        {
            bl[i] = 8;
            alphabet[i] = i;
        }

        for (; i < 256; i++)
        {
            bl[i] = 9;
            alphabet[i] = i;
        }

        for (; i < 280; i++)
        {
            bl[i] = 7;
            alphabet[i] = i;
        }

        for (; i < 288; i++)
        {
            bl[i] = 8;
            alphabet[i] = i;
        }

        default_lit_len_tree = huffman_from_bit_lengths(bl, alphabet, 286);

        for (size_t i = 0; i < 30; i++)
        {
            bl[i] = 5;
        }

        default_dist_tree = huffman_from_bit_lengths(bl, alphabet, 30);

        done = 1;
    }
}

// Block types
enum blocktype
{
    NOCOMPRESSION = 0b00,
    FIXEDCOMPRESSION = 0b01,
    DYNAMICCOMPRESSION = 0b10
};

// Decompress a single block
uint8_t decompress_block(struct bitstream* stream, struct exp_buffer* buf);

// Decompress a non-compressed block
uint8_t decompress_non_compressed_block(struct bitstream* stream, struct exp_buffer* buf);

uint8_t decompress_default_compressed_block(struct bitstream* stream, struct exp_buffer* buf);

// Decompress a block compressed with dynamic huffman codings
uint8_t decompress_dynamic_compressed_block(struct bitstream* stream, struct exp_buffer* buf);

// Decompress data stored in the DEFLATE format, this will return a buffer
// which needs to be free()ed at a later point to avoid a memory leak, this
// function will return a null pointer if the decompression fails.
uint8_t* deflate_decompress(void* data, size_t* length)
{
    init_default_trees();
    DEBUG_MSG("Attempting to decompress data.\n");

    // Convert the pointer to a bit stream
    DEBUG_MSG("Converting to bit stream\n");
    struct bitstream stream = (struct bitstream){.ptr = data, .byte = 0, .bit = 0};

    // Return data buffer
    struct exp_buffer result = new_exp_buffer(1024);

    // While there is still data to decompress, continue doing so
    while (!decompress_block(&stream, &result));

    *length = result.index;
    return result.buf;
}


// Decompress a single block
uint8_t decompress_block(struct bitstream* stream, struct exp_buffer* buf)
{
    DEBUG_MSG("Decompressing block at byte %ld, bit %ld\n", stream->byte, stream->bit);

    // Get the final block flag
    uint8_t final = read_bit(stream);

    // Get the block type
    enum blocktype block_type = read_bits(stream, 2);

    // Decompress the proper kind of block
    switch (block_type)
    {
        case NOCOMPRESSION:
            decompress_non_compressed_block(stream, buf);
            break;

        case FIXEDCOMPRESSION:
            decompress_default_compressed_block(stream, buf);
            break;

        case DYNAMICCOMPRESSION:
            decompress_dynamic_compressed_block(stream, buf);
            break;

        default: 
            printf("Unknown block type: %x\n", block_type);
            assert(0);
            break;
    }

    return final;
}


// Decompress a non-compressed block
uint8_t decompress_non_compressed_block(struct bitstream* stream, struct exp_buffer* buf)
{
    // Extract the length and negated lengths
    uint16_t len = read_short(stream);
    uint16_t nlen = read_short(stream);

    // Make sure the length and negated length match
    if (len != ~nlen)
    {
        printf("LEN and NLEN do not match:\n  Len: %x\n NLen: %x\n", len, nlen);
        assert(0);
    }

    // Copy bytes over to the expandable buffer
    for (size_t i = 0; i < len; i++)
    {
        append_byte_to_buffer(buf, read_byte(stream));
    }

    return 0;
}

// Decompress a compressed block with the given trees
uint8_t decompress_compressed_with(struct bitstream* stream, struct huffman_node* lit_len_tree, struct huffman_node* dist_tree, struct exp_buffer* buf)
{
    // while True:
    for (;;)
    {
    //     sym = decode_symbol(r, literal_length_tree)
        uint16_t sym;
        uint8_t result = huffman_decode(lit_len_tree, stream, &sym);
        if (result) return result;

    //     if sym <= 255: # Literal byte
        if (sym <= 255)
        {
    //         out.append(sym)
            append_byte_to_buffer(buf, (uint8_t)sym);
        }
    
    //     elif sym == 256: # End of block
        else if (sym == 256) return 0;
    //         return
    //     else: # <length, backward distance> pair
        else
        {
    //         sym -= 257
            sym -= 257;
    //         length = r.read_bits(LengthExtraBits[sym]) + LengthBase[sym]
            size_t length = (size_t)read_bits16(stream, LengthExtraBits[(size_t)sym]) + LengthBase[sym];
    //         dist_sym = decode_symbol(r, distance_tree)
            uint16_t dist_sym;
            result = huffman_decode(dist_tree, stream, &dist_sym);
            if (result) return result;
    //         dist = r.read_bits(DistanceExtraBits[dist_sym]) + DistanceBase[dist_sym]
            size_t dist = (size_t)read_bits16(stream, DistanceExtraBits[(size_t)dist_sym]) + DistanceBase[dist_sym];
    //         for _ in range(length):

            for (size_t i = 0; i < length; i++)
            {
    //             out.append(out[-dist])
                uint8_t b = buf->buf[buf->index - dist];
                append_byte_to_buffer(buf, b);
            }
        }
    }
}

// Decompress a block compressed with the default huffman codings
uint8_t decompress_default_compressed_block(struct bitstream* stream, struct exp_buffer* buf)
{
    return decompress_compressed_with(stream, default_lit_len_tree, default_dist_tree, buf);
}

// Decode the trees stored at the beginning of dynamic compressed blocks
uint8_t decode_trees(struct huffman_node** lit_len_tree, struct huffman_node** dist_tree, struct bitstream* stream)
{
    static size_t CodeLengthCodesOrder[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    // # The number of literal/length codes
    // HLIT = r.read_bits(5) + 257
    size_t hlit = (size_t)read_bits16(stream, 5) + 257;

    // # The number of distance codes
    // HDIST = r.read_bits(5) + 1
    size_t hdist = (size_t)read_bits16(stream, 5) + 1;

    // # The number of code length codes
    // HCLEN = r.read_bits(4) + 4
    size_t hclen = (size_t)read_bits16(stream, 4) + 4;

    // # Read code lengths for the code length alphabet
    // code_length_tree_bl = [0 for _ in range(19)]
    uint8_t code_length_tree_bl[19];

    for (size_t i = 0; i < 19; i++)
    {
        code_length_tree_bl[i] = 0;
    }

    // for i in range(HCLEN):
    for (size_t i = 0; i < hclen; i++)
    {
    //     code_length_tree_bl[CodeLengthCodesOrder[i]] = r.read_bits(3)
        uint16_t d = read_bits16(stream, 3);
        code_length_tree_bl[CodeLengthCodesOrder[i]] = d;
    }

    // # Construct code length tree
    // code_length_tree = bl_list_to_tree(code_length_tree_bl, range(19))
    uint16_t code_len_alphabet[19];

    for (size_t i = 0; i < 19; i++)
    {
        code_len_alphabet[i] = i;
    }

    struct huffman_node* code_length_tree = huffman_from_bit_lengths(code_length_tree_bl, code_len_alphabet, 19);

    // # Read literal/length + distance code length list
    // bl = []
    uint8_t bit_lengths[286 + 30];

    // while len(bl) < HLIT + HDIST:
    for (size_t i = 0; i < hlit + hdist; i++)
    {
    //     sym = decode_symbol(r, code_length_tree)
        uint16_t sym;
        uint8_t result = huffman_decode(code_length_tree, stream, &sym);
        if (result) return result;

    //     if 0 <= sym <= 15: # literal value
        if (0 <= sym && sym <= 15)
        {
    //         bl.append(sym)
            bit_lengths[i] = (uint8_t)sym;
        }

    //     elif sym == 16:
        else if (sym == 16)
        {
    //         # copy the previous code length 3..6 times.
    //         # the next 2 bits indicate repeat length ( 0 = 3, ..., 3 = 6 )
    //         prev_code_length = bl[-1]
            uint8_t prev_code_length = bit_lengths[i - 1];
    //         repeat_length = r.read_bits(2) + 3
            size_t repeat_length = (size_t)read_bits16(stream, 2) + 3;

    //         bl.extend(prev_code_length for _ in range(repeat_length))
            i--;
            for (size_t j = 0; j < repeat_length; j++)
            {
                bit_lengths[++i] = prev_code_length;
            }
        }
    //     elif sym == 17:
        else if (sym == 17)
        {
    //         # repeat code length 0 for 3..10 times. (3 bits of length)
    //         repeat_length = r.read_bits(3) + 3
            size_t repeat_length = (size_t)read_bits16(stream, 3) + 3;
    //         bl.extend(0 for _ in range(repeat_length))
            i--;
            for (size_t j = 0; j < repeat_length; j++)
            {
                bit_lengths[++i] = 0;
            }
        }
    //     elif sym == 18:
        else if (sym == 18)
        {
    //         # repeat code length 0 for 11..138 times. (7 bits of length)
    //         repeat_length = r.read_bits(7) + 11
            size_t repeat_length = (size_t)read_bits16(stream, 7) + 11;
    //         bl.extend(0 for _ in range(repeat_length))
            i--;
            for (size_t j = 0; j < repeat_length; j++)
            {
                bit_lengths[++i] = 0;
            }
        }
    //     else:
        else
        {
            printf("Bad symbol %i\n", sym);
            assert(0 && "Invalid Symbol");
        }
    //         raise Exception('invalid symbol')
    }

    uint16_t alphabet[286];

    for (size_t i = 0; i < 286; i++)
    {
        alphabet[i] = i;
    }

    // # Construct trees
    // literal_length_tree = bl_list_to_tree(bl[:HLIT], range(286))
    *lit_len_tree = huffman_from_bit_lengths(bit_lengths, alphabet, hlit);
    // distance_tree = bl_list_to_tree(bl[HLIT:], range(30))
    *dist_tree = huffman_from_bit_lengths(bit_lengths + hlit, alphabet, hdist);
    // return literal_length_tree, distance_tree

    return 0;
}

// Decompress a block compressed with dynamic huffman codings
uint8_t decompress_dynamic_compressed_block(struct bitstream* stream, struct exp_buffer* buf)
{
    struct huffman_node* lit_len_tree;
    struct huffman_node* dist_tree;

    decode_trees(&lit_len_tree, &dist_tree, stream);

    uint8_t result = decompress_compressed_with(stream, lit_len_tree, dist_tree, buf);

    huffman_free(lit_len_tree);
    huffman_free(dist_tree);

    return 0;
}