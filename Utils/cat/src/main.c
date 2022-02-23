#include <libc/assert.h>
#include <libc/errno.h>
#include <libc/stdbool.h>
#include <libc/stdio.h>
#include <libc/string.h>
#include <libc/stdlib.h>

#include "argparse.h"

void show_usage(char*);

void print_file(char* name);

bool show_lines = false;

int main(int argc, char** argv)
{
    // Parse command line arguments
    struct Arguments args;
    int arg_parse_result = arg_parse(&args, argc, argv);
    assert(!arg_parse_result);

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
    FILE* file;
    if (strcmp(name, "-") == 0)
    {
        file = stdin;
    }
    else
    {
        errno = 0;

        file = fopen(name, "r");

        if (file == 0 || errno != 0)
        {
            fprintf(stderr, "Unable to open file `%s`: %s\n", name, strerror(errno));
            exit(1);
        }
    }

    char buffer[4096];
    size_t count;

    size_t line = 1;

    while (fgets(buffer, 4095, file))
    {
        if (show_lines) { print_line_number(line); }

        int length = strlen(buffer);

        if (buffer[length - 1] == '\n')
        {
            buffer[length - 1] = 0;
        }

        printf("%s\n", buffer);
        line++;
    }

    if (errno)
    {
        perror("file read");
        exit(1);
    }
}

void show_usage(char* prog_name)
{
    printf("Usage: %s [OPTIONS] ... [FILE] ...\n", prog_name);
    printf(" Output the contents of FILE to stdout\n\n");
    printf("       -h --help          Show the usage\n");
    printf("       -n --number        Give line numbers\n");
}