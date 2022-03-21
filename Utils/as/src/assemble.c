#include "assemble.h"

#include "as.h"

#include "tokenizer.h"

// Assemble a file from the file handle
void assemble_file_handle(FILE* file, const char* input_name, const char* output_name)
{
    LOG("Assembling %s to %s\n", input_name, output_name);

    // Tokenize the stream
    struct token* tokens = tokenize(file, input_name);

    char buffer[128];
    char buffer2[128];

    // Iterate over all of the tokens, printing them out
    for (int i = 0; tokens[i].type != TOK_NULL; i++)
    {
        printf(" %s at %s\n", render_token(buffer, tokens[i]), render_location(buffer2, tokens[i].location));
    }

    // Free the tokens, this is very important as the array can grow quite large
    free_tokens(tokens);
}