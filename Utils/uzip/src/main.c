#include <libc/errno.h>
#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <libc/string.h>

#include "bitstream.h"
#include "consts.h"
#include "deflate.h"
#include "huffman.h"

void test();

// Main Entry Point
int main(int argc, char** argv)
{
    // Output file name
    const char* output_filename = "out";

    // Check to see if we are given the filename to read in
    if (argc < 2)
    {
        printf("Please provide a file to decompress.\n");
        return 1;
    }

    // Get the filename as its own variable
    const char* filename = argv[1];

    // Before doing any reading from files, make sure errno is in a known state
    errno = 0;

    // Attempt to open the file
    FILE* file = fopen(filename, "rb");
    
    // Print out an error message if an error occurs
    if (file == NULL || errno != 0)
    {
        printf("Unable to open `%s`: %s\n", filename, strerror(errno));
        return 2;
    }

    // Read the first 4KiB of the file into memory, this will hopefully be enough for now
    // TODO: Change this to dynamically allocate the size of the file
    uint8_t buffer[4096];
    fread(buffer, 1, 4096, file);

    // Print out an error message if the read failed
    if (errno != 0)
    {
        printf("Unable to read from file: %s\n", strerror(errno));
        return 3;
    }

    // Close the file handle we were given
    fclose(file);

    // Print out an error message if we were unable to close the file
    if (errno != 0)
    {
        printf("Unable to close file: %s\n", strerror(errno));
        return 4;
    }

    // Check to see if the file is a gzip archive by checking the magic number
    if (buffer[0] != GZIP_MAGIC0 || buffer[1] != GZIP_MAGIC1)
    {
        printf("File is not a gzip archive\n");
        return 5;
    }

    // Make sure the archive was compressed using DEFLATE
    if (buffer[2] != CM_DEFLATE)
    {
        printf("File was not compressed using deflate\n");
        return 6;
    }

    // Store the compression flags
    uint8_t flags = buffer[3];

    // Offset into the buffer from the file
    size_t offset = 4;

    // Skip past the modified time, xfl, and os fields
    offset += 4 + 1 + 1;

    // Skip past any extra data stored within the header
    if (flags & FEXTRA)
    {
        // Extract the length of the extra data
        uint16_t length = *(uint16_t*)(buffer + offset);

        // Skip past the length parameter and the extra data
        offset += 2 + length;
    }

    // Skip past the original filename
    if (flags & FNAME)
    {
        // Extract the name
        const char* name = (const char*)buffer + offset;

        output_filename = name;

        // Print the name
        // printf("Original Name: `%s`\n", name);

        // Move past the name
        offset += strlen(name) + 1;
    }
    
    // Skip past any comments stored in the header
    if (flags & FCOMMENT)
    {
        // Extract the comment
        const char* comment = (const char*)buffer + offset;

        // Print the comment
        // printf("Comment: `%s`\n", comment);

        // Move past the comment
        offset += strlen(comment) + 1;
    }

    // Skip past the crc data if present
    if (flags & FHCRC)
    {
        // Skip the 16 bits
        offset += 2;
    }

    // Now, finally we are at the compressed data
    size_t length;
    uint8_t* data = deflate_decompress(buffer + offset, &length);

    // Check if the decompressed data is NULL, and display an error message if so
    if (data == NULL)
    {
        printf("Unable to decompress data.\n");
        return 7;
    }

    // Open the output file
    FILE* outf = fopen(output_filename, "wb");

    // Print out an error message if an error occurs
    if (outf == NULL || errno != 0)
    {
        printf("Unable to open `%s`: %s\n", output_filename, strerror(errno));
        return 8;
    }

    fwrite(data, 1, length, outf);

    // Print out an error message if the write failed
    if (errno != 0)
    {
        printf("Unable to write to file: %s\n", strerror(errno));
        return 9;
    }

    // Close the file handle we were given
    fclose(outf);

    // Print out an error message if we were unable to close the file
    if (errno != 0)
    {
        printf("Unable to close file: %s\n", strerror(errno));
        return 10;
    }

    // Free the buffer with the decompressed data
    free(data);

    return 0;
}
