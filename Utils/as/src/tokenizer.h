#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <libc/stdint.h>
#include <libc/stddef.h>
#include <libc/stdio.h>

// Token types
enum token_type
{
    TOK_NULL = 0, // none
    TOK_DIRECTIVE, // string
    TOK_IDENTIFIER, // string
    TOK_STRING, // string
    TOK_NUMBER, // number
    TOK_SYMBOL, // character
    TOK_EOF, // none
};

// Location struture
struct location
{
    const char* filename;
    size_t line;
    size_t column;
};

// Token structure
struct token
{
    enum token_type type;
    union 
    {
        const char* string;
        char character;
        uint64_t number;
    } data;

    struct location location;
};

// Render a location structure to a string, buffer must be atleast 128 characters in size
char* render_location(char* buffer, struct location location);

// Render a token structure to a string, buffer must be atleast 128 characters in size
char* render_token(char* buffer, struct token token);

// Free the array of tokens returned by `tokenize`
void free_tokens(struct token* tokens);

// Tokenize an incoming filestream into an array of tokens which must be freed,
// note that any tokens which contain strings also need to be individually
// freed this array will be terminated by a TOK_NULL token
struct token* tokenize(FILE* stream, const char* name);

#endif // TOKENIZER_H