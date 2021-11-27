#include <libc/assert.h>
#include <libc/stdio.h>
#include <libc/sys/syscalls.h>

#include "argparse.h"

void show_usage(char*);

int main(int argc, char** argv)
{
    // Parse command line arguments
    struct Arguments args;
    int arg_parse_result = arg_parse(&args, argc, argv);
    assert(!arg_parse_result);

    // If the help has been requested, show the usage
    if (arg_check_long(&args, "help") || arg_check_short(&args, 'h'))
    {
        show_usage(argv[0]);
        return 0;
    }

    // Get the directory names
    char** directory_names = arg_get_free(&args);

    if (*directory_names == 0)
    {
        show_usage(argv[0]);
        return 1;
    }

    while (*directory_names)
    {
        if (mkdir(*directory_names, 0x1FF) < 0)
        {
            eprintf("Unable to create directory `%s`\n", *directory_names);
            return 1;
        }

        directory_names++;
    }

    return 0;
}

void show_usage(char* prog_name)
{
    printf("Usage: %s DIRECTORY ...\n", prog_name);
    printf(" Create one or more new directories with the given name(s).\n\n");
    printf("       -h --help          Show the usage\n");
}