
#include <libc/stdbool.h>
#include <libc/errno.h>
#include "libc/stdio.h"
#include "libc/sys/syscalls.h"
#include <libc/termios.h>
#include "libc/string.h"
#include "signals.h"
#include <libc/unistd.h>


char* PATH = "/bin/";

static int RUNNING_PID = 0;
static bool WAITING = true;
static bool IS_TIMING = false;

static bool SHOW_TAG = true;

static int RETURN_CODE = 0;

static struct termios terminal_settings;

void display_tag();

int handle_redirect(char** argv);

int run_exec(char* exec, char** argv, char** envp);
int run_exec_time(char* exec, char** argv, char** envp);

void handler(int sig, struct siginfo_t *info, void *ucontext)
{
    printf("Got SIGINT\n");

    if (RUNNING_PID > 0)
    {
        // sys_kill(RUNNING_PID, SIGINT);
        printf("\n");
    }
    else
    {
        WAITING = false;
        RETURN_CODE = 127;
    }

    IS_TIMING = false;

    sys_sigreturn();
}

bool read_line_from(int fd, char** line);

int main(int argc, char** argv, const char** envp)
{
    tcgetattr(STDIN_FILENO, &terminal_settings);

    // Setup the handler for SIGINT
    struct sigaction new;
    struct sigaction old;

    int input_fd = 0;

    if (argc > 1)
    {
        // If a file is passed, then the shell is being used to execute a shell script
        input_fd = sys_open(argv[1], O_RDONLY);

        if (input_fd < 0)
        {
            printf("Unable to open file `%s`: %s\n", argv[1], strerror(-input_fd));
            return 1;
        }

        SHOW_TAG = false;
    }

    new.sa_flags = SA_SIGINFO;
    new.sa_sigaction = handler;

    sys_sigaction(SIGINT, &new, &old);

    char* this_argv[64];
    this_argv[0] = 0;

    while (1)
    {
        if (SHOW_TAG)
        {
            display_tag();
        }

        WAITING = true;

        char* buffer;
        if (!read_line_from(input_fd, &buffer))
        {
            goto END;
        }

        /*
        while (WAITING)
        {
            int count = read(input_fd, buffer, 63);

            if (count == 0) goto END;

            buffer[count - 1] = 0;

            break;
        }*/

        if (buffer[0] == 0)
        {
            continue;
        }


        if (!WAITING)
        {
            printf("\n");
            continue;
        }

        while (*buffer == ' ')
        {
            buffer++;
        }

        if (buffer[0] == '#')
        {
            continue;
        }

        if (buffer[0] == 'c' && buffer[1] == 'd' && buffer[2] == ' ')
        {
            char path_buffer[64];

            int i = 2;

            while (buffer[i] == ' ')
            {
                i ++;
            }

            int j = 0;

            while (buffer[i] != '\0')
            {
                path_buffer[j] = buffer[i];
                i++;
                j++;
            }

            path_buffer[j] = '\0';

            if (sys_chdir(path_buffer) == -1)
            {
                eprintf("Unable to switch to `%s`\n", path_buffer);
            }

            continue;
        }
        else if (strcmp("reset", buffer) == 0)
        {
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal_settings);
            continue;
        }

        bool do_time = false;

        if (buffer[0] == 't' && buffer[1] == 'i' && buffer[2] == 'm' && buffer[3] == 'e' && buffer[4] == ' ')
        {
            strcpy(buffer, &buffer[5]);
            do_time = true;
        }

        if (strcmp("quit", buffer) == 0)
        {
            break;
        }

        int buffer_index = 0;
        int argv_index = 0;
        this_argv[0] = buffer;

        while (buffer[buffer_index] != 0)
        {
            if (buffer[buffer_index] == ' ')
            {
                buffer[buffer_index] = 0;

                if (*this_argv[argv_index] != 0)
                {
                    argv_index++;
                }
                this_argv[argv_index] = &buffer[buffer_index + 1];
            }
            buffer_index++;
        }

        this_argv[++argv_index] = 0;

        if (*this_argv[0] == 0)
        {
            continue;
        }

        if (do_time)
        {
            run_exec_time(this_argv[0], this_argv, envp);
        }
        else
        {
            run_exec(this_argv[0], this_argv, envp);
        }
    }

    END:
    return 0;
}

void display_tag()
{
    char buffer[64];

    int pos = sys_getcwd(buffer, 63);
    buffer[pos] = 0;

    if (RETURN_CODE == 0)
    {
        printf("%s$> ", buffer);    
    }
    else if (RETURN_CODE == 130)
    {
        printf("%s$ [SIGINT]> ", buffer);
        RETURN_CODE = 0;
    }
    else if (RETURN_CODE == 137)
    {
        printf("%s$ [SIGKILL]> ", buffer);
        RETURN_CODE = 0;
    }
    else if (RETURN_CODE == 143)
    {
        printf("%s$ [SIGTERM]> ", buffer);
        RETURN_CODE = 0;
    }
    else
    {
        printf("%s$ [%i]> ", buffer, RETURN_CODE);
        RETURN_CODE = 0;
    }
}

int handle_redirect(char** argv)
{
    bool next_pipe_out = false;

    for (int i = 0; argv[i] != 0; i++)
    {
        if (!next_pipe_out)
        {
            if (strcmp(argv[i], ">") == 0)
            {
                // Found a cheveron
                argv[i] = 0;
                next_pipe_out = true;
            }
            else
            {
                next_pipe_out = false;
            }
        }
        else
        {
            int fd = sys_open(argv[i], O_CREAT | O_TRUNC | O_WRONLY);

            if (fd < 0)
            {
                eprintf("Unable to open `%s`\n", argv[i]);

                return -1;
            }

            sys_dup2(fd, 1);

            return fd;
        }
    }

    return 0;
}


int run_exec(char* exec, char** argv, char** envp)
{
    short pid = sys_fork();

    if (pid == 0)
    {
        pid_t pid = sys_getpid();
        sys_setpgid(pid, pid);

        tcsetpgrp(STDIN_FILENO, pid);

        // handle_redirect(argv);
        sys_execve(argv[0], (const char**)argv, (const char**)envp);

        if (argv[0][0] != '/')
        {
            char next_buffer[128];

            int i = 0;

            while (PATH[i] != 0)
            {
                next_buffer[i] = PATH[i];
                i++;
            }

            int j = 0;

            while (argv[0][j] != 0)
            {
                next_buffer[i] = argv[0][j];
                i++;
                j++;
            }

            next_buffer[i] = 0;

            sys_execve(next_buffer, (const char**)argv, (const char**)envp);
        }

        eprintf("Unable to locate executable `%s`\n", argv[0]);

        sys_exit(-1);
    }
    else
    {   
        RUNNING_PID = pid;
        sys_wait(&RETURN_CODE);
        RUNNING_PID = 0;

        return 0;
    }
}


int run_exec_time(char* exec, char** argv, char** envp)
{

    IS_TIMING = true;
    unsigned long start;
    unsigned long end;

    int fd = sys_open("/dev/rtc0", O_RDONLY);

    if (fd < 0)
    {
        eprintf("Unable to open /dev/rtc0\n");
        return -1;
    }

    sys_ioctl(fd, RTC_RD_TIMESTAMP, (unsigned long)&start);

    for (int i = 0; i < 10; IS_TIMING && i++)
    {
        if (run_exec(exec, argv, envp) < 0)
        {
            return -1;
        }
    }
    IS_TIMING = false;

    sys_ioctl(fd, RTC_RD_TIMESTAMP, (unsigned long)&end);
    
    int avg = (end - start) / 10 / 1000000;

    printf("Average Runtime: %i ms\n", avg);

    return 0;
}


bool read_line_from(int fd, char** line)
{
    // TODO: This only accepts files up to one KiB, this needs to be fixed
    static bool READ_NEXT = true;
    static char BUFFER[1024];

    char* ptr = NULL;

    if (READ_NEXT)
    {
        int length = sys_read(fd, BUFFER, 1023);

        if (length == 0)
        {
            return false;
        }

        BUFFER[length] = 0;
        ptr = BUFFER;

        READ_NEXT = false;
    }

    char* token = strtok(ptr, "\n");

    if (token == NULL)
    {
        READ_NEXT = true;
        return read_line_from(fd, line);
    }
    else
    {
        *line = token;
    }

    return true;
}