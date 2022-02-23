#include <libc/errno.h>
#include <libc/stdbool.h>
#include <libc/stdint.h>
#include <libc/stdlib.h>
#include <libc/string.h>
#include <libc/stdio.h>
#include <libc/sys/syscalls.h>

#include "exec.h"

int RETURN_CODE = 0;
bool SHOW_TAG = true;

void display_tag();
bool read_input(int fd, char* buffer, int length);

int main(int argc, char** argv, const char** envp)
{
    FILE* input_stream = stdin;

    if (argc > 1)
    {
        // Try to open the file which the shell is loading
        errno = 0;
        FILE* stream = fopen(argv[1], "r");

        if (stream == NULL || errno)
        {
            printf("Unable to open file `%s`: %s\n", argv[1], strerror(errno));
            return 1;
        }

        // Hide the tag
        SHOW_TAG = false;

        input_stream = stream;
    }

    save_tty_settings();

    while (1)
    {
        if (SHOW_TAG)
            display_tag();

        char buffer[1024];
        if (fgets(buffer, 1000, input_stream) != NULL)
        {
            char* arguments[32];

            int len = strlen(buffer);
            if (buffer[len - 1] == '\n')
            {
                buffer[len - 1] = 0;
            }

            int count = string_to_arguments(buffer, arguments, 32);

            if (count != 0)
            {
                bool run_as_daemon = strcmp(arguments[count - 1], "&") == 0;

                // If the command starts with a "#", then it is a comment and can be ignored
                if (arguments[0][0] == '#')
                {
                    continue;
                }

                // If the command is quit, the shell can be terminated
                if (strcmp(arguments[0], "quit") == 0)
                {
                    break;
                }
                // If the command is cd, run the cd operation
                else if (strcmp(arguments[0], "cd") == 0)
                {
                    RETURN_CODE = command_cd(count, (const char**)arguments, envp);
                }
                // If the command is reset, load the teletype settings
                else if (strcmp(arguments[0], "reset") == 0)
                {
                    load_tty_settings();
                }
                // Otherwise, attempt to execute it as an executable program
                else
                {
                    pid_t pid = execute_from_args(count, (const char**)arguments, envp, &RETURN_CODE, run_as_daemon, -1);

                    // Wait for the process to finish before looping back   
                    if (!run_as_daemon)
                    {
                        errno = 0;
                        while (sys_wait(&RETURN_CODE) != pid);
                    }
                }
            }
        }
        else
        {
            break;
        }
    }

    return 0;
}

void display_tag()
{
    char buffer[64];

    int pos = sys_getcwd(buffer, 63);
    buffer[pos] = 0;

    if (RETURN_CODE == 0)
    {
        printf("|%s$> ", buffer);    
    }
    else if (RETURN_CODE == 130)
    {
        printf("|%s$ [SIGINT]> ", buffer);
        RETURN_CODE = 0;
    }
    else if (RETURN_CODE == 137)
    {
        printf("|%s$ [SIGKILL]> ", buffer);
        RETURN_CODE = 0;
    }
    else if (RETURN_CODE == 143)
    {
        printf("|%s$ [SIGTERM]> ", buffer);
        RETURN_CODE = 0;
    }
    else
    {
        printf("|%s$ [%i]> ", buffer, RETURN_CODE);
        RETURN_CODE = 0;
    }
}
