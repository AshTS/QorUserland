#ifndef _PARSER_H
#define _PARSER_H

#include <libc/stddef.h>

typedef struct location
{
    char* file;
    size_t line;
    size_t column;
} Location;

char* render_location(Location*);

struct Line
{
    char* data;
    Location loc;
};

struct ParsingError
{
    Location loc;
    char* error_text;
};

enum TokenType
{
    DIRECTIVE,
    REGISTER,
    IDENTIFIER,
    SYMBOL,
    INSTRUCTION,
    NUMBER,
    EOF
};

typedef struct Token
{
    enum TokenType type;
    union 
    {
        char* str;
        char c;
        long int num;
    } data;
    Location location;
} Token;

char* render_token(Token*);

char* get_next_line(char* buffer, size_t* line_number);

Token* tokenize_buffer(char* buffer, char*, size_t*);


#endif // _PARSER_H