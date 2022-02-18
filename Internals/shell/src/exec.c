#include "exec.h"

#include <libc/errno.h>
#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <libc/string.h>
#include <libc/sys/syscalls.h>
#include <libc/sys/types.h>
#include <libc/termios.h>
#include <libc/unistd.h>

// Update the path stored interally
char* get_path();

// Get a path entry from the internally stored path
char* get_path_entry(int i);

const char* PATH = NULL;
char* MALLOCED_PATH = NULL;

int string_to_arguments(const char* str, char** arguments, int max)
{
    if (max == 0) return 0;

    arguments[0] = strtok(str, " ");

    if (arguments[0] == NULL)
    {
        return 0;
    }

    for (int i = 1; i < max; i++)
    {
        arguments[i] = strtok(NULL, " ");

        if (arguments[i] == NULL)
        {
            return i;
        }
    }

    return max;
}

// Tries to run the program in the context of every element in the path, if this function returns, it was unable to locate the file anywhere.
int try_all_paths(int argc, const char** argv, const char** envp)
{
    // Just try the default (in the current working directory)
    sys_execve(argv[0], argv, envp);

    // Load the path
    get_path();

    // Loop through every path
    int i = 0;
    char* path_prefix;
    while (path_prefix = get_path_entry(i++))
    {
        int total_length = strlen(argv[0]) + strlen(path_prefix) + 2;
        char* buffer = malloc(total_length);
        strcpy(buffer, path_prefix);
        strcat(buffer, "/");
        strcat(buffer, argv[0]);

        char* argv_0_backup = argv[0];
        argv[0] = buffer;

        sys_execve(argv[0], argv, envp);

        argv[0] = argv_0_backup;

        free(buffer);
    }
}


int execute_from_args(int argc, const char** argv, const char** envp, int* return_value)
{
    pid_t pid = sys_fork();
    
    if (pid == 0)
    {
        int walking_index = 0;
        // Repeatedly check for redirections or pipes
        while (walking_index < argc)
        {
            if (strcmp(argv[walking_index], ">") == 0)
            {
                // Make sure that there exists a file
                if (walking_index + 1 >= argc)
                {
                    eprintf("No file given for redirection\n");
                    exit(-1);
                }

                char* filename = argv[walking_index + 1];

                // Try to open the file
                int file_descriptor = open(filename, O_CREAT | O_RDWR);

                if (file_descriptor < 0)
                {
                    eprintf("Unable to open or create file `%s`: %s\n", filename, strerror(-file_descriptor));
                    exit(-1);
                }

                // Apply the redirection
                int result = sys_dup2(file_descriptor, 1);

                if (result < 0)
                {
                    eprintf("Unable to apply redirection: %s\n", filename, strerror(-result));
                    exit(-1);
                }

                // Modify the argv
                for (int i = walking_index + 2; i < argc; i++)
                {
                    argv[i - 2] = argv[i];
                }

                argv[walking_index] = 0;

                argc -= 2;
            }
            else if (strcmp(argv[walking_index], "<") == 0)
            {
                // Make sure that there exists a file
                if (walking_index + 1 >= argc)
                {
                    eprintf("No file given for input redirection\n");
                    exit(-1);
                }

                char* filename = argv[walking_index + 1];

                // Try to open the file
                int file_descriptor = open(filename, O_RDONLY);

                if (file_descriptor < 0)
                {
                    eprintf("Unable to open or file `%s`: %s\n", filename, strerror(-file_descriptor));
                    exit(-1);
                }

                // Apply the redirection
                int result = sys_dup2(file_descriptor, 0);

                if (result < 0)
                {
                    eprintf("Unable to apply input redirection: %s\n", filename, strerror(-result));
                    exit(-1);
                }

                // Modify the argv
                for (int i = walking_index + 2; i < argc; i++)
                {
                    argv[i - 2] = argv[i];
                }

                argv[walking_index] = 0;

                argc -= 2;
            }
            else if (strcmp(argv[walking_index], "|") == 0)
            {
                // First, delete the pipe symbol
                argv[walking_index] = 0;

                // Next, create the pipe
                int fds[2];
                int result = sys_pipe(fds);

                if (result < 0)
                {
                    eprintf("Unable to create pipe: %s\n", strerror(result));
                    exit(-1);
                }

                // Next, backup the previous read descriptor
                int backup_read = sys_dup(0);

                if (backup_read < 0)
                {
                    eprintf("Unable to backup the read descriptor: %s\n", strerror(backup_read));
                    exit(-1);
                }

                // Redirect the read end of the pipe as the input to the second half of the pipe
                result = sys_dup2(fds[0], 0);

                // Run the second half of the pipe
                execute_from_args(argc - walking_index - 1, argv + walking_index + 1, envp, return_value);

                // Replace the read end of the pipe
                result = sys_dup2(backup_read, 0);

                // Replace the write end of the pipe
                result = sys_dup2(fds[1], 1);

                // Step back the size of argv
                argc = walking_index;

                break;
            }
            else
            {
                walking_index++;
            }
        }

        // Set the current process as the terminal's foreground group
        pid_t child_pid = sys_getpid();
        sys_setpgid(child_pid, child_pid);

        tcsetpgrp(STDIN_FILENO, child_pid);

        // Execute the program
        try_all_paths(argc, argv, envp);

        // If we are still in this process at this point, the load failed
        eprintf("Unable to load executable `%s`\n", argv[0]);
        exit(-1);
    }
}

// Update the path stored interally
char* get_path()
{
    PATH = getenv("PATH");

    if (MALLOCED_PATH != NULL)
    {
        free(MALLOCED_PATH);
    }

    MALLOCED_PATH = strdup(PATH);

    if (MALLOCED_PATH == NULL)
    {
        eprintf("Error: Unable to allocate MALLOCED_PATH\n");
        exit(1);
    }

    for (int i = 0; PATH[i]; i++)
    {
        if (PATH[i] == ':')
        {
            MALLOCED_PATH[i] = 0;
        }

        i++;
    }

    return PATH;
}

// Get a path entry from the internally stored path
//
// NOTE: This returns a string on the heap, it needs to have free called.
char* get_path_entry(int index)
{
    if (PATH == NULL)
    {
        return NULL;
    }

    if (MALLOCED_PATH == NULL)
    {
        eprintf("Error: MALLOCED_PATH is not allocated when PATH has been.\n");
        exit(1);
    }

    for (int i = 0; PATH[i]; i++)
    {
        if (index == 0)
        {
            return strdup(&MALLOCED_PATH[i]);
        }

        if (PATH[i] == ':')
        {
            index -= 1;
        }
    }

    return NULL;
}

int command_cd(int argc, const char** argv, const char** envp)
{
    // The cd command only accepts one argument
    if (argc == 1)
    {
        printf("cd requires one argument.\n");
        return 1;
    }
    else if (argc > 2)
    {
        printf("cd requires only one argument.\n");
        return 1;
    }

    // Execute the change directory syscall
    if (sys_chdir(argv[1]) == -1)
    {
        printf("Unable to switch to `%s`\n", argv[1]);
        return 1;
    }

    return 0;
}

static struct termios terminal_settings;

void save_tty_settings()
{
    tcgetattr(STDIN_FILENO, &terminal_settings);
}

void load_tty_settings()
{
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal_settings);
}

bool check_exist(const char* fname)
{
    errno = 0;
    FILE* stream = fopen(fname, "rb");

    if (stream == NULL || errno)
    {
        errno = 0;
        return false;
    }
    
    fclose(stream);

    return true;
}