#ifndef _ARGPARSE_H
#define _ARGPARSE_H

#include <libc/stdbool.h>

struct Arguments
{
    char** long_args;
    char* short_args;

    int argc;
    char** argv;
};

int arg_parse(struct Arguments*, int, char**);

bool arg_check_short(struct Arguments*, char);
bool arg_check_long(struct Arguments*, char*);

char** arg_get_free(struct Arguments*);

void arg_free(struct Arguments*);

#endif // _ARGPARSE_H