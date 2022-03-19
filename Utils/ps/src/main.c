#include <libc/assert.h>
#include <libc/dirent.h>
#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <libc/string.h>

#include "argparse.h"

void do_list(char*);
void show_usage(char*);

struct process_data get_pid(int pid);

void display_proc(struct process_data);

int parse_integer(char* s, int* value);

struct process_data
{
    pid_t pid;
    char* name;
    size_t resident_mem;
    size_t data_mem;
    size_t text_mem;
};

int main(int argc, char** argv)
{
    // Parse command line arguments
    struct Arguments args;
    int arg_parse_result = arg_parse(&args, argc, argv);
    assert(!arg_parse_result);

    // If the help has been requested, show the usage
    if (arg_check_long(&args, "help") || arg_check_short(&args, 'h'))
    {
        show_usage(argv[0]);
        return 0;
    }

    printf("  PID    Mem  Name\n");
    do_list("/proc");

    return 0;
}

void do_list(char* dir_name)
{
    // First attempt to open the directory
    DIR* directory = opendir(dir_name);

    if (directory == NULL)
    {
        fprintf(stderr, "Unable to open directory `%s`\n", dir_name);
        exit(-1);
    }

    struct dirent* entry;

    while ((entry = readdir(directory)))
    {
        if (entry->d_name[0] == '.')
        {
            continue;
        }

        // Parse the pid from the name
        int pid;
        parse_integer(entry->d_name, &pid);

        display_proc(get_pid(pid));
    }

    // Close the directory
    closedir(directory);
}

void show_usage(char* prog_name)
{
    printf("Usage: %s [OPTIONS] ... \n", prog_name);
    printf(" List the processes on the system\n\n");
    printf("       -h --help          Show the usage\n");
}

struct process_data get_pid(int pid)
{
    char buffer[128];
    struct process_data result;

    result.pid = pid;

    sprintf(buffer, "/proc/%i/statm", pid);

    FILE* f = fopen(buffer, "r");

    if (f < 0)
    {
        perror(buffer);
    }

    fread(buffer, 1, 128, f);

    int i = 0;
    int total, resident, shared, text, lib, data, dirty;

    i += parse_integer(buffer + i, &total);
    while (buffer[i] != 0 && (buffer[i] < '0' || buffer[i] > '9')) {i++;}
    i += parse_integer(buffer + i, &resident);
    while (buffer[i] != 0 && (buffer[i] < '0' || buffer[i] > '9')) {i++;}
    i += parse_integer(buffer + i, &shared);
    while (buffer[i] != 0 && (buffer[i] < '0' || buffer[i] > '9')) {i++;}
    i += parse_integer(buffer + i, &text);
    while (buffer[i] != 0 && (buffer[i] < '0' || buffer[i] > '9')) {i++;}
    i += parse_integer(buffer + i, &lib);
    while (buffer[i] != 0 && (buffer[i] < '0' || buffer[i] > '9')) {i++;}
    i += parse_integer(buffer + i, &data);
    while (buffer[i] != 0 && (buffer[i] < '0' || buffer[i] > '9')) {i++;}
    i += parse_integer(buffer + i, &dirty);

    result.data_mem = data;
    result.resident_mem = resident;
    result.text_mem = text;

    fclose(f);

    sprintf(buffer, "/proc/%i/cmdline", pid);
    f = fopen(buffer, "r");

    if (f < 0)
    {
        perror(buffer);
    }

    size_t bytes_read = fread(buffer, 1, 128, f);

    buffer[bytes_read] = 0;

    result.name = strdup(buffer);

    fclose(f);

    return result;
}

void display_proc(struct process_data proc)
{
    //        PID      Mem  Name
    printf("%5i  %5lu  %s\n", proc.pid, proc.data_mem + proc.resident_mem + proc.text_mem, proc.name);
}

int parse_integer(char* s, int* value)
{
    int v = 0;
    int i;

    for (i = 0; s[i] >='0' && s[i] <= '9'; i++)
    {
        v *= 10;
        v += s[i] - '0';
    }

    *value = v;

    return i;
}