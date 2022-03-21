#include "tokenizer.h"

#include "as.h"

#include <libc/assert.h>
#include <libc/stdlib.h>
#include <libc/string.h>

// Render a location structure to a string, buffer must be atleast 128 characters in size
char* render_location(char* buffer, struct location location)
{
    // line _ col _ in file _
    assert((strlen(location.filename + 19 + 20) < 127) && "Would produce a buffer overrun");

    sprintf(buffer, "line %lu col %lu in file %s", location.line, location.column, location.filename);

    return buffer;
}

// Convert a token type to a string
char* token_type_to_string(enum token_type type)
{
    switch (type)
    {
        case TOK_NULL: return "NULL";
        case TOK_DIRECTIVE: return "DIRECTIVE";
        case TOK_IDENTIFIER: return "IDENTIFIER";
        case TOK_STRING: return "STRING";
        case TOK_NUMBER: return "NUMBER";
        case TOK_SYMBOL: return "SYMBOL";
        case TOK_EOF: return "EOF";
        default: return "UNKNOWN";
    }
}

// Render a token structure to a string, buffer must be atleast 128 characters in size
char* render_token(char* buffer, struct token token)
{
    char* token_type_str = token_type_to_string(token.type);

    switch (token.type)
    {
        case TOK_DIRECTIVE:
        case TOK_IDENTIFIER:
        case TOK_STRING:
            assert((strlen(token_type_str) + strlen(token.data.string) + 2 < 127) && "Would produce a buffer overrun");
            sprintf(buffer, "%s(%s)", token_type_str, token.data.string);
            break;
        case TOK_NUMBER:
            sprintf(buffer, "%s(%li)", token_type_str, token.data.number);
            break;
        case TOK_SYMBOL:
            sprintf(buffer, "%s(%c)", token_type_str, token.data.character);
            break;
        case TOK_EOF:
        case TOK_NULL:
            sprintf(buffer, "%s", token_type_str);
            break; 
        default:
            sprintf(buffer, "UNKNOWN FORMAT %i", token.type);
    }

    return buffer;
}


// Free the array of tokens returned by `tokenize`
void free_tokens(struct token* tokens)
{
    // First, iterate over every token in the array, checking each token to check if it needs to have its string freed
    for (int i = 0; tokens[i].type != TOK_NULL; i++)
    {
        switch (tokens[i].type)
        {
            case TOK_DIRECTIVE:
            case TOK_IDENTIFIER:
            case TOK_STRING:
                free(tokens[i].data.string);
            default:
                break;
        }
    }

    // Second, free the entire array
    free(tokens);
}

// Add a token to the output array of tokens
struct token* add_token(struct token* tokens, size_t* length, size_t* max_length, struct token token)
{
    // If there is enough space, we can just insert the token
    if (*length < *max_length)
    {
        tokens[(*length)++] = token;
        return tokens;
    }

    // But if not, we need to allocate more space
    size_t new_size = *max_length ? *max_length * 2 : 16;

    // Allocate the new buffer
    struct token* new_buffer = malloc(new_size * sizeof(struct token));

    // If there was data in the previous buffer, copy it over
    if (*max_length)
    {
        memcpy(new_buffer, tokens, *max_length * sizeof(struct token));
    }

    // If the previous buffer was not null, free it
    if (tokens)
    {
        free(tokens);
    }

    // Update the new max_length and return the new buffer
    *max_length = new_size;

    // Finally, perform the add with the new buffer
    return add_token(new_buffer, length, max_length, token);
}

// Check if a character should be in a directive or identifier
bool check_ident_char(char c)
{
    if (c >= 'a' && c <= 'z')
    {
        return true;
    }
    if (c >= 'A' && c <= 'Z')
    {
        return true;
    }
    if (c >= '0' && c <= '9')
    {
        return true;
    }
    if (c == '.' || c == '_' || c == '-')
    {
        return true;
    }

    return false;
}

// Macro to create the location
#define LOCATION (struct location){.filename = name, .line = line, .column = column}

// Macros to create tokens of different types
#define IDENTIFIER_TOKEN(L, S) (struct token){.type = TOK_IDENTIFIER, .location = (L), .data = {.string = (S)}}
#define DIRECTIVE_TOKEN(L, S) (struct token){.type = TOK_DIRECTIVE, .location = (L), .data = {.string = (S)}}
#define STRING_TOKEN(L, S) (struct token){.type = TOK_STRING, .location = (L), .data = {.string = (S)}}
#define SYMBOL_TOKEN(L, C) (struct token){.type = TOK_SYMBOL, .location = (L), .data = {.character = (C)}}
#define NUMBER_TOKEN(L, N) (struct token){.type = TOK_NUMBER, .location = (L), .data = {.number = (N)}}
#define EOF_TOKEN(L) (struct token){.type = TOK_EOF, .location = (L)}
#define NULL_TOKEN (struct token){.type = TOK_NULL}

// Tokenize an incoming filestream into an array of tokens which must be freed,
// note that any tokens which contain strings also need to be individually
// freed this array will be terminated by a TOK_NULL token
struct token* tokenize(FILE* stream, const char* name)
{
    LOG("Tokenizing file %s\n", name);

    // Result data
    size_t result_length = 0;
    size_t result_max_length = 0;
    struct token* tokens = NULL;

    // Running location data
    size_t line = 1;
    size_t column = 1;

    // Current line buffer
    char current_line[4096];

    while (fgets(current_line, 4095, stream))
    {
        // Initialize the pointer which will walk along the current line
        char* ptr = current_line;

        // Now loop over the line character by character
        while (*ptr != 0)
        {
            column = ptr - current_line + 1;
            char* walk;
            char backup;
            char* buffer;

            switch (*ptr)
            {
                case '\n':
                    line++;
                    column = 1;
                case ' ':
                case '\t':
                    ptr++;
                    continue;
                // Here, we are handling any symbols
                case ':':
                case ',':
                case '(':
                case ')':
                    tokens = add_token(tokens, &result_length, &result_max_length, SYMBOL_TOKEN(LOCATION, *ptr));
                    ptr++;
                    continue;
                // Here, we are starting a directive
                case '.':
                    walk = ptr;
                    while (check_ident_char(*walk++));
                    buffer = malloc(walk - ptr);
                    memcpy(buffer, ptr, walk - ptr);
                    buffer[walk - ptr - 1] = 0;

                    tokens = add_token(tokens, &result_length, &result_max_length, DIRECTIVE_TOKEN(LOCATION, buffer));

                    ptr = walk - 1;
                    continue;
                case '"':
                    walk = ptr + 1;
                    while (*walk != 0 && *walk != '"') { walk++; };
                    buffer = malloc(walk - ptr);
                    memcpy(buffer, ptr + 1, walk - ptr);
                    buffer[walk - ptr - 1] = 0;

                    tokens = add_token(tokens, &result_length, &result_max_length, STRING_TOKEN(LOCATION, buffer));

                    ptr = walk + 1;
                    continue;
            }

            if (*ptr >= '0' && *ptr <= '9' || *ptr == '-')
            {
                walk = ptr;

                size_t number = 0;
                size_t multiply = 1;

                if (*walk == '-')
                {
                    multiply = -1;
                    *walk++;
                }

                while (*walk >= '0' && *walk <= '9')
                {
                    number *= 10;
                    number += *walk - '0';
                    walk++;
                }

                tokens = add_token(tokens, &result_length, &result_max_length, NUMBER_TOKEN(LOCATION, multiply * number));

                ptr = walk;
                continue;
            }

            if (check_ident_char(*ptr))
            {
                walk = ptr;
                while (check_ident_char(*walk++));
                buffer = malloc(walk - ptr);
                memcpy(buffer, ptr, walk - ptr);
                buffer[walk - ptr - 1] = 0;

                tokens = add_token(tokens, &result_length, &result_max_length, IDENTIFIER_TOKEN(LOCATION, buffer));

                ptr = walk - 1;
                continue;
            }

            printf("Unhandled character %c\n", *ptr);
            goto END;
            ptr++;
        }
    }

    // Add the EOF and NULL tokens
    tokens = add_token(tokens, &result_length, &result_max_length, EOF_TOKEN(LOCATION));

    END:
    tokens = add_token(tokens, &result_length, &result_max_length, NULL_TOKEN);

    return tokens;
}