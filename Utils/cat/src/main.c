#include <libc/assert.h>
#include <libc/errno.h>
#include <libc/stdbool.h>
#include <libc/stdio.h>
#include <libc/string.h>

#include "argparse.h"

void show_usage(char*);

void print_file(char* name);

bool show_lines = false;

int main(int argc, char** argv)
{
    // Parse command line arguments
    struct Arguments args;
    assert(!arg_parse(&args, argc, argv));

    show_lines = arg_check_short(&args, 'n') || arg_check_long(&args, "number");

    if (arg_check_short(&args, 'h') || arg_check_long(&args, "help"))
    {
        show_usage(argv[0]);
        return 0;
    }

    char** to_print = arg_get_free(&args);

    if (*to_print == 0)
    {
        print_file("-");
    }
    else
    {
        while (to_print && *to_print)
        {
            print_file(*to_print++);
        }
    }
}

void print_line_number(size_t line)
{
    printf("%4ld ", line);
}

void print_file(char* name)
{
    if (strcmp(name, "-") == 0)
    {
        assert(0 && "Not yet Implemented: cat does not yet implement the ability to print from stdin");
    }

    errno = 0;

    FILE* file = fopen(name, "r");

    if (file == 0 || errno != 0)
    {
        fprintf(stderr, "Unable to open file `%s`: %s\n", name, strerror(errno));
        exit(1);
    }

    char buffer[1024];
    size_t count;

    size_t line = 1;

    if (show_lines) { print_line_number(line); }

    while ((count = fread(buffer, 1, 1023, file)))
    {
        if (errno != 0)
        {
            fprintf(stderr, "Unable to read file `%s`: %s\n", name, strerror(errno));
            exit(1);
        }

        buffer[count] = 0;

        if (!show_lines)
        {
            printf("%s", buffer);
        }
        else
        {
            char* walk = buffer;
            char* ptr;

            while ((ptr = strchr(walk, '\n')))
            {
                *ptr = '\0';

                printf("%s\n", walk);
                walk = ptr + 1;
                print_line_number(++line);
            }

            printf("%s", walk);
        }
    }

    printf("\n");
}

void show_usage(char* prog_name)
{
    printf("Usage: %s [OPTIONS] ... [FILE] ...\n", prog_name);
    printf(" Output the contents of FILE to stdout\n\n");
    printf("       -h --help          Show the usage\n");
    printf("       -n --number        Give line numbers\n");
}