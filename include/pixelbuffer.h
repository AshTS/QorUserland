#ifndef PIXELBUFFER_H
#define PIXELBUFFER_H

// Bit 0: 1
// Bit 1: 4
// Bit 2: 8
// Bit 3: 16
// Bit 4: 32

#define BPP1        0x1     // 0b00001
#define BPP4        0x2     // 0b00010
#define BPP5        0x3     // 0b00011
#define BPP8        0x4     // 0b00100
#define BPP12       0x6     // 0b00110
#define BPP16       0x8     // 0b01000
#define BPP24       0xC     // 0b01100
#define BPP32       0x10    // 0b10000

// Bit 5: 1
// Bit 6: 3

#define CHNLS1      1 << 5  // 0b01 << 5
#define CHNLS3      2 << 5  // 0b10 << 5
#define CHNLS4      3 << 5  // 0b11 << 5

// Bit 7: Reverse Order of RGB

#define ORDER_RGB   0 << 7  // 0b0 << 7
#define ORDER_BGR   1 << 7  // 0b1 << 7

#define GRAY1       ORDER_RGB | CHNLS1 | BPP1
#define GRAY4       ORDER_RGB | CHNLS1 | BPP4
#define GRAY8       ORDER_RGB | CHNLS1 | BPP8

#define RGB12       ORDER_RGB | CHNLS3 | BPP12
#define BGR12       ORDER_BGR | CHNLS3 | BPP12

#define RGBA16      ORDER_RGB | CHNLS4 | BPP16
#define BGRA16      ORDER_BGR | CHNLS4 | BPP16

#define RGB24       ORDER_RGB | CHNLS3 | BPP24
#define BGR24       ORDER_BGR | CHNLS3 | BPP24

#define RGBA32      ORDER_RGB | CHNLS4 | BPP32
#define BGRA32      ORDER_BGR | CHNLS4 | BPP32

#define GET_BITS_PER_PIXEL(v) ((((v) & 0x1E) << 1) + ((v) & 1))
#define GET_NUM_CHANNELS(v)   ((((v) & 0x20) >> 5) + 3*(((v) & 0x40) >> 6))

#if GET_BITS_PER_PIXEL(BPP1) != 1
#error "BPP1"
#endif


#if GET_BITS_PER_PIXEL(BPP4) != 4
#error "BPP4"
#endif

#if GET_BITS_PER_PIXEL(BPP5) != 5
#error "BPP5"
#endif

#if GET_BITS_PER_PIXEL(BPP8) != 8
#error "BPP8"
#endif

#if GET_BITS_PER_PIXEL(BPP12) != 12
#error "BPP12"
#endif

#if GET_BITS_PER_PIXEL(BPP16) != 16
#error "BPP16"
#endif

#if GET_BITS_PER_PIXEL(BPP24) != 24
#error "BPP24"
#endif

#if GET_BITS_PER_PIXEL(BPP32) != 32
#error "BPP32"
#endif


#if GET_NUM_CHANNELS(CHNLS1) != 1
#error "CHNLS1"
#endif

#if GET_NUM_CHANNELS(CHNLS3) != 3
#error "CHNLS3"
#endif

#if GET_NUM_CHANNELS(CHNLS4) != 4
#error "CHNLS4"
#endif

// Pixel Color Format
typedef uint8_t pixel_format;

// Pixel buffer with variable format and line length
struct pixel_buffer
{
    pixel_format fmt; // Pixel format

    size_t width; // Width in pixels
    size_t height; // Height in pixels

    size_t line_length; // Length of each line in bits

    void* raw_buffer; // Raw buffer
};

struct pixel_rgba32
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

#endif // PIXELBUFFER_H