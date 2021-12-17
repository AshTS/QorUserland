#include <libc/assert.h>
#include <libc/stdio.h>
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

    char buffer[512];

    int ret = getcwd(buffer, 511);

    if (ret < 0)
    {
        printf("Error: %s\n", strerror(-ret));
        return 1;
    }

    printf("%s\n", buffer);

    return 0;
}

void show_usage(char* prog_name)
{
    printf("Usage: %s\n", prog_name);
    printf(" Print the current working directory.\n\n");
    printf("       -h --help          Show the usage\n");
}