#ifndef _RISCV_SYMBOLS_H
#define _RISCV_SYMBOLS_H

#include "elf.h"

// Elf 64 symbol table bindings
#define STB_LOCAL 0
#define STB_GLOBAL 1
#define STB_WEAK 2

// Elf 64 symbol table entry types
#define STT_NOTYPE 0
#define STT_OBJECT 1
#define STT_FUNC 2
#define STT_SECTION 3
#define STT_FILE 4
#define STT_COMMON 5
#define STT_TLS 6

// Symbol table symbol visibilities
#define STV_DEFAULT 0
#define STV_INTERNAL 1
#define STV_HIDDEN 2
#define STV_PROTECTED 3
#define STV_EXPORTED 4
#define STV_SINGLETON 5
#define STV_ELIMINATE 6

// Structure for a symbol in the symbol table for 64 bit elf files
typedef struct
{
    uint32_t st_name;
    unsigned char st_info;
    unsigned char st_other;
    uint16_t  st_shndx;
    uint16_t st_value;
    uint64_t st_size;
} Elf64_Sym;

#endif // _RISCV_SYMBOLS_H