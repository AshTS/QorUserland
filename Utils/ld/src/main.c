#include <libc/assert.h>
#include <libc/errno.h>
#include <libc/fcntl.h>
#include <libc/stdbool.h>
#include <libc/stdio.h>
#include <libc/string.h>
#include <libc/stdlib.h>
#include <libc/unistd.h>

#include "argparse.h"

#include "as.h"
#include "elf.h"
#include "database.h"
#include "vector.h"

uint8_t verbose_flag = false;

void show_usage(char*);
int output_elf_to_file(const char* filename);
int include_file(const char* filename);

int main(int argc, char** argv)
{
    // Parse command line arguments
    struct Arguments args;
    int arg_parse_result = arg_parse(&args, argc, argv);
    assert(!arg_parse_result);

    verbose_flag = arg_check_short(&args, 'v') || arg_check_long(&args, "verbose");

    if (arg_check_short(&args, 'h') || arg_check_long(&args, "help"))
    {
        show_usage(argv[0]);
        return 0;
    }

    char** files = arg_get_free(&args);

    if (*files == 0)
    {
        printf("ld: no input file given\n");
        exit(1);
    }
    else
    {
        while (files && *files)
        {
            if (include_file(*files++))
            {
                exit(1);
            }
        }

        dump_section_database();
        printf("\n");
        dump_symbol_database();
        printf("\n");
        dump_relocation_database();

        output_elf_to_file("out");
    }
}


void show_usage(char* prog_name)
{
    printf("Usage: %s [OPTIONS] ... [FILES] ...\n", prog_name);
    printf(" Link the object FILES given into an executable\n\n");
    printf("       -h --help          Show the usage\n");
    printf("       -v --verbose       Show a verbose output\n");
}

int include_file(const char* filename)
{
    LOG("Including File %s\n", filename);

    // Attempt to stat file
    struct stat stat_data;
    if (stat(filename, &stat_data))
    {
        printf("ld: Unable to open file %s: %s\n", filename, strerror(errno));
        return 1;
    }

    // We can then get the length of the file
    uint64_t length = stat_data.st_size;
    LOG("File length: %lu\n", length);

    // Now, allocate a buffer for the file data, this will not be freeed during execution of the program as this data must remain in memory so as to avoid copying it all
    uint8_t* buffer = malloc(length);

    // Attempt to open the file
    FILE* file = fopen(filename, "rb");
    if (errno)
    {
        printf("ld: Unable to open file %s: %s\n", filename, strerror(errno));
        return 1;
    }

    // Attempt to read from the file
    if (fread(buffer, length, 1, file) != 1)
    {
        printf("ld: Unable to read file %s: %s\n", filename, strerror(errno));
        return 1;
    }

    // Attempt to close the file
    if (fclose(file))
    {
        printf("ld: Unable to close file %s: %s\n", filename, strerror(errno));
        return 1;
    }

    return register_elf_buffer(buffer, filename);
}


int output_elf_to_file(const char* filename)
{
    // Construct the ELF file
    struct vector elf_buffer;
    
    if (link(&elf_buffer))
    {
        return 1;
    }

    FILE* f = fopen(filename, "wb");
    if (f == NULL)
    {
        printf("ld: Unable to open output file %s: %s\n", filename, strerror(errno));
        return 1;
    }


    if (fwrite(VEC_TO_ARRAY(elf_buffer, uint8_t), elf_buffer.length, 1, f) != 1)
    {
        printf("ld: Unable to write to file %s: %s\n", filename, strerror(errno));
        return 1;
    }

    if (fclose(f))
    {
        printf("ld: Unable to close file %s: %s\n", filename, strerror(errno));
        return 1;
    }

    return 0;
}