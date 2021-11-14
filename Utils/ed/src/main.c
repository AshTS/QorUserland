#include <libc/assert.h>
#include <libc/stdbool.h>
#include <libc/stdio.h>
#include <libc/string.h>

#include "argparse.h"

#include "editor.h"
#include "parser.h"

extern void dump();

enum state
{
    COMMAND,
    INSERTING,
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

    char** file_argument = arg_get_free(&args);

    if (file_argument != 0)
    {
        editor_open(edit, *file_argument);
    }

    enum state state = COMMAND;

    show_prompt = false;

    while (1)
    {
        if (0) printf("State:\n Cursor: %ld\n  Line Count: %ld\n  Allocated: %ld\n", edit->cursor, edit->line_count, edit->lines_alloc);
        if (show_prompt && state == COMMAND)
        {
            printf("* ");
        }

        // Read from stdin
        size_t bytes_read;
        while (!(bytes_read = fread(buffer, 1, 511, stdin)));
        buffer[bytes_read - 1] = 0;

        if (state == COMMAND)
        {
            // Parse the command
            struct command* cmd = parse_command(buffer, edit);

            if (cmd == 0)
            {
                command_alloc_free(cmd);
                continue;
            }

            if (0) printf("Command:\n  Start: %ld\n  End: %ld\n  cmd:%c\n  Arg:`%s`\n", cmd->start_line, cmd->end_line, cmd->cmd, cmd->arguments);

            if (cmd->cmd == 'q')
            {
                command_alloc_free(cmd);
                break;
            }
            else if (cmd->cmd == 'P')
            {
                show_prompt = true;
            } 
            else if (cmd->cmd == 'p' || cmd->cmd == 'n')
            {
                editor_print_lines(edit, cmd->start_line, cmd->end_line, cmd->cmd == 'n');
            }
            else if (cmd->cmd == 'a')
            {
                edit->cursor = cmd->start_line + 1;
                
                edit->cursor--;
                state = INSERTING;
            }
            else if (cmd->cmd == 'i')
            {
                if (cmd->start_line < 2)
                {
                    edit->cursor = 1;
                }
                else
                {
                    edit->cursor = cmd->start_line;
                }

                edit->cursor--;
                state = INSERTING;
            }
            else if (cmd->cmd == 'd')
            {
                edit->cursor = cmd->start_line - 1;

                editor_delete_lines(edit, cmd->start_line, cmd->end_line - cmd->start_line + 1);
            }
            else if (cmd->cmd == 'w')
            {
                if (cmd->arguments && cmd->arguments[0] != 0)
                {
                    strcpy(default_file, cmd->arguments);
                }
                else if (default_file[0] == 0)
                {
                    printf("? No default file given.\n");
                    command_alloc_free(cmd);
                    continue;
                }

                editor_write(edit, default_file);
            }
            else if (cmd->cmd == 'e')
            {
                if (cmd->arguments && cmd->arguments[0] != 0)
                {
                    strcpy(default_file, cmd->arguments);

                    editor_open(edit, default_file);
                }
                else
                {
                    printf("? No default file given.\n");
                }
            }
            
            command_alloc_free(cmd);
        }
        else if (state == INSERTING)
        {
            if (strcmp(buffer, ".") == 0)
            {
                state = COMMAND;
                continue;
            }
            else
            {
                edit->cursor++;
                editor_insert_line(edit, edit->cursor, buffer);
            }
        }
    }
    
    editor_free(edit);

    arg_free(&args);

    return 0;
}

void show_usage(char* prog_name)
{
    printf("Usage: %s [OPTIONS] ... [FILE]\n", prog_name);
    printf(" Edit a text file\n\n");
    printf("       -h --help          Show the usage\n");
}