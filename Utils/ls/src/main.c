#include <libc/assert.h>
#include <libc/stdio.h>
#include <libc/dirent.h>
#include <libc/stdlib.h>

#include "argparse.h"


bool show_all = false;
bool show_lines = false;

void do_list(char* dir_name);
void show_usage(char*);

int main(int argc, char** argv)
{
    // Parse command line arguments
    struct Arguments args;
    int arg_parse_result = arg_parse(&args, argc, argv);
    assert(!arg_parse_result);

    // Get the arguments
    show_all = arg_check_short(&args, 'a') || arg_check_long(&args, "all");
    show_lines = arg_check_short(&args, '1');

    // If the help has been requested, show the usage
    if (arg_check_long(&args, "help") || arg_check_short(&args, 'h'))
    {
        show_usage(argv[0]);
        return 0;
    }

    // Run ls in the listed directories, or just "." if no arguments are passed
    char** raw_arguments = arg_get_free(&args);
    if (*raw_arguments)
    {
        while (*raw_arguments)
        {
            do_list(*raw_arguments);
            raw_arguments++;
        }
    }
    else
    {
        do_list(".");
    }

    return 0;
}

void do_list(char* dir_name)
{
    // First attempt to open the directory
    DIR* directory = opendir(dir_name);

    if (directory == NULL)
    {
        fprintf(stderr, "Unable to open directory `%s`\n", dir_name);
        exit(-1);
    }

    struct dirent* entry;

    int line_total = 0;
    int width = 16;

    const char* format = show_lines ? "%s" : "%16s";

    while ((entry = readdir(directory)))
    {
        if (!show_all && entry->d_name[0] == '.')
        {
            continue;
        }

        line_total += printf(format, entry->d_name);

        if (show_lines || line_total + width >= 80)
        {
            printf("\n");
            line_total = 0;
        }
    }

    if (line_total > 0)
    {
        printf("\n");
    }

    // Close the directory
    closedir(directory);
}

void show_usage(char* prog_name)
{
    printf("Usage: %s [OPTIONS] ... [FILE] ...\n", prog_name);
    printf(" List the files in the given directories\n\n");
    printf("       -a --all           Show all entries (including those starting with '.')\n");
    printf("       -h --help          Show the usage\n");
    printf("       -1                 Show one entry per line\n");
}