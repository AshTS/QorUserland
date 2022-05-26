#include "elf.h"

#include "database.h"

#include <libc/assert.h>
#include <elf/relocations.h>

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

    
    printf("S: %lx\n", S);
    printf("P: %lx\n", P);
    printf("A: %lx\n", A);
    
    if (reloc_type == R_RISCV_CALL)
    {
        int64_t value = S + A - P;

        printf("VALUE: %lx\n", value);

        // Some math to make sure the sign extension plays nice
        int64_t jalr_arg = (value << 20) >> 20;
        int64_t auipc_arg = (value - jalr_arg) >> 12;

        instructions[0] |= (auipc_arg & 0xfffff) << 12;
        instructions[1] |= (jalr_arg & 0xfff) << 20;
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