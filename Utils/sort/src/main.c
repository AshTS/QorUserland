#include <libc/assert.h>
#include <libc/errno.h>
#include <libc/stdbool.h>
#include <libc/stdio.h>
#include <libc/string.h>
#include <libc/stdlib.h>

#include "argparse.h"


static int max_lines = 0;
static int line_count = 0;
static char** lines_buffer = NULL;

void show_usage(char* prog_name);

void expand_line_buffer();
void add_line_to_buffer(char* line);
void load_lines_from(FILE* stream);

int cmp(const void* a, const void* b);

static bool reverse = false;

int main(int argc, char** argv)
{
    // Parse command line arguments
    struct Arguments args;
    int arg_parse_result = arg_parse(&args, argc, argv);
    assert(!arg_parse_result);

    // Get the flags from the arguments
    reverse = (arg_check_short(&args, 'r') || arg_check_long(&args, "reverse"));

    // Show the help if it has been requested
    if (arg_check_short(&args, 'h') || arg_check_long(&args, "help"))
    {
        show_usage(argv[0]);
        return 0;
    }

    // Load lines from files if any are given
    char** passed_files = arg_get_free(&args);

    for (int i = 0; passed_files[i]; i++)
    {
        // The file "-" is used to denote stdin
        if (strcmp(passed_files[i], "-") == 0)
        {
            load_lines_from(stdin);
        }
        else
        {
            FILE* fp = fopen(passed_files[i], "r");
            if (!fp)
            {
                printf("Unable to open file `%s`: %s\n", passed_files[i], strerror(errno));
                exit(1);
            }

            load_lines_from(fp);

            fclose(fp);
        }
    }

    // If not, load them from stdin
    if (passed_files[0] == NULL)
    {
        load_lines_from(stdin);
    }

    // Sort the lines obtained
    qsort(lines_buffer, line_count, sizeof(char*), cmp);

    // Print out the lines, freeing them as I go
    for (int i = 0; i < line_count; i++)
    {
        printf("%s\n", lines_buffer[i]);

        free(lines_buffer[i]);
    }

    // Free the buffer of pointers
    free(lines_buffer);
}

void show_usage(char* prog_name)
{
    printf("Usage: %s [OPTIONS] ... [FILES] ...\n", prog_name);
    printf(" Sort the lines in FILES to stdout\n\n");
    printf("       -h --help          Show the usage\n");
}

void expand_line_buffer()
{
    // Calculate the size of the new buffer
    int new_size;
    if (max_lines == 0)
    {
        new_size = 1;
    }
    else
    {
        new_size = max_lines * 2;
    }

    // Allocate the buffer
    void* new_buffer = malloc(sizeof(char*) * new_size);

    // If necessary, copy over the old buffer
    if (lines_buffer != NULL)
    {
        memcpy(new_buffer, lines_buffer, sizeof(char*) * max_lines);
        free(lines_buffer);
    }

    // Assign the new buffer to the variable
    lines_buffer = new_buffer;
    max_lines = new_size;
}

void add_line_to_buffer(char* line)
{
    int length = strlen(line);

    if (line[length - 1] == '\n')
    {
        line[length - 1] = 0;
    }

    if (line_count == max_lines)
    {
        expand_line_buffer();
    }

    lines_buffer[line_count++] = strdup(line);
}

void load_lines_from(FILE* stream)
{
    char line_buffer[1024];

    errno = 0;

    while (fgets(line_buffer, 1023, stream))
    {
        add_line_to_buffer((const char*)line_buffer);
    }

    if (errno != 0)
    {
        perror("file read");
        exit(1);
    }
}

int cmp(const void* a, const void* b)
{
    return strcmp(*((char**) a), *((char**) b)) * (reverse ? -1 : 1);
}