#ifndef __PANIC_H
#define __PANIC_H 1

#include "macro.h"

NO_RETURN void panic();
NO_RETURN __attribute__((format(printf, 1, 2))) void
panicf(const char *restrict format, ...);
NO_RETURN void halt();

#endif
