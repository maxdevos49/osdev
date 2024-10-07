#include "panic.h"
#include "devices/tty.h"
#include "macro.h"
#include "memory/stack.h"
#include "serial.h"
#include "string/utility.h"
#include <stddef.h>
#include <stdint.h>

NO_RETURN void halt()
{
	for (;;) {
		asm("hlt");
	}
}

NO_RETURN void panic()
{
	printf(KPANIC "Trace:\n");
	strace(10, NULL, NULL);
	halt();
}

NO_RETURN __attribute__((format(printf, 1, 2))) void
panicf(const char *restrict format, ...)
{
	printf(KPANIC);

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

	strace(10, NULL, NULL);

	halt();
}
