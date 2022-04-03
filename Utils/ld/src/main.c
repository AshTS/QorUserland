#include <libc/assert.h>
#include <libc/errno.h>
#include <libc/stdbool.h>
#include <libc/stdio.h>
#include <libc/string.h>
#include <libc/stdlib.h>

#include "argparse.h"


bool verbose_flag = false;

void show_usage(char*);

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
            printf("%s\n", *files++);
        }
    }
}


void show_usage(char* prog_name)
{
    printf("Usage: %s [OPTIONS] ... [FILES] ...\n", prog_name);
    printf(" Link the object FILES given into an executable\n\n");
    printf("       -h --help          Show the usage\n");
    printf("       -v --verbose       Show a verbose output\n");
}