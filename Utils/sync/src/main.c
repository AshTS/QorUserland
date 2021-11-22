#include <libc/stdio.h>
#include <libc/sys/syscalls.h>

int main(int argc, char** argv)
{
    printf("Syncing filesystem...\n");

    if (sync())
    {
        printf("Problem syncing filesystem\n");
        return 1;
    }

    return 0;
}