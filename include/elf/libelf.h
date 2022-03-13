#ifndef _ELF_LIBELF_H
#define _ELF_LIBELF_H

#include "elf.h"

// Error codes
#define LIB_ELF_BADMAGIC 1
#define LIB_ELF_BADWIDTH 2
#define LIB_ELF_BADENDIAN 3
#define LIB_ELF_BADVERSION 4

// Verify that the header given is an elf header
int elf_verify(Elf64_Ehdr* header);

// Extract information about the section header with the given index 
Elf64_Shdr* elf_get_sh(Elf64_Ehdr* header, int i);

// Get the name of a section from an offset into the string section
char* elf_get_section_name(Elf64_Ehdr* header, int offset);

// Convert a lib elf error code into a string
char* elf_strerror(int error);

#endif // _ELF_LIBELF_H