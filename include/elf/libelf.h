#ifndef _ELF_LIBELF_H
#define _ELF_LIBELF_H

#include "elf.h"
#include "symbols.h"
#include "relocations.h"

// Error codes
#define LIB_ELF_BADMAGIC 1
#define LIB_ELF_BADWIDTH 2
#define LIB_ELF_BADENDIAN 3
#define LIB_ELF_BADVERSION 4

// Verify that the header given is an elf header
int elf_verify(Elf64_Ehdr* header);

// Extract information about the section header with the given index 
Elf64_Shdr* elf_get_sh(Elf64_Ehdr* header, int i);

// Get the name of a section from an offset into the section name string section
char* elf_get_section_name(Elf64_Ehdr* header, int offset);

// Get a string from an offset into the string header
char* elf_get_string(Elf64_Ehdr* header, int offset);

// Convert a lib elf error code into a string
char* elf_strerror(int error);

// Check if there is a symbol for the given address
Elf64_Sym* elf_check_for_symbol(Elf64_Ehdr* header, uint64_t addr, uint16_t section_index);

// Generate a symbol string for an address, give atleast 128 bytes for the buffer
// The section index parameter is a hint to speed up searching for the right symbol
char* elf_get_symbol(Elf64_Ehdr* header, char* buffer, uint64_t addr, uint16_t section_index);

#endif // _ELF_LIBELF_H