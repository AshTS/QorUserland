#include <libc/assert.h>
#include <libc/fcntl.h>
#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <libc/string.h>
#include <libc/unistd.h>

#include "argparse.h"

#include "settings.h"

void show_usage(char*);

void dump_file(const char*);
extern void dump_ptr(void*, uint64_t);

struct objdump_settings output_settings;

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

    // Interpret the command line flags
    output_settings.disassemble = true;
    output_settings.show_data = true;
    output_settings.show_symbols = true;

    const char** filename = (const char**)arg_get_free(&args);

    if (*filename == NULL)
    {
        printf("objdump: no file given\n");
        return 1;
    }

    dump_file(*filename);

    return 0;
}

void show_usage(char* prog_name)
{
    printf("Usage: %s [OPTIONS] ... [FILE] ...\n", prog_name);
    printf(" Display the contents of an object file\n\n");
    printf("       -h --help          Show the usage\n");
}

void dump_file(const char* name)
{
    // Attempt to stat the file to determine its length
    struct stat data;
    int result = stat((const char*)name, &data);

    if (result < 0)
    {
        printf("objdump: cannot stat file %s: %s\n", name, strerror(-result));
        exit(1);
    }

    size_t length = data.st_size;

    // Attempt to open the file
    int fd = open(name, O_RDONLY);

    if (fd < 0)
    {
        printf("objdump: cannot open file %s: %s\n", name, strerror(-fd));
        exit(1);
    }

    // Attempt to memory map the file
    size_t ptr = (size_t)sys_mmap(0, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (ptr < 0)
    {
        printf("objdump: cannot map file %s: %s\n", name, strerror(-ptr));
        exit(1);
    }

    dump_ptr((void*)ptr, length);

    // Attempt to unmap the file
    result = sys_munmap((void*)ptr, length);

    if (result < 0)
    {
        printf("objdump: cannot unmap file %s: %s\n", name, strerror(-result));
        exit(1);
    }

    // Attempt to close the file
    result = close(fd);

    if (result < 0)
    {
        printf("objdump: cannot unmap file %s: %s\n", name, strerror(-result));
        exit(1);
    }
}