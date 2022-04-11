#ifndef _RISCV_RELOCATIONS_H
#define _RISCV_RELOCATIONS_H

#include "elf.h"

#define R_RISCV_NONE 0
#define R_RISCV_32 1
#define R_RISCV_64 2
#define R_RISCV_RELATIVE 3
#define R_RISCV_COPY 4
#define R_RISCV_JUMP_SLOT 5
#define R_RISCV_BRANCH 16
#define R_RISCV_JAL 17
#define R_RISCV_CALL 18
#define R_RISCV_CALL_PLT 19
#define R_RISCV_GOT_HI20 20
#define R_RISCV_PCREL_HI20 23
#define R_RISCV_PCREL_LO12_I 24
#define R_RISCV_PCREL_LO12_S 25
#define R_RISCV_HI20 26
#define R_RISCV_LO12_I 27
#define R_RISCV_ADD8 33
#define R_RISCV_ADD16 34
#define R_RISCV_ADD32 35
#define R_RISCV_ADD64 36
#define R_RISCV_SUB8 37
#define R_RISCV_SUB16 38
#define R_RISCV_SUB32 39
#define R_RISCV_SUB64 40
#define R_RISCV_ALIGN 43
#define R_RISCV_RVC_BRANCH 44
#define R_RISCV_RVC_JUMP 45
#define R_RISCV_RVC_LUI 46
#define R_RISCV_RELAX 51
#define R_RISCV_SUB6 52
#define R_RISCV_SET6 53
#define R_RISCV_SET8 54
#define R_RISCV_SET16 55
#define R_RISCV_SET32 56
#define R_RISCV_32_PCREL 57

#define ELF_R_INFO(sym, type) (((sym) << 32) | ((type)))
#define ELF_R_SYM(info) ((uint32_t)((info) >> 32))
#define ELF_R_TYPE(info) ((uint32_t)((info) & 0xFFFFFFFF))

typedef struct 
{
    uint64_t r_offset;
    uint64_t r_info;
    uint64_t r_addend;
} Elf64_Rela;


#endif // _RISCV_RELOCATIONS_H