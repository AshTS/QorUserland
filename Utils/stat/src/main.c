#include <libc/assert.h>
#include <libc/errno.h>
#include <libc/stdio.h>
#include <libc/string.h>
#include <libc/time.h>
#include <libc/sys/stat.h>

#include "argparse.h"

void show_usage(char*);
void stat_file(const char* s);

void print_file(char* name);

bool show_lines = false;

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

    char** files = arg_get_free(&args);

    for (int i = 0; files[i]; i++)
    {
        stat_file(files[i]);
    }
}

char* perm_str(uint32_t mode, char* str)
{
    if (mode & 0x4000)
    {
        str[0] = 'd';
    }

    if (mode & 0x001)
    {
        str[9] = 'x';
    }
    if (mode & 0x002)
    {
        str[8] = 'w';
    }
    if (mode & 0x004)
    {
        str[7] = 'r';
    }

    if (mode & 0x008)
    {
        str[6] = 'x';
    }
    if (mode & 0x010)
    {
        str[5] = 'w';
    }
    if (mode & 0x020)
    {
        str[4] = 'r';
    }

    if (mode & 0x040)
    {
        str[3] = 'x';
    }
    if (mode & 0x080)
    {
        str[2] = 'w';
    }
    if (mode & 0x100)
    {
        str[1] = 'r';
    }

    return str;
}

void stat_file(const char* s)
{
    char* ptr = strchr(s, '/');

    if (ptr == NULL || *ptr != '/')
    {
        ptr = (char*)s;
    }
    else
    {
        ptr += 1;
    }

    struct stat st;

    if (stat(s, &st) < 0)
    {
        printf("stat: unable to stat file %s: %s\n", s, strerror(errno));
    }

    char perms[11] = "----------";
    
    perm_str(st.st_mode, perms);

    printf("File: %s\n", ptr);
    printf("Size: %-12lu  Blocks: %-9lu  IO Block: %-6lu %s\n", st.st_size, st.st_blocks, st.st_blksize, "regular file");
    printf("Device: %-10lu  Inode: %-10lu  Links: %i\n", st.st_dev, st.st_ino, st.st_nlink);
    printf("Access: (%04o/%s)  Uid: %-17hu  Gid: %-17hu\n", st.st_mode & 0x3ff, perms, st.st_uid, st.st_gid);
}

void show_usage(char* prog_name)
{
    printf("Usage: %s [OPTIONS] ... [FILE] ...\n", prog_name);
    printf(" Output the contents of FILE to stdout\n\n");
    printf("       -h --help          Show the usage\n");
}