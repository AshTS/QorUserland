#include "argparse.h"

#include <libc/assert.h>
#include <libc/stdlib.h>
#include <libc/string.h>

int arg_parse(struct Arguments* args, int argc, char** argv)
{
    if (argv[argc] != 0)
    {
        assert(0 && "argparse requires argv[argc] = 0");
    }

    args->argc = argc;
    args->argv = argv;
    
    args->long_args = malloc(sizeof(char*) * argc);
    args->short_args = malloc(sizeof(char) * 16);

    int long_arg_i = 0;
    int short_arg_i = 0;
    
    for (int i = 1; i < argc; i++)
    {
        assert(argv[i] != 0);

        if (argv[i][0] == 0)
        {
            assert(0 && "Empty argument");
        }

        if (argv[i][0] == '-')
        {
            if (argv[i][1] == '-')
            {
                args->long_args[long_arg_i++] = &argv[i][2];
            }
            else
            {
                for (int j = 1; argv[i][j] != 0; j++)
                {
                    args->short_args[short_arg_i++] = argv[i][j];
                }
            }
        }
    }

    args->long_args[long_arg_i] = 0;
    args->short_args[short_arg_i] = 0;

    return 0;
}

bool arg_check_short(struct Arguments* args, char c)
{
    for (int i = 0; args->short_args[i]; i++)
    {
        if (args->short_args[i] == c)
        {
            return true;
        }
    }

    return false;
}

bool arg_check_long(struct Arguments* args, char* s)
{
    for (int i = 0; args->long_args[i]; i++)
    {
        if (strcmp(args->long_args[i], s) == 0)
        {
            return true;
        }
    }

    return false;
}

char** arg_get_free(struct Arguments* args)
{
    char** ptr = args->argv + 1;

    int i = 0;
    for(;;)
    {
        i++;

        if (i == args->argc)
        {
            break;
        }
        else if (*(args->argv[i]) == '-')
        {
            ptr = &(args->argv[(i + 1)]);
            if (i + 1 == args->argc)
            {
                break;
            }
        }
    }

    return ptr;
}

void arg_free(struct Arguments* args)
{
    free(args->long_args);
    free(args->short_args);
}