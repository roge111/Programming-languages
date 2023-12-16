#ifndef _UTIL_H_
#define _UTIL_H_

#include "stdlib.h"
#include <stddef.h>
#include <stdio.h>

inline size_t size_max(size_t x, size_t y) { return (x >= y) ? x : y; }

//_Noreturn void err( const char* msg, ... );

inline void log_message(const char *msg) { fprintf(stdout, "Note: %s\n", msg); }

inline _Noreturn void error(const char *msg) {
  fprintf(stderr, "Error: %s\n", msg);
  exit(1);
}

#endif
