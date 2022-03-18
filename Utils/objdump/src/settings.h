#ifndef SETTINGS_H
#define SETTINGS_H

#include <libc/stdbool.h>
#include <libc/stdint.h>

struct objdump_settings
{
    bool disassemble;
    bool show_data;
    bool show_symbols;
};

extern struct objdump_settings output_settings;

#endif // SETTINGS_H