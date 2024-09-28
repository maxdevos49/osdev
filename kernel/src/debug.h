#ifndef __DEBUG_H
#define __DEBUG_H 1

#define DEBUG 1

#ifndef DEBUG
#define DEBUG 0
#endif

#include "macro.h"
#include "string/utility.h"
#include "type.h"
#include <stddef.h>

void debug_hex_table(void *address, size_t length);
const char *debug_code_str(err_code code);

#define debug_code(error_code)                                                 \
	do {                                                                       \
		if (DEBUG) {                                                           \
			const char *error_code_str = debug_code_str(error_code);           \
			if (error_code_str == NULL) {                                      \
				printf(KDEBUG "ERROR CODE: %#x | %s (%s at %d)\n", error_code, \
					   __func__, __FILE__, __LINE__);                          \
			} else {                                                           \
				printf(KDEBUG "%s (%#x) | %s (%s at %d)\n", error_code_str,    \
					   error_code, __func__, __FILE__, __LINE__);              \
			}                                                                  \
		}                                                                      \
	} while (0)

#endif
