#ifndef __UNALIGNED_H
#define __UNALIGNED_H 1

#include "./string/utility.h"
#include "debug.h"
#include "type.h"
#include <stdint.h>

// Consumes a uint8_t from a stream ptr. Returns an error code if the read will
// go out of bounds.
static inline err_code read_u8(void **ptr, uintptr_t ptr_end, uint8_t *output)
{
	if (*(uintptr_t *)ptr + sizeof(uint8_t) > ptr_end) {
		debug_code(ERROR_OUT_OF_BOUNDS);
		return ERROR_OUT_OF_BOUNDS;
	}

	*output = **(uint8_t **)ptr;

	*(uintptr_t *)ptr += sizeof(uint8_t);

	return 0;
}

// Consumes a potentially unaligned uint16_t from a stream ptr. Returns an error
// code if the read will go out of bounds.
static inline err_code read_u16(void **ptr, uintptr_t ptr_end, uint16_t *output)
{
	if (*(uintptr_t *)ptr + sizeof(uint16_t) > ptr_end) {
		debug_code(ERROR_OUT_OF_BOUNDS);
		return ERROR_OUT_OF_BOUNDS;
	}

	memcpy(output, *ptr, sizeof(uint16_t));

	*(uintptr_t *)ptr += sizeof(uint16_t);

	return 0;
}

// Consumes a potentially unaligned uint32_t from a stream ptr. Returns an error
// code if the read will go out of bounds.
static inline err_code read_u32(void **ptr, uintptr_t ptr_end, uint32_t *output)
{
	if (*(uintptr_t *)ptr + sizeof(uint32_t) > ptr_end) {
		debug_code(ERROR_OUT_OF_BOUNDS);
		return ERROR_OUT_OF_BOUNDS;
	}

	memcpy(output, *ptr, sizeof(uint32_t));

	*(uintptr_t *)ptr += sizeof(uint32_t);

	return 0;
}

// Consumes a potentially unaligned uint64_t from a stream ptr. Returns an error
// code if the read will go out of bounds.
static inline err_code read_u64(void **ptr, uintptr_t ptr_end, uint64_t *output)
{
	if (*(uintptr_t *)ptr + sizeof(uint64_t) > ptr_end) {
		debug_code(ERROR_OUT_OF_BOUNDS);
		return ERROR_OUT_OF_BOUNDS;
	}

	memcpy(output, *ptr, sizeof(uint64_t));

	*(uintptr_t *)ptr += sizeof(uint64_t);

	return 0;
}

// Consumes a int8_t from a stream ptr. Returns an error code if the read will
// go out of bounds.
static inline err_code read_s8(void **ptr, uintptr_t ptr_end, int8_t *output)
{
	if (*(uintptr_t *)ptr + sizeof(int8_t) > ptr_end) {
		debug_code(ERROR_OUT_OF_BOUNDS);
		return ERROR_OUT_OF_BOUNDS;
	}

	*output = **(int8_t **)ptr;

	*(uintptr_t *)ptr += sizeof(int8_t);

	return 0;
}

// Consumes a potentially unaligned int16_t from a stream ptr. Returns an error
// code if the read will go out of bounds.
static inline err_code read_s16(void **ptr, uintptr_t ptr_end, int16_t *output)
{
	if (*(uintptr_t *)ptr + sizeof(int16_t) > ptr_end) {
		debug_code(ERROR_OUT_OF_BOUNDS);
		return ERROR_OUT_OF_BOUNDS;
	}

	memcpy(output, *ptr, sizeof(int16_t));

	*(uintptr_t *)ptr += sizeof(int16_t);

	return 0;
}

// Consumes a potentially unaligned int32_t from a stream ptr. Returns an error
// code if the read will go out of bounds.
static inline err_code read_s32(void **ptr, uintptr_t ptr_end, int32_t *output)
{
	if (*(uintptr_t *)ptr + sizeof(int32_t) > ptr_end) {
		debug_code(ERROR_OUT_OF_BOUNDS);
		return ERROR_OUT_OF_BOUNDS;
	}

	memcpy(output, *ptr, sizeof(int32_t));

	*(uintptr_t *)ptr += sizeof(int32_t);

	return 0;
}

// Consumes a potentially unaligned int64_t from a stream ptr. Returns an error
// code if the read will go out of bounds.
static inline err_code read_s64(void **ptr, uintptr_t ptr_end, int64_t *output)
{
	if (*(uintptr_t *)ptr + sizeof(int64_t) > ptr_end) {
		debug_code(ERROR_OUT_OF_BOUNDS);
		return ERROR_OUT_OF_BOUNDS;
	}

	memcpy(output, *ptr, sizeof(int64_t));

	*(uintptr_t *)ptr += sizeof(int64_t);

	return 0;
}

#endif
