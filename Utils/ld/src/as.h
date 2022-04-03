#ifndef LD_H
#define LD_H

#define DEBUG

#include <libc/stdbool.h>
#include <libc/stdio.h>

extern uint8_t verbose_flag;

#ifdef DEBUG
    #define LOG(...) printf(__VA_ARGS__)
#else
    #define LOG(...) if (verbose_flag) { printf(__VA_ARGS__); }
#endif

#endif // LD_H