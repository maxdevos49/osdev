#include <stdint.h>
#include <stddef.h>

#include "utility.h"
#include "../graphics/graphics.h"
#include "../devices/tty.h"
#include "../serial.h"

int printf(const char *restrict format, ...)
{
	char buffer[512];

	va_list args;
	va_start(args, format);

	size_t written = vsnprintf(buffer, 512, format, args);

	va_end(args);

	for (int i = 0; i < written; i++)
	{
		serial_write(buffer[i]);
	}

	if (TTY_ready())
	{
		TTY_puts(buffer);
	}

	return written;
}
