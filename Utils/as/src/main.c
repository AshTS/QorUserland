#include <libc/assert.h>
#include <libc/stdio.h>

#include "argparse.h"

void show_usage(char*);

int main(int argc, char** argv)
{
    // Parse command line arguments
    struct Arguments args;
    assert(!arg_parse(&args, argc, argv));

    // If the help has been requested, show the usage
    if (arg_check_long(&args, "help") || arg_check_short(&args, 'h'))
    {
        show_usage(argv[0]);
        return 0;
    }
}

void show_usage(char* prog_name)
{
    printf("Usage: %s [OPTIONS] ... [FILE] ...\n", prog_name);
    printf(" Assemble a RISC-V assembly file into an executable\n\n");
    printf("       -h --help          Show the usage\n");
}