#ifndef ASSEMBLE_H
#define ASSEMBLE_H

#include <libc/stdio.h>

// Assemble a file from the file handle
void assemble_file_handle(FILE* file, const char* input_name, const char* output_name);

#endif // ASSEMBLE_H