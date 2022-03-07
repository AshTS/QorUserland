#ifndef EXEC_H
#define EXEC_H

#include <libc/stdbool.h>
#include <libc/sys/types.h>

struct return_handle
{
    int return_code;
    pid_t pid;
};

int string_to_arguments(char* str, char** arguments, int max);

int execute_from_args(int argc, const char** argv, const char** envp, bool as_daemon, int pipe_in, int to_close, struct return_handle* returns, int return_index);

int command_cd(int argc, const char** argv, const char** envp);

void save_tty_settings();
void load_tty_settings();

bool check_exist(const char* fname);

#endif // EXEC_H