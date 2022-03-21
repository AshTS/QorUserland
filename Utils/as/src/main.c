#include <libc/assert.h>
#include <libc/errno.h>
#include <libc/stdbool.h>
#include <libc/stdio.h>
#include <libc/string.h>
#include <libc/stdlib.h>

#include "argparse.h"

#include "as.h"
#include "assemble.h"

bool verbose_flag = false;

void show_usage(char*);
void assemble_filename(const char* name);

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
        printf("as: no input file given\n");
        exit(1);
    }
    else
    {
        while (files && *files)
        {
            assemble_filename((const char*)*files++);
        }
    }
}

void assemble_filename(const char* name)
{
    // Open the file handle
    FILE* file;
    errno = 0;
    file = fopen(name, "r");

    if (file == 0 || errno != 0)
    {
        printf("as: unable to open file `%s`: %s\n", name, strerror(errno));
        exit(1);
    }

    // Determine the output name
    char* dup_name = malloc(strlen(name) + 2);
    strcpy(dup_name, name);
    const char* output_name = strchr(dup_name, '/');
    if (output_name == NULL) { output_name = dup_name; } else { output_name++; }
    char* period = strchr(dup_name, '.');
    if (period) { *period = 0; }
    strcat(dup_name, ".o");

    // Execute the assembler
    assemble_file_handle(file, name, (const char*)output_name);

    // Free the duplicated name
    free(dup_name);

    // Close the file handle
    if (fclose(file) < 0)
    {
        printf("as: unable to close file `%s`: %s\n", name, strerror(errno));
        exit(1);
    }
}

void show_usage(char* prog_name)
{
    printf("Usage: %s [OPTIONS] ... [FILES] ...\n", prog_name);
    printf(" Assemble the FILES given into object files\n\n");
    printf("       -h --help          Show the usage\n");
    printf("       -v --verbose       Show a verbose output\n");
}