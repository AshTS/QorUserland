#include <libc/assert.h>
#include <libc/errno.h>
#include <libc/stdio.h>
#include <libc/stdlib.h>
#include <libc/string.h>

#include "argparse.h"

#include "generic.h"
#include "parser.h"
#include "codegen.h"
#include "riscv.h"
#include "elf.h"

#define DEBUG

#ifdef DEBUG
bool SHOW_DEBUG = true;
#else
bool SHOW_DEBUG = false;
#endif

void show_usage(char*);
int assemble_file(char*);

struct GenerationSettings* generate(Token* tokens, size_t count, size_t* section_count);

int main(int argc, char** argv)
{
    #ifdef DEBUG
    printf("Debug Build\n");
    #endif

    // Parse command line arguments
    struct Arguments args;
    assert(!arg_parse(&args, argc, argv));

    // If the help has been requested, show the usage
    if (arg_check_long(&args, "help") || arg_check_short(&args, 'h'))
    {
        show_usage(argv[0]);
        return 0;
    }

    SHOW_DEBUG = SHOW_DEBUG || (arg_check_long(&args, "verbose") || arg_check_short(&args, 'V'));

    // Get the files to assemble from the command line
    char** files_to_assemble = arg_get_free(&args);

    if (*files_to_assemble == 0)
    {
        eprintf("No files given to assemble.\n");
        return 1;
    }

    while (*files_to_assemble)
    {
        int result = assemble_file(*files_to_assemble);

        if (result != 0)
        {
            return result;
        }

        files_to_assemble++;
    }
}

void show_usage(char* prog_name)
{
    printf("Usage: %s [OPTIONS] ... [FILE] ...\n", prog_name);
    printf(" Assemble a RISC-V assembly file into an executable\n\n");
    printf("       -h --help          Show the usage\n");
    printf("       -V --verbose       Enable debugging output");
}

int assemble_file(char* filename)
{
    DEBUG_PRINTF("Assemble file `%s`\n", filename);

    errno = 0;
    FILE* file = fopen(filename, "r");

    if (file == 0 || errno > 0)
    {
        eprintf("Error: Unable to open file `%s`: %s\n", filename, strerror(errno));
        return 1;
    }

    char buffer[1024];
    int bytes_read = fread(buffer, 1, 1023, file);
    if (errno > 0)
    {
        eprintf("Error: Unable to read file: %s\n", strerror(errno));
        return 1;
    }

    buffer[bytes_read] = 0;

    fclose(file);
    if (errno > 0)
    {
        eprintf("Error: Unable to close file: %s\n", strerror(errno));
        return 1;
    }

    size_t count;
    Token* tokens = tokenize_buffer(buffer, filename, &count);

    size_t section_count;

    if (tokens == NULL)
    {
        return 1;
    }

    struct GenerationSettings* result = generate(tokens, count, &section_count);

    bool fail = false;

    if (link(result))
    {
        if (!write_to_elf(result, "a.out"))
        {
            fail = true;
        }
    }
    else
    {
        fail = true;
    }

    free(tokens);
    
    settings_alloc_free(result);

    return fail;
}

struct GenerationSettings* generate(Token* tokens, size_t count, size_t* section_count)
{
    for (size_t i = 0; i < count; i++)
    {
        printf("%s\n", render_token(&tokens[i]));
    }

    struct Instruction inst; 
    struct ParsingError error;

    struct GenerationSettings* settings = settings_alloc_new();

    while (tokens->type != EOF)
    {
        if (!parse_code(&tokens, settings, &error))
        {
            printf("Error:   %s at %s\n", error.error_text, render_location(&error.loc));
        }
    }
    return settings;
}