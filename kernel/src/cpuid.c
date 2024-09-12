#include <stddef.h>

#include "cpuid.h"

// AMD64 Programmers Manual Volume 3 P.627
struct LONG_MODE_SIZE_IDENTIFIERS cpuid_long_mode_size_identifiers(void)
{
	uint32_t function = 0x80000008;
	uint32_t eax;
	asm volatile(
		"cpuid\n\t"
		: "=a"(eax)
		: "0"(function));

	struct LONG_MODE_SIZE_IDENTIFIERS result = {
		.physical_address_size = eax & 0xff,
		.virtual_address_size = (eax >> 8) & 0xff,
		.guest_physical_address_size = (eax >> 16) & 0xff};

	return result;
}
