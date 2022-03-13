#include <elf/elf.h>
#include <elf/libelf.h>

#define TO_STR_CASE(ident) case ident: return #ident;
char* elf_strerror(int error)
{
    switch (error)
    {
        case 0: return "Success";
        case LIB_ELF_BADMAGIC: return "Bad magic";
        case LIB_ELF_BADWIDTH: return "ELF width 32 not supported";
        case LIB_ELF_BADENDIAN: return "Big endian not supported";
        case LIB_ELF_BADVERSION: return "Bad ELF version";
        default:
            return "Default Taken";
    }
}
#undef TO_STR_CASE