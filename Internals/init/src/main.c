#include <libc/sys/syscalls.h>
#include <libc/dirent.h>
#include <libc/stdio.h>

#define INIT_DIRECTORY "/etc/startup.d"
#define KERNEL_LOG(...) { int lfd = open("/var/syslog", O_APPEND); raw_fprintf(lfd, __VA_ARGS__); close(lfd); }

void setup_stdio()
{
    int a = open("/dev/null", O_RDWR);
    int b = open("/dev/fb0", O_RDWR);
    int c = open("/dev/null", O_RDWR);

    dup2(a, 0);
    dup2(b, 1);
    dup2(c, 2);
}

int execute_process(char** argc, char** envp);

int main()
{
    char* envp[1];
    envp[0] = 0;

    setup_stdio();

    DIR* directory = opendir(INIT_DIRECTORY);

    if (directory == 0)
    {
        KERNEL_LOG("Unable to open starup directory `%s`\n", INIT_DIRECTORY);
        return 1;
    }
    else
    {
        struct dirent* entry;
        while ((entry = readdir(directory)))
        {
            if (entry->d_name[0] == '.')
            {
                continue;
            }

            KERNEL_LOG("Executing %s\n", entry->d_name);
            char path[256];
			sprintf(path, INIT_DIRECTORY "/%s", entry->d_name);
            char* argv[2];
            argv[0] = path;
            argv[1] = 0;
            execute_process(argv, envp);
        }
    }

    KERNEL_LOG("Shutting Down\n");

    closedir(directory);

    sync();
}

int execute_process(char** argv, char** envp)
{
    int child_pid = fork();

    // As the child
    if (child_pid == 0)
    {
        execve(argv[0], argv, envp);
        exit(0);
    }

    // As the parent
    wait(0);

    return child_pid;
}