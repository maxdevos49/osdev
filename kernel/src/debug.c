#include "string/utility.h"
#include <stddef.h>
#include <stdint.h>

void debug_hex_table(void *address, size_t length)
{
	uint8_t *byte = (uint8_t *)address;

	printf("                 |  0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f "
		   "| Decoded text\n");

	for (size_t i = 0; i < length; i++) {
		if (i % 16 == 0) {
			if (i != 0) {
				printf("| ");

				for (size_t ii = (i - 16); ii < i; ii++) {
					char c = *(byte + ii);
					if (c > 31 && c < 127) {
						printf("%c", c);
					} else {
						printf(".");
					}
				}

				printf("\n");
			}

			printf("%016lx | ", (uintptr_t)byte + i);
		}

		printf("%02x ", *(byte + i));
	}

	printf("\n");
}
