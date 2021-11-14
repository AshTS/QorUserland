#include "editor.h"

#include <libc/assert.h>
#include <libc/errno.h>
#include <libc/stdlib.h>
#include <libc/stdio.h>
#include <libc/string.h>

#define STRING_INITIAL_SIZE 32
#define INITIAL_LINE_BUFFER_SIZE 32

#ifdef NDEBUG
#define LOG(s, ...) 
#else
#define LOG(s, ...) printf(" DEBUG: " s, __VA_ARGS__)
#endif

char* expand_buffer(char* original, size_t original_length, size_t new_length)
{
    // LOG("Expanding buffer at %p from %ld bytes to %ld bytes\n", original, original_length, new_length);

    assert(new_length >= original_length);

    char* new = malloc(new_length);

    strcpy(new, original);

    free(original);

    return new;
}

struct string* string_alloc_new()
{
    struct string* str = malloc(sizeof(struct string));
    // LOG("Allocatting new string at %p\n", str);

    str->buffer = malloc(STRING_INITIAL_SIZE);
    str->buffer[0] = 0;

    str->length = STRING_INITIAL_SIZE;

    return str;
}

void string_free(struct string* str)
{
    // LOG("Freeing string at %p\n", str);

    free(str->buffer);
    free(str);
}

void string_replace(struct string* str, char* data)
{
    size_t len = strlen(data);
    // LOG("Replacing string at %p with %ld bytes of data\n", str, len);

    while (len >= str->length - 1)
    {
        size_t old = str->length;

        str->buffer = expand_buffer(str->buffer, old, old * 2);
        str->length = old * 2;
    }

    strcpy(str->buffer, data);
}

char* string_extract(struct string* str)
{
    return str->buffer;
}

struct editor* editor_alloc_new()
{
    struct editor* editor = malloc(sizeof(struct editor));

    editor->line_buffer = malloc(sizeof(struct string*) * INITIAL_LINE_BUFFER_SIZE);
    editor->line_count = INITIAL_LINE_BUFFER_SIZE;

    for (int i = 0; i < INITIAL_LINE_BUFFER_SIZE; i++)
    {
        editor->line_buffer[i] = 0;
    }

    editor->cursor = 0;
    editor->length = 0;

    return editor;
}

void editor_expand_lines(struct editor* editor)
{
    size_t old = editor->line_count;
    size_t new = editor->line_count * 2;

    struct string** new_buffer = malloc(sizeof(struct string*) * new);

    memcpy(new_buffer, editor->line_buffer, old * sizeof(struct string*));

    for (int i = old; i< new; i++)
    {
        new_buffer[i] = 0;
    }

    editor->line_count = new;
    editor->line_buffer = new_buffer;
}

void editor_free(struct editor* editor)
{
    for (size_t i = 0; i < editor->line_count; i++)
    {
        if (editor->line_buffer[i])
        {
            string_free(editor->line_buffer[i]);
        }
    }

    free(editor->line_buffer);
    free(editor);
}

void print_line(struct string* line, size_t number, bool line_numbers)
{
    if (line_numbers)
    {
        printf("%4ld %s\n", number, string_extract(line));
    }
    else
    {
        printf("%s\n", string_extract(line));
    }
}

void editor_print_lines(struct editor* editor, size_t start, size_t end, bool line_numbers)
{
    // Start and end both being zero means print the entire buffer
    if (start == end && end == 0)
    {
        struct string** walk = editor->line_buffer;
        size_t line = 1;

        while (*walk != 0)
        {
            print_line(*walk++, line++, line_numbers);
        }
    }
    else
    {
        for (size_t line = start - 1; line < end; line++)
        {
            print_line(editor->line_buffer[line], line + 1, line_numbers);
        }
    }
}

void editor_write_line(struct editor* editor, size_t line, char* data)
{
    while (line >= editor->line_count)
    {
        editor_expand_lines(editor);
    }

    if (!editor->line_buffer[line - 1])
    {
        editor->line_buffer[line - 1] = string_alloc_new();
    }

    string_replace(editor->line_buffer[line - 1], data);
}

void editor_delete_line(struct editor* editor, size_t line)
{
    if (editor->line_buffer[line - 1])
    {
        string_free(editor->line_buffer[line - 1]);

        while (editor->line_buffer[line - 1])
        {
            editor->line_buffer[line - 1] = editor->line_buffer[line];
            line++;
        }
    }
}

void editor_write(struct editor* editor, char* name)
{
    FILE* file = fopen(name, "w");

    if (!file || errno)
    {
        printf("Unable to open file `%s` : %s\nWrite Failed\n", name, strerror(errno));
        return;
    }

    struct string** walk = editor->line_buffer;

    size_t total_bytes_written = 0;

    while (*walk != 0)
    {
        total_bytes_written += fprintf(file, "%s\n", (*walk)->buffer);
        walk++;
    }

    printf("%ld\n", total_bytes_written);

    fclose(file);
}