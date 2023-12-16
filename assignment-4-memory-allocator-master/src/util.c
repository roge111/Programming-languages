#include "util.h"

//_Noreturn void err( const char* msg, ... ) {
//  va_list args;
//  va_start (args, msg);
//  vfprintf(stderr, msg, args); // NOLINT
//  va_end (args);
//  abort();
//}

extern inline void log_message(const char* msg);

extern inline _Noreturn void error(const char* msg);

extern inline size_t size_max( size_t x, size_t y );
