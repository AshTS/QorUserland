#include <libc/sys/syscalls.h>
#include <libc/dirent.h>
#include <libc/stdio.h>
#include <libc/string.h>
#include <libc/stdlib.h>

#define INIT_DIRECTORY "/etc/startup.d"
#define KERNEL_LOG(...) { int lfd = sys_open("/var/syslog", O_APPEND); raw_fprintf(lfd, __VA_ARGS__); sys_close(lfd); }

void setup_stdio()
{
    int a = sys_open("/dev/null", O_RDWR);
    int b = sys_open("/dev/fb0", O_RDWR);
    int c = sys_open("/dev/null", O_RDWR);

    sys_dup2(a, 0);
    sys_dup2(b, 1);
    sys_dup2(c, 2);
}

int execute_process(char** argc, char** envp);

int compare_strings(const void* a, const void* b)
{
    return strcmp(*(const char**)a, *(const char**)b);
}

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
        size_t count = 0;
        size_t length = 4;
        char** buffer = malloc(length * sizeof(char*));
        struct dirent* entry;
        while ((entry = readdir(directory)))
        {
            if (entry->d_name[0] == '.')
            {
                continue;
            }

            count++;

            if (count >= length)
            {
                char** new_buf = malloc(2 * length * sizeof(char*));

                memcpy(new_buf, buffer, length * sizeof(char*));

                free(buffer);
                buffer = new_buf;

                length *= 2;
            }

            buffer[count - 1] = entry->d_name;
        }

        KERNEL_LOG("Found %i Files\n", count);

        qsort(buffer, count, sizeof(char*), compare_strings);

        for (int i = 0; i < count; i++)
        {
            KERNEL_LOG("Executing %s\n", buffer[i]);
            char path[256];
            sprintf(path, INIT_DIRECTORY "/%s", buffer[i]);
            char* argv[2];
            argv[0] = path;
            argv[1] = 0;
            execute_process(argv, envp);
        }

        free(buffer);
    }

    KERNEL_LOG("Shutting Down\n");

    closedir(directory);

    sys_sync();
}

int execute_process(char** argv, char** envp)
{
    int child_pid = sys_fork();

    // As the child
    if (child_pid == 0)
    {
        sys_execve(argv[0], argv, envp);
        sys_exit(0);
    }

    // As the parent
    sys_wait(0);

    return child_pid;
}