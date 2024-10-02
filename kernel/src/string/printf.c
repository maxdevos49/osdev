#include <stddef.h>
#include <stdint.h>

#include "../devices/tty.h"
#include "../graphics/graphics.h"
#include "../serial.h"
#include "utility.h"

int printf(const char *restrict format, ...)
{
	char buffer[512];

	va_list args;
	va_start(args, format);

	size_t written = vsnprintf(buffer, 512, format, args);

	va_end(args);

	for (size_t i = 0; i < written; i++) {
		serial_write(buffer[i]);
	}

	if (TTY_ready()) {
		TTY_puts(buffer);
	}

	return written;
}
