#ifndef EXEC_H
#define EXEC_H

int string_to_arguments(const char* str, char** arguments, int max);

int execute_from_args(int argc, const char** argv, const char** envp, int* return_value);

int command_cd(int argc, const char** argv, const char** envp);

void save_tty_settings();
void load_tty_settings();

#endif EXEC_H