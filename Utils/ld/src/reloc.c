#include "elf.h"

#include "database.h"

#include <libc/assert.h>

int apply_relocation(void* data, struct symbol_data* symbol, struct relocation_database_entry relocation, uint64_t current_section_addr)
{
    uint32_t* instructions = data + relocation.relocation.r_offset;

    if (symbol == NULL)
    {
        printf("ld: Symbol %s not defined\n", relocation.symbol_name);
        return 1;
    }

    int reloc_type = ELF_R_TYPE(relocation.relocation.r_info);

    printf("Applying relocation %i with symbol %s\n", reloc_type, relocation.symbol_name);

    uint64_t S = symbol->expanded_value;
    uint64_t P = current_section_addr + relocation.relocation.r_offset;
    uint64_t A = relocation.relocation.r_addend;

    /*
    printf("S: %lx\n", S);
    printf("P: %lx\n", P);
    printf("A: %lx\n", A);
    printf("Value: %li\n", value);
    */

    if (reloc_type == 18)
    {
        int64_t value = S + A - P;

        if (value < 0 && (value & ~0x7ff) == 0xfffffffffffff800)
        {
            instructions[1] |= (value & 0xfff) << 20;
        }
        else if (value > 0 && value <= 0x7ff)
        {
            instructions[1] |= (value & 0xfff) << 20;
        }
        else
        {
            printf("Not yet implemented\n");
            assert(0);
            return 1;
        }
    }
    else if (reloc_type == 23)
    {
        int64_t value = (S + A - P) & (((1 << 20) - 1) << 12);

        instructions[0] |= value;
    }
    else if (reloc_type == 24)
    {
        int64_t value = (S - P) & ((1 << 12) - 1);

        instructions[0] |= (value << 20);
    }
    else
    {
        printf("ld: Not yet implemented relocation type %i\n", reloc_type);
        return 1;
    }

    return 0;
}