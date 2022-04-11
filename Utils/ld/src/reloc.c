#include "elf.h"

#include "database.h"

int apply_relocation(void* data, struct symbol_data* symbol, struct relocation_database_entry relocation, uint64_t current_section_addr)
{
    if (symbol == NULL)
    {
        printf("ld: Symbol %s not defined\n", relocation.symbol_name);
        return 1;
    }

    int reloc_type = ELF_R_TYPE(relocation.relocation.r_info);

    printf("Applying relocation %i with symbol %s\n", reloc_type, relocation.symbol_name);

    if (false)
    {

    }
    else
    {
        printf("ld: Not yet implemented relocation type %i\n", reloc_type);
        return 1;
    }

    return 0;
}