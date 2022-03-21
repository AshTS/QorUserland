#ifndef AS_H
#define AS_H

// #define DEBUG

#include <libc/stdbool.h>

extern bool verbose_flag;

#ifdef DEBUG
    #define LOG(...) printf(__VA_ARGS__)
#else
    #define LOG(...) if (verbose_flag) { printf(__VA_ARGS__); }
#endif

#endif // AS_H