#ifndef _EDITOR_H
#define _EDITOR_H

#include <libc/stdint.h>
#include <libc/stddef.h>
#include <libc/stdbool.h>

struct string
{
    char* buffer;
    size_t length;
};

struct string* string_alloc_new();
void string_free(struct string*);
void string_replace(struct string*, char*);
char* string_extract(struct string*);

struct editor
{
    struct string** line_buffer;
    size_t line_count;
    size_t cursor;

    size_t length;
};

struct editor* editor_alloc_new();
void editor_free(struct editor*);
void editor_expand_lines(struct editor*);
void editor_print_lines(struct editor*, size_t start, size_t end, bool);
void editor_write_line(struct editor*, size_t, char*);
void editor_delete_line(struct editor*, size_t);
void editor_write(struct editor*, char*);

#endif // _EDITOR_H