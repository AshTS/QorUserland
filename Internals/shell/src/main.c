#include <libc/errno.h>
#include <libc/stdbool.h>
#include <libc/stdint.h>
#include <libc/stdlib.h>
#include <libc/string.h>
#include <libc/stdio.h>
#include <libc/sys/syscalls.h>
#include <libc/termios.h>
#include <libc/unistd.h>

#include "exec.h"

int RETURN_CODE = 0;
bool SHOW_TAG = true;

int PIPE_COUNT = 1;
struct return_handle* RETURNS;

void display_tag();
bool read_input(int fd, char* buffer, int length);

int main(int argc, char** argv, const char** envp)
{
    RETURNS = malloc(sizeof(struct return_handle) * 32);
    RETURNS[0].return_code = 0;

    pid_t pgid = sys_getpid();
    
    tcsetpgrp(STDIN_FILENO, pgid);

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
                    PIPE_COUNT = execute_from_args(count, (const char**)arguments, envp, run_as_daemon, -1, 0, RETURNS, 0);

                    // Wait for the process to finish before looping back   
                    if (!run_as_daemon)
                    {
                        errno = 0;

                        int remaining = PIPE_COUNT;
                        int code;

                        while (remaining)
                        {
                            pid_t this_pid = sys_wait(&code);

                            for (int i = 0; i < PIPE_COUNT; i++)
                            {
                                if (RETURNS[i].pid == this_pid)
                                {
                                    RETURNS[i].return_code = code;
                                    remaining--;
                                    break;
                                }
                            }
                        }
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
    char buffer[128];

    int pos = sys_getcwd(buffer, 63);
    buffer[pos] = 0;

    printf("|%s$", buffer);

    bool show_return = false;

    for (int i = 0; i < PIPE_COUNT; i++)
    {
        if (RETURNS[i].return_code)
        {
            show_return = true;
            break;
        }
    }

    if (show_return)
    {
        printf(" [");
        for (int i = 0; i < PIPE_COUNT; i++)
        {
            int return_code = RETURNS[i].return_code;

            if (return_code == 130)
            {
                printf("SIGINT");
            }
            else if (return_code == 137)
            {
                printf("SIGKILL");
            }
            else if (return_code == 143)
            {
                printf("SIGTERM");
            }
            else
            {
                printf("%i", return_code);
            }

            if (i + 1 < PIPE_COUNT)
            {
                printf("|");
            }
        } 
        printf("]");
    }

    printf("> ");
}
