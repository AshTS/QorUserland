#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <libc/stdint.h>
#include <libc/stdlib.h>

#include "bitstream.h"

struct huffman_node
{
    struct huffman_node* left;
    struct huffman_node* right;
    uint16_t value;
};

void huffman_free(struct huffman_node* ptr);
struct huffman_node* huffman_new();
int32_t huffman_insert(struct huffman_node* ptr, size_t code, size_t bits, uint16_t value);
struct huffman_node* huffman_follow(struct huffman_node* ptr, uint8_t bit, uint16_t* value);

uint8_t huffman_decode(struct huffman_node* ptr, struct bitstream* stream, uint16_t* value);

struct huffman_node* huffman_from_bit_lengths(uint8_t* bit_lengths, uint16_t* alphabet, size_t n);

#endif // HUFFMAN_H