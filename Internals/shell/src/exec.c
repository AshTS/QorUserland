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

int find_redirection(int argc, const char** argv)
{
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(">", argv[i]) == 0)
        {
            return i;
        }
    }

    return -1;
}

int execute_from_args(int argc, const char** argv, const char** envp, int* return_value)
{
    pid_t pid = sys_fork();
    
    if (pid == 0)
    {
        // First, check if there needs to be a redirection (a chevron, not a pipe)
        int redirection_index;
        if ((redirection_index = find_redirection(argc, argv)) >= 1)
        {
            eprintf("%i\n", redirection_index);
            // If there is a redirection, open the file (if it exists)
            if (redirection_index + 1 < argc)
            {
                char* name = argv[redirection_index + 1];
                int fd = sys_open(name, O_CREAT | O_RDWR);

                if (fd < 0)
                {
                    eprintf("Unable to open or create file `%s`: %s\n", name, strerror(-fd));
                    exit(-1);
                }

                int result = sys_dup2(fd, 1);

                if (result < 0)
                {
                    eprintf("Unable to redirect to file: %s\n", strerror(-result));
                    exit(-1);
                }

                for (int i = redirection_index + 2; i < argc; i++)
                {
                    argv[i - 2] = argv[i];
                }

                argc -= 2;

                argv[argc] = NULL;
            }
            else
            {
                eprintf("No file given to redirect to\n");
                exit(-1);
            }
        }

        // In the child process
        try_all_paths(argc, argv, envp);

        // If we are still in this process at this point, the load failed
        eprintf("Unable to load executable `%s`\n", argv[0]);
        exit(-1);
    }
    else
    {
        // In the parent process

        // Wait for the process to finish before looping back
        sys_wait(return_value);
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