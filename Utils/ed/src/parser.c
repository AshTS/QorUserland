#include "parser.h"

#include <libc/assert.h>
#include <libc/stdlib.h>
#include <libc/stdio.h>

struct command* command_alloc_new()
{
    struct command* cmd = malloc(sizeof(struct command));
    
    cmd->start_line = 0;
    cmd->end_line = 0;
    cmd->cmd = ' ';
    cmd->arguments = 0;
    
    return cmd;
}

void command_alloc_free(struct command* cmd)
{
    free(cmd);
}

// If index is 1, we are operating on the second buffer, otherwise we are
// operating on the first buffer.
// If the function returns 1, we are done parsing the pair, if it returns 0, we
// need to parse the seperator and then the rest of the pair.
// Returns negative one if the first symbol is not a valid symbol for the line
// spec
int parse_single_line_spec(size_t* first, size_t* second, int index, char** buffer, struct editor* editor)
{
    assert(index == 0 || index == 1);

    size_t running = 0;
    bool found = false;

    bool offset = false;
    bool direction_neg = false;

    size_t current = 0;

    while (1)
    {
        switch (**buffer)
        {
            case '.':
                found = true;
                current = editor->cursor;
                break;
            case '$':
                found = true;
                current = editor->line_count;
                break;
            case '+':
                offset = true;
                current = editor->cursor;
                break;
            case '-':
                offset = true;
                direction_neg = true;
                current = editor->cursor;
                break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                found = true;
                running *= 10;
                running += **buffer - '0';
                break;
            default:
                if (found || offset)
                {
                    size_t value;
                    if (offset && !found)
                    {
                        if (direction_neg)
                        {
                            value = editor->cursor - 1;
                        }
                        else
                        {
                            value = editor->cursor + 1;
                        }
                    }
                    else if (offset && direction_neg)
                    {
                        value = current - running;
                    }
                    else
                    {
                        value = current + running;
                    }
                    
                    if (index == 0)
                    {
                        *first = value;
                    }
                    else
                    {
                        *second = value;
                    }
                    return 0;
                }
                return -1;
        }

        (*buffer)++;
    }
}

int parse_line_spec(size_t* first, size_t* second, char** buffer, struct editor* editor)
{
    int result0 = parse_single_line_spec(first, second, 0, buffer, editor);

    // Skip parsing the line spec, the first character isn't valid
    if (result0 == -1)
    {
        *first = editor->cursor;
    }

    // If there is a seperator, 
    if (**buffer == ',')
    {
        if (result0 == -1) *first = 1;
        *second = editor->line_count;
        (*buffer)++;
    }
    else if (**buffer == ';')
    {
        if (result0 == -1) *first = editor->cursor;
        *second = editor->line_count;
        (*buffer)++;
    }
    else
    {
        *second = *first;
        return 0;
    }

    parse_single_line_spec(first, second, 1, buffer, editor);
    return 0;
}

void consume_whitespace(char** buffer)
{
    for(;;)
    {
        switch (**buffer)
        {
            case '\n':
            case '\t':
            case ' ':
                (*buffer)++;
                break;
            default:
                return;
        }
    }
}

struct command* parse_command(char* buffer, struct editor* editor)
{
    struct command* cmd = command_alloc_new();

    // Parse the line spec
    parse_line_spec(&cmd->start_line, &cmd->end_line, &buffer, editor);

    // Parse the command
    cmd->cmd = *buffer;
    buffer++;

    // Assign the argument pointer
    consume_whitespace(&buffer);
    cmd->arguments = buffer;

    return cmd;
}