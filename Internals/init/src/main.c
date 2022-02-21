#include <libc/sys/syscalls.h>
#include <libc/dirent.h>
#include <libc/stdio.h>
#include <libc/string.h>
#include <libc/stdlib.h>

#define INIT_DIRECTORY "/etc/startup.d"
#define SYSTEM_LOG(...) { int lfd = sys_open("/var/syslog", O_APPEND); raw_fprintf(lfd, __VA_ARGS__); sys_close(lfd); }

void setup_stdio()
{
    int a = sys_open("/dev/null", O_RDWR);
    int b = sys_open("/dev/fb0", O_RDWR);
    int c = sys_open("/dev/null", O_RDWR);

    sys_dup2(a, 0);
    sys_dup2(b, 1);
    sys_dup2(c, 2);
}

int execute_process(char** argc, const char** envp);

int compare_strings(const void* a, const void* b)
{
    return strcmp(*(const char**)a, *(const char**)b);
}

int main(int argc, const char** argv, const char** envp)
{
    setup_stdio();

    DIR* directory = opendir(INIT_DIRECTORY);

    if (directory == 0)
    {
        SYSTEM_LOG("Unable to open startup directory `%s`\n", INIT_DIRECTORY);
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

        SYSTEM_LOG("Found %i Files\n", count);

        qsort(buffer, count, sizeof(char*), compare_strings);

        for (int i = 0; i < count; i++)
        {
            SYSTEM_LOG("Executing %s\n", buffer[i]);
            char path[256];
            sprintf(path, INIT_DIRECTORY "/%s", buffer[i]);
            char* argv[2];
            argv[0] = path;
            argv[1] = 0;
            execute_process((char**)argv, envp);
        }

        free(buffer);
    }

    SYSTEM_LOG("Shutting Down\n");

    closedir(directory);

    sys_reboot(REBOOT_MAGIC1, REBOOT_MAGIC2, REBOOT_CMD_HALT, NULL);
}

int execute_process(char** argv, const char** envp)
{
    int child_pid = sys_fork();

    // As the child
    if (child_pid == 0)
    {
        sys_execve(argv[0], (const char**)argv, envp);
        sys_exit(0);
    }

    // As the parent
    sys_wait(0);

    return child_pid;
}