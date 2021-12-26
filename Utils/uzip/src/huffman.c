#include "huffman.h"

#include <libc/stdio.h>
#include <libc/stdlib.h>

#define MAX(a, b) ((a) < (b) ? (b) : (a))

void huffman_free(struct huffman_node* ptr)
{
    if (ptr != NULL)
    {
        huffman_free(ptr->left);
        huffman_free(ptr->right);

        free(ptr);
    }
}

struct huffman_node* huffman_new()
{
    struct huffman_node* node = malloc(sizeof(struct huffman_node));

    node->left = NULL;
    node->right = NULL;
    node->value = 0;

    return node;
}

int32_t huffman_insert(struct huffman_node* ptr, size_t code, size_t bits, uint16_t value)
{
    if (bits == 0)
    {
        ptr->value = value;
        return 0;
    }

    uint8_t bit = (code >> (bits - 1)) & 1;

    struct huffman_node** side;

    if (bit)
    {
        side = &ptr->right;
    }
    else
    {
        side = &ptr->left;
    }

    if (*side == NULL)
    {
        *side = huffman_new();
    }

    return huffman_insert(*side, code, bits - 1, value);
}

struct huffman_node* huffman_follow(struct huffman_node* ptr, uint8_t bit, uint16_t* value)
{
    if (bit)
    {
        if (ptr->right != NULL)
        {
            if (ptr->right->left == NULL && ptr->right->right == NULL)
            {
                *value = ptr->right->value;
                return ptr;
            }
        }
        return ptr->right;
    }
    else
    {
        if (ptr->left != NULL)
        {
            if (ptr->left->left == NULL && ptr->left->right == NULL)
            {
                *value = ptr->left->value;
                return ptr;
            }
        }
        return ptr->left;
    }
}

uint8_t huffman_decode(struct huffman_node* ptr, struct bitstream* stream, uint16_t* value)
{
    for(;;)
    {
        struct huffman_node* next_ptr = huffman_follow(ptr, read_bit(stream), value);

        if (next_ptr == ptr)
        {
            return 0;
        }

        if (next_ptr == NULL)
        {
            printf("Got to a bad symbol, unable to continue decoding!\n");
            return 1;
        }

        ptr = next_ptr;
    }
}


/*
    This function has been adapted from this blog post https://pyokagan.name/blog/2019-10-18-zlibinflate/ and converted from python to C
*/

struct huffman_node* huffman_from_bit_lengths(uint8_t* bit_lengths, uint16_t* alphabet, size_t n)
{
    // MAX_BITS = max(bl)

    uint8_t max_bits = bit_lengths[0];

    for (size_t i = 1; i < n; i++)
    {
        if (bit_lengths[i] > max_bits)
        {
            max_bits = bit_lengths[i];
        }
    }

    // bl_count = [sum(1 for x in bl if x == y and y != 0) for y in range(MAX_BITS+1)]

    size_t* bl_count = malloc((max_bits + 1) * sizeof(size_t));

    for (size_t i = 0; i < (max_bits + 1); i++)
    {
        bl_count[i] = 0;
    }

    for (size_t i = 0; i < n; i++)
    {
        bl_count[bit_lengths[i]]++;
    }

    bl_count[0] = 0;


    // next_code = [0, 0]

    size_t next_code_size = 2;

    if (max_bits + 1 > 2)
    {
        next_code_size = max_bits + 1;
    }

    uint16_t* next_code = malloc(next_code_size * sizeof(uint16_t));

    next_code[0] = 0;
    next_code[1] = 0;

    // for bits in range(2, MAX_BITS+1):
    for (size_t bits = 2; bits < max_bits + 1; bits++)
    {
    //     next_code.append((next_code[bits-1] + bl_count[bits-1]) << 1)
        next_code[bits] = (next_code[bits-1] + bl_count[bits-1]) << 1;
    }
    
    // t = HuffmanTree()
    struct huffman_node* t = huffman_new();

    // for c, bitlen in zip(alphabet, bl):
    for (size_t i = 0; i < n; i++)
    {
        uint8_t bitlen = bit_lengths[i];
        uint16_t c = alphabet[i];

    //     if bitlen != 0:
        if (bit_lengths[i] != 0)
        {
    //         t.insert(next_code[bitlen], bitlen, c)
            huffman_insert(t, next_code[(size_t)bitlen], bitlen, c);
    //         next_code[bitlen] += 1
            next_code[(size_t)bitlen]++;
        }
    }

    free(bl_count);
    free(next_code);
    
    // return t
    return t;
}