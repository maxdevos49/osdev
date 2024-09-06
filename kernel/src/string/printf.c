#include <stdint.h>
#include <stddef.h>

#include "utility.h"
#include "../graphics/graphics.h"
#include "../devices/tty.h"

int printf(const char *restrict format, ...)
{
	char buffer[512];

	va_list args;
	va_start(args, format);

	size_t written = vsnprintf(buffer, 512, format, args);

	va_end(args);

	TTY_puts(buffer);

	return written;
}