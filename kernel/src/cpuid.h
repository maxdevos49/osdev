#ifndef __CPUID_H
#define __CPUID_H 1

#include <stdint.h>

struct LONG_MODE_SIZE_IDENTIFIERS
{
	uint8_t physical_address_size;
	uint8_t virtual_address_size;
	uint8_t guest_physical_address_size;
};

struct LONG_MODE_SIZE_IDENTIFIERS cpuid_long_mode_size_identifiers(void);

#endif
