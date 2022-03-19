#include <libc/assert.h>
#include <libc/errno.h>
#include <libc/fcntl.h>
#include <libc/stdbool.h>
#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <libc/string.h>
#include <libc/sys/stat.h>
#include <libc/sys/syscalls.h>
#include <libc/unistd.h>

#include "argparse.h"

void show_usage(char*);

void output_file(char* name);

static int line_num_length = 7;
static int sub_grouping = 2;
static int super_grouping = 16;
static bool show_ascii = false;

int main(int argc, char** argv)
{
    // Parse command line arguments
    struct Arguments args;
    int arg_parse_result = arg_parse(&args, argc, argv);
    assert(!arg_parse_result);

    if (arg_check_short(&args, 'h') || arg_check_long(&args, "help"))
    {
        show_usage(argv[0]);
        return 0;
    }

    // Check for canonical mode request
    if (arg_check_short(&args, 'C') || arg_check_long(&args, "canonical"))
    {
        line_num_length = 8;
        sub_grouping = 1;
        super_grouping = 8;
        show_ascii = true;
    }

    char** to_print = arg_get_free(&args);

    if (*to_print == 0)
    {
        printf("input from stdin not yet supported\n");
        return 2;
    }
    else
    {
        while (to_print && *to_print)
        {
            output_file(*to_print++);
        }
    }
}

void hex_dump(void* buf, size_t length)
{
    char* buffer = (char*)buf;

    size_t line = 0;

    while (line < length)
    {
        printf("%0*lx ", line_num_length, line);

        for (size_t i = 0; i < 16; i++)
        {
            if (line + i < length)
            {
                printf("%02x", buffer[line + i]);
            }
            else
            {
                printf("  ");
            }

            if ((i + 1) % sub_grouping == 0)
            {
                printf(" ");
            }

            if ((i + 1) % super_grouping == 0)
            {
                printf(" ");
            }
        }

        if (show_ascii)
        {
            printf("|");

            for (size_t i = 0; i < 16; i++)
            {
                if (line + i < length)
                {
                    char c = buffer[line + i];

                    if (c >= 0x20 && c < 0x80)
                    {
                        printf("%c", c);
                    }
                    else
                    {
                        printf(".");
                    }
                }
                else
                {
                    printf(" ");
                }
            }

            printf("|");
        }

        printf("\n");

        line += 16;
    }
}

void output_file(char* name)
{
    // Attempt to stat the file to determine its length
    struct stat data;
    int result = stat((const char*)name, &data);

    if (result < 0)
    {
        printf("hexdump: cannot stat file %s: %s\n", name, strerror(-result));
        exit(1);
    }

    size_t length = data.st_size;

    // Attempt to open the file
    int fd = open(name, O_RDONLY);

    if (fd < 0)
    {
        printf("hexdump: cannot open file %s: %s\n", name, strerror(-fd));
        exit(1);
    }

    // Attempt to memory map the file
    size_t ptr = (size_t)sys_mmap(0, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (ptr < 0)
    {
        printf("hexdump: cannot map file %s: %s\n", name, strerror(-ptr));
        exit(1);
    }

    hex_dump((void*)ptr, length);

    // Attempt to unmap the file
    result = sys_munmap((void*)ptr, length);

    if (result < 0)
    {
        printf("hexdump: cannot unmap file %s: %s\n", name, strerror(-result));
        exit(1);
    }

    // Attempt to close the file
    result = close(fd);

    if (result < 0)
    {
        printf("hexdump: cannot unmap file %s: %s\n", name, strerror(-result));
        exit(1);
    }
}

void show_usage(char* prog_name)
{
    printf("Usage: %s [OPTIONS] ... [FILE] ...\n", prog_name);
    printf(" Display the contents of a file in hexadecimal\n\n");
    printf("       -C --canonical     Display the canonical output with ASCII\n");
    printf("       -h --help          Show the usage\n");
}