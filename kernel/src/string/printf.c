#include <stddef.h>
#include <stdint.h>

#include "../devices/tty.h"
#include "../serial.h"
#include "color.h"
#include "utility.h"

int printf(const char *restrict format, ...)
{
	char buffer[512];

	va_list args;
	va_start(args, format);

	size_t written = vsnprintf(buffer, 512, format, args);

	va_end(args);

	for (size_t i = 0; i < written; i++) {
		if (buffer[i] == '\e') {
			if (memcmp(buffer + i, BLK, 7) == 0) {
				i += 6;
			} else if (memcmp(buffer + i, RED, 7) == 0) {
				i += 6;
			} else if (memcmp(buffer + i, GRN, 7) == 0) {
				i += 6;
			} else if (memcmp(buffer + i, YEL, 7) == 0) {
				i += 6;
			} else if (memcmp(buffer + i, BLU, 7) == 0) {
				i += 6;
			} else if (memcmp(buffer + i, MAG, 7) == 0) {
				i += 6;
			} else if (memcmp(buffer + i, CYN, 7) == 0) {
				i += 6;
			} else if (memcmp(buffer + i, WHT, 7) == 0) {
				i += 6;
			}

			continue;
		}

		serial_write(buffer[i]);
	}

	if (TTY_ready()) {
		TTY_puts(buffer);
	}

	return written;
}
