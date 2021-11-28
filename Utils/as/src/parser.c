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
        case STRING:
            count = sprintf(BUFFER, "String(%s)", token->data.str);
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
    if (*s == '-')
    {
        s++;
    }

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
    long mult = 1;

    if (*s == '-')
    {
        mult = -1;
        s++;
    }

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

    return result * mult;
}

Token interpret_token(char* token, char* filename, size_t line_number, size_t column)
{
    if (*token == '.')
    {
        if (strcmp(token, ".section") == 0)
        {
            return token_str_type(DIRECTIVE, token, filename, line_number, column);
        }
        else if (strcmp(token, ".align") == 0)
        {
            return token_str_type(DIRECTIVE, token, filename, line_number, column);
        }
        else if (strcmp(token, ".bytes") == 0)
        {
            return token_str_type(DIRECTIVE, token, filename, line_number, column);
        }
        else
        {
            return token_str_type(IDENTIFIER, token, filename, line_number, column);
        }
    }
    else if 
        (
            strcmp(token, "lui") == 0 ||
            strcmp(token, "jal") == 0 ||
            strcmp(token, "jalr") == 0 ||
            strcmp(token, "lb") == 0 ||
            strcmp(token, "lh") == 0 ||
            strcmp(token, "lw") == 0 ||
            strcmp(token, "ld") == 0 ||
            strcmp(token, "lbu") == 0 ||
            strcmp(token, "lhu") == 0 ||
            strcmp(token, "lwu") == 0 ||
            strcmp(token, "sb") == 0 ||
            strcmp(token, "sh") == 0 ||
            strcmp(token, "sw") == 0 ||
            strcmp(token, "sd") == 0 ||
            strcmp(token, "beq") == 0 ||
            strcmp(token, "bne") == 0 ||
            strcmp(token, "blt") == 0 ||
            strcmp(token, "bge") == 0 ||
            strcmp(token, "gltu") == 0 ||
            strcmp(token, "bgeu") == 0 ||
            strcmp(token, "addi") == 0 ||
            strcmp(token, "slti") == 0 ||
            strcmp(token, "sltiu") == 0 ||
            strcmp(token, "xori") == 0 ||
            strcmp(token, "ori") == 0 ||
            strcmp(token, "andi") == 0 ||
            strcmp(token, "slli") == 0 ||
            strcmp(token, "srli") == 0 ||
            strcmp(token, "srai") == 0 ||
            strcmp(token, "addiw") == 0 ||
            strcmp(token, "slliw") == 0 ||
            strcmp(token, "srliw") == 0 ||
            strcmp(token, "sraiw") == 0 ||
            strcmp(token, "addw") == 0 ||
            strcmp(token, "subw") == 0 ||
            strcmp(token, "sllw") == 0 ||
            strcmp(token, "srlw") == 0 ||
            strcmp(token, "sraw") == 0 ||
            strcmp(token, "add") == 0 ||
            strcmp(token, "sub") == 0 ||
            strcmp(token, "sll") == 0 ||
            strcmp(token, "slt") == 0 ||
            strcmp(token, "sltu") == 0 ||
            strcmp(token, "xor") == 0 ||
            strcmp(token, "srl") == 0 ||
            strcmp(token, "sra") == 0 ||
            strcmp(token, "or") == 0 ||
            strcmp(token, "and") == 0 ||
            strcmp(token, "ecall") == 0 ||
            strcmp(token, "li") == 0 ||
            strcmp(token, "la") == 0 ||
            strcmp(token, "call") == 0 ||
            strcmp(token, "ret") == 0 ||
            strcmp(token, "push") == 0 ||
            strcmp(token, "pop") == 0 ||
            strcmp(token, "j") == 0
        )
    {
        return token_str_type(INSTRUCTION, token, filename, line_number, column);
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
    size_t token_buffer_size = 1;
    *count = 0;
    Token* token_buffer = malloc(sizeof(Token) * token_buffer_size);

    size_t line_number = 0;
    char* current_line = get_next_line(buffer, &line_number);

    while (current_line)
    {
        char* current_element = current_line;
        char* str_next = NULL;

        size_t len;
        bool repeat = true;

        while (repeat)
        {
            len = strcspn(current_element, "\"");
            if (!*(current_element + len))
            {
                repeat = false;
                str_next = NULL;
            }
            else
            {
                *(current_element + len) = 0;
                str_next = current_element + len + 1;
            }
            
            char* tok = strtok(current_element, " ");
            while (tok)
            {
                char* ptr;
                size_t len;
                while (1)
                {
                    len = strcspn(tok, ":,()");

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

            if (str_next != NULL)
            {
                len = strcspn(str_next, "\"");

                if (!*(str_next + len))
                {
                    printf("ERROR: Unexpected end of line while parsing string at %s\n", render_location(&LOCATION(filename, line_number, 1 + str_next + len - current_line)));

                    free(token_buffer);

                    return NULL;
                }

                *(str_next + len) = 0;
                current_element = str_next + len + 1;

                ADD_TOKEN(token_str_type(STRING, str_next, filename, line_number, 1 + str_next - current_line));
            }
        }

        current_line = get_next_line(NULL, &line_number);
    }

    struct Token eof;

    eof.location = LOCATION(filename, line_number + 1, 0);
    eof.type = EOF;

    ADD_TOKEN(eof);

    return token_buffer;
}