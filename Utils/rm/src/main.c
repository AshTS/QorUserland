#include <libc/assert.h>
#include <libc/stdio.h>
#include <libc/dirent.h>
#include <libc/stdlib.h>
#include <libc/string.h>
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

    // Get the list of FILEs
    char** files = arg_get_free(&args);

    // Make sure there is atleast one argument
    if (*files == 0)
    {
        printf("Missing operand\n");
        exit(1);
    }

    // Call the rm syscall on all of the files passed
    for (int i = 0; files[i]; i++)
    {
        int result = sys_unlink(files[i]);

        if (result < 0)
        {
            printf("Cannot remove %s: %s\n", files[i], strerror(-result));
            exit(1);
        }
    }

    return 0;
}

void show_usage(char* prog_name)
{
    printf("Usage: %s [OPTIONS] ... [FILES] ...\n", prog_name);
    printf(" Remove or unlink FILES \n\n");
    printf("       -h --help          Show the usage\n");
}