#include <libc/assert.h>
#include <libc/stdbool.h>
#include <libc/stdio.h>
#include <libc/string.h>

#include "argparse.h"

#include "editor.h"

enum state
{
    COMMAND,
    APPENDING
};

bool show_prompt = false;

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

    // Create the editor object
    struct editor* edit = editor_alloc_new();

    // Allocate a buffer for stdin
    char buffer[512];

    // Default file to write to
    char default_file[512];
    default_file[0] = 0;

    enum state state = COMMAND;

    show_prompt = true;

    while (1)
    {
        if (show_prompt && state == COMMAND)
        {
            printf("* ", state);
        }

        // Read from stdin
        size_t bytes_read;
        while (!(bytes_read = fread(buffer, 1, 511, stdin)));
        buffer[bytes_read - 1] = 0;

        // Split the command on a space
        char cmd_buf[512];
        char arg_buf[512];

        cmd_buf[0] = 0;
        arg_buf[0] = 0;

        char* ptr = strchr(buffer, ' ');

        if (ptr == 0)
        {
            strcpy(cmd_buf, buffer);
        }
        else
        {
            *ptr = 0;

            strcpy(cmd_buf, buffer);
            strcpy(arg_buf, ptr + 1);
        }

        if (state == COMMAND)
        {
            if (strcmp(buffer, "q") == 0)
            {
                break;
            }
            else if (strcmp(buffer, "P") == 0)
            {
                show_prompt = true;
            } 
            else if (strcmp(buffer, ",p") == 0)
            {
                editor_print_lines(edit, 0, 0, false);
            }
            else if (strcmp(buffer, "a") == 0)
            {
                state = APPENDING;
            }
            else if (strcmp(cmd_buf, "w") == 0)
            {
                if (strlen(arg_buf) == 0 && default_file[0] == 0)
                {
                    printf("Must give a file before default write\n");
                }
                else if (strlen(arg_buf) == 0)
                {
                    editor_write(edit, default_file);
                }
                else
                {
                    editor_write(edit, arg_buf);
                    strcpy(default_file, arg_buf);
                }
            }
        }
        else if (state == APPENDING)
        {
            if (strcmp(buffer, ".") == 0)
            {
                state = COMMAND;
                continue;
            }

            editor_write_line(edit, edit->cursor + 1, buffer);

            edit->cursor++;
        }
    }
    
    editor_free(edit);

    arg_free(&args);

    return 0;
}

void show_usage(char* prog_name)
{
    printf("Usage: %s [OPTIONS] ... [FILE] ...\n", prog_name);
    printf(" Edit a text file\n\n");
    printf("       -h --help          Show the usage\n");
}