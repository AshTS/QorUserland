#ifndef _PARSER_H
#define _PARSER_H

#include <libc/stdint.h>
#include <libc/stddef.h>

#include "editor.h"

struct command
{
    size_t start_line;
    size_t end_line;
    char cmd;
    char* arguments;
};

struct command* command_alloc_new();
void command_alloc_free(struct command*);

struct command* parse_command(char*, struct editor*);

#endif // _PARSER_H