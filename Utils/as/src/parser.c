#include "parser.h"

#include <libc/assert.h>
#include <libc/stdbool.h>
#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <libc/string.h>

size_t count_lines_to(char* start, char* end)
{
    assert(start < end);

    size_t count = 0;

    while (start != end)
    {
        if (*start == 0)
        {
            break;
        }

        count += (*start++ == '\n');
    }

    return count;
}

char* render_location(Location* loc)
{
    static char buffer[64];

    size_t count = sprintf(buffer, "%s Line %ld, Col %ld", loc->file, loc->line, loc->column);
    assert(count < 64);

    return buffer;
}

char* get_next_line(char* buffer, size_t* line_number)
{
    static char* last_buffer;
    static size_t line;

    if (buffer == NULL)
    {
        buffer = last_buffer;
    }
    else
    {
        line = 1;
    }

    if (buffer == NULL)
    {
        return NULL;
    }

    char* walk = buffer;

    while (*walk)
    {
        if (*walk == '\n')
        {
            *walk = 0;
            *line_number = line++;
            last_buffer = walk + 1;

            while (*last_buffer == '\n')
            {
                line++;
                last_buffer++;
            }
            return buffer;
        }

        walk++;
    }


    *line_number = line++;
    last_buffer = NULL;

    return buffer;
}


char* render_token(Token* token)
{
    static char BUFFER[256];
    static size_t buf_len = 256;
    size_t count;

    switch (token->type)
    {
        case DIRECTIVE:
            count = sprintf(BUFFER, "Directive(%s)", token->data.str);
            assert(count < buf_len);
            break;
        case REGISTER:
            count = sprintf(BUFFER, "Register(%ld)", token->data.num);
            assert(count < buf_len);
            break;
        case IDENTIFIER:
            count = sprintf(BUFFER, "Identifier(%s)", token->data.str);
            assert(count < buf_len);
            break;
        case INSTRUCTION:
            count = sprintf(BUFFER, "Instruction(%s)", token->data.str);
            assert(count < buf_len);
            break;
        case SYMBOL:
            count = sprintf(BUFFER, "Symbol(%c)", token->data.c);
            assert(count < buf_len);
            break;
        case NUMBER:
            count = sprintf(BUFFER, "Number(%ld)", token->data.num);
            assert(count < buf_len);
            break;
        case EOF:
            count = sprintf(BUFFER, "EOF");
            assert(count < buf_len);
            break;
        default:
            assert(0);
            break;
    }

    return BUFFER;
}

#define ADD_TOKEN(token) { \
        token_buffer[(*count)++] = (token); \
        if (*count == token_buffer_size) \
        { \
            Token* new_buf = malloc(sizeof(Token) * 2 * token_buffer_size); \
            memcpy(new_buf, token_buffer, sizeof(Token) * token_buffer_size); \
            free(token_buffer); \
            token_buffer = new_buf; \
            token_buffer_size *= 2; \
        } \
    }

#define LOCATION(f, ln, c) (Location) { .file=(f), .line=(ln), .column=(c)}

Token token_str_type(enum TokenType t, char* s, char* filename, size_t line_number, size_t column)
{
    Token token = (Token){ .type = t, .data = 0, .location = LOCATION(filename, line_number, column) };
    token.data.str = s;

    return token;
}

Token token_char_type(enum TokenType t, char c, char* filename, size_t line_number, size_t column)
{
    Token token = (Token){ .type = t, .data = 0, .location = LOCATION(filename, line_number, column) };
    token.data.c = c;

    return token;
}

Token token_num_type(enum TokenType t, size_t num, char* filename, size_t line_number, size_t column)
{
    Token token = (Token){ .type = t, .data = 0, .location = LOCATION(filename, line_number, column) };
    token.data.num = num;

    return token;
}

bool check_is_number(char* s)
{
    while (*s)
    {
        if (!(*s >= '0' && *s <= '9'))
        {
            return false;
        }

        s++;
    }

    return true;
}

long get_number(char* s)
{
    long result = 0;

    while (*s)
    {
        if (!(*s >= '0' && *s <= '9'))
        {
            return -1;
        }
        else
        {
            result *= 10;
            result += *s - '0';
        }

        s++;
    }

    return result;
}

Token interpret_token(char* token, char* filename, size_t line_number, size_t column)
{
    if (*token == '.')
    {
        if (strcmp(token, ".section") == 0)
        {
            return token_str_type(DIRECTIVE, token, filename, line_number, column);
        }
        else
        {
            return token_str_type(IDENTIFIER, token, filename, line_number, column);
        }
    }
    else if (*token == 'x')
    {
        long num = get_number(token + 1);

        if (num >= 0 && num <= 31)
        {
            return token_num_type(REGISTER, num, filename, line_number, column);
        }

        return token_str_type(IDENTIFIER, token, filename, line_number, column);
    }
    else if (*token == 't')
    {
        long num = get_number(token + 1);

        if (num >= 0 && num <= 6)
        {
            if (num < 3)
                return token_num_type(REGISTER, num + 5, filename, line_number, column);
            else if (num < 7)
                return token_num_type(REGISTER, num + 28 - 3, filename, line_number, column);
        }
        else if (strcmp(token, "tp") == 0)
        {
            return token_num_type(REGISTER, 4, filename, line_number, column);
        }

        return token_str_type(IDENTIFIER, token, filename, line_number, column);
    }
    else if (*token == 's')
    {
        long num = get_number(token + 1);

        if (num >= 0 && num <= 11)
        {
            if (num < 2)
                return token_num_type(REGISTER, num + 8, filename, line_number, column);
            else if (num < 12)
                return token_num_type(REGISTER, num + 18 - 2, filename, line_number, column);
        }
        else if (strcmp(token, "sp") == 0)
        {
            return token_num_type(REGISTER, 2, filename, line_number, column);
        }

        return token_str_type(IDENTIFIER, token, filename, line_number, column);
    }
    else if (*token == 'a')
    {
        long num = get_number(token + 1);

        if (num >= 0 && num <= 7)
        {
            return token_num_type(REGISTER, num + 10, filename, line_number, column);
        }
        else if (strcmp(token, "addi") == 0)
        {
            return token_str_type(INSTRUCTION, token, filename, line_number, column);
        }

        return token_str_type(IDENTIFIER, token, filename, line_number, column);
    }
    else if (strcmp(token, "zero") == 0)
    {
        return token_num_type(REGISTER, 0, filename, line_number, column);
    }
    else if (strcmp(token, "ra") == 0)
    {
        return token_num_type(REGISTER, 1, filename, line_number, column);
    }
    else if (strcmp(token, "gp") == 0)
    {
        return token_num_type(REGISTER, 3, filename, line_number, column);
    }
    else if (strcmp(token, "fp") == 0)
    {
        return token_num_type(REGISTER, 8, filename, line_number, column);
    }
    else if (strcmp(token, "ecall") == 0)
    {
        return token_str_type(INSTRUCTION, token, filename, line_number, column);
    }
    else if (strcmp(token, "jal") == 0)
    {
        return token_str_type(INSTRUCTION, token, filename, line_number, column);
    }
    else
    {
        if (check_is_number(token))
        {
            return token_num_type(NUMBER, get_number(token), filename, line_number, column);
        }
        else
        {
            return token_str_type(IDENTIFIER, token, filename, line_number, column);
        }
    }
}

char* min_ptr(char* a, char* b)
{
    return a < b ? a : b;
}

Token* tokenize_buffer(char* buffer, char* filename, size_t* count)
{
    // char* get_next_line(char* buffer, size_t* line_number)

    size_t token_buffer_size = 1;
    *count = 0;
    Token* token_buffer = malloc(sizeof(Token) * token_buffer_size);

    size_t line_number = 0;
    char* current_line = get_next_line(buffer, &line_number);

    while (current_line)
    {
        char* tok = strtok(current_line, " ");

        while (tok)
        {
            char* ptr;
            size_t len;
            while (1)
            {
                len = strcspn(tok, ":,");

                if (!*(tok + len))
                {
                    break;
                }

                ptr = tok + len;

                char symbol = *ptr;
                *ptr = 0;

                ADD_TOKEN(interpret_token(tok, filename, line_number, 1 + tok - current_line));
                ADD_TOKEN(token_char_type(SYMBOL, symbol, filename, line_number, 1 + ptr - current_line));

                tok = ptr + 1;
            }

            if (*tok != 0)
            {
                ADD_TOKEN(interpret_token(tok, filename, line_number, 1 + tok - current_line));
            }

            tok = strtok(NULL, " ");
        }

        current_line = get_next_line(NULL, &line_number);
    }

    struct Token eof;

    eof.location = LOCATION(filename, line_number + 1, 0);
    eof.type = EOF;

    ADD_TOKEN(eof);

    return token_buffer;
}