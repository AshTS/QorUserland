#include "libc/sys/syscalls.h"
#include "libc/stdio.h"
#include "libc/string.h"
#include "signals.h"

#define ESC 27
#define TERMINATE 3

#define COL0 0
#define COL1 35

#define WIDTH 33
#define HEIGHT 30

void start_shell()
{
    const char* name = "/bin/shell";
    const char* argv[2];

    argv[0] = name;
    argv[1] = 0;

    const char* envp[1];

    envp[0] = 0;

    execve("/bin/shell", argv, envp);
}
int main(int argc, char** argv)
{
    int fd = open("/dev/tty0", O_RDONLY);
    dup2(fd, 0);
    fd = open("/dev/tty0", O_RDONLY);
    dup2(fd, 1);
    fd = open("/dev/tty0", O_RDONLY);
    dup2(fd, 2);

    start_shell();
}
