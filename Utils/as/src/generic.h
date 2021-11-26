#ifndef GENERIC_H
#define GENERIC_H

#include <libc/stdbool.h>

extern bool SHOW_DEBUG;

#define DEBUG_PRINTF(...) ((SHOW_DEBUG) ? printf(__VA_ARGS__) : 0)

#endif // GENERIC_H