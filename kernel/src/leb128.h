#ifndef __UTILITY_H
#define __UTILITY_H 1

#include <stddef.h>
#include <stdint.h>

#define DECODE_ULEB128(stream, intType)                                        \
	((intType)decode_uleb128((uint8_t **)stream))
#define DECODE_SLEB128(stream, intType)                                        \
	((intType)decode_sleb128((uint8_t **)stream, sizeof(intType) * 8))

uint64_t decode_uleb128(uint8_t **stream);
int64_t decode_sleb128(uint8_t **stream, size_t bit_size);

#endif
