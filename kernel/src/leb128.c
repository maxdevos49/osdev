#include "debug.h"
#include "type.h"
#include <stddef.h>
#include <stdint.h>

static inline err_code decode_uleb128(uint8_t **stream, uintptr_t stream_end,
									  uint64_t *output)
{
	if (*(uintptr_t *)stream >= stream_end) {
		debug_code(ERROR_OUT_OF_BOUNDS);
		return ERROR_OUT_OF_BOUNDS;
	}

	*output = 0;
	size_t shift = 0;
	uint8_t byte = 0;

	while (1) {
		byte = **stream;

		*output |= ((byte & 0x7f) << shift);

		if (*(uintptr_t *)stream + sizeof(uint8_t) > stream_end) {
			debug_code(ERROR_OUT_OF_BOUNDS);
			return ERROR_OUT_OF_BOUNDS;
		}

		*stream += sizeof(uint8_t);

		if ((byte & 0x80) == 0)
			break;

		shift += 7;
	}

	return 0;
}

static inline err_code decode_sleb128(uint8_t **stream, uintptr_t stream_end,
									  int64_t *output, size_t output_size)
{
	if (*(uintptr_t *)stream >= stream_end) {
		debug_code(ERROR_OUT_OF_BOUNDS);
		return ERROR_OUT_OF_BOUNDS;
	}

	*output = 0;
	size_t shift = 0;
	uint8_t byte = 0;

	while (1) {
		byte = **stream;

		*output |= ((byte & 0x7f) << shift);
		shift += 7;

		if (*(uintptr_t *)stream + sizeof(uint8_t) > stream_end) {
			debug_code(ERROR_OUT_OF_BOUNDS);
			return ERROR_OUT_OF_BOUNDS;
		}

		*stream += sizeof(uint8_t);

		if ((byte & 0x80) == 0)
			break;
	}

	/* sign bit of byte is second high order bit (0x40) */
	if ((shift < output_size) && (byte & 0x40))
		/* sign extend. */
		*output |= (~0UL << shift);

	return 0;
}

// Decode unsigned LEB128 to uint8_t
err_code leb128_to_u8(void **stream, uintptr_t stream_end, uint8_t *output)
{
	uint64_t temp = 0;

	err_code err = 0;
	if ((err = decode_uleb128((uint8_t **)stream, stream_end, &temp))) {
		debug_code(err);
		return err;
	}

	*output = (uint8_t)temp;

	return 0;
}

// Decode unsigned LEB128 to uint16_t
err_code leb128_to_u16(void **stream, uintptr_t stream_end, uint16_t *output)
{
	uint64_t temp = 0;

	err_code err = 0;
	if ((err = decode_uleb128((uint8_t **)stream, stream_end, &temp))) {
		debug_code(err);
		return err;
	}

	*output = (uint16_t)temp;

	return 0;
}

// Decode unsigned LEB128 to uint32_t
err_code leb128_to_u32(void **stream, uintptr_t stream_end, uint32_t *output)
{
	uint64_t temp = 0;

	err_code err = 0;
	if ((err = decode_uleb128((uint8_t **)stream, stream_end, &temp))) {
		debug_code(err);
		return err;
	}

	*output = (uint32_t)temp;

	return 0;
}

// Decode unsigned LEB128 to uint64_t
err_code leb128_to_u64(void **stream, uintptr_t stream_end, uint64_t *output)
{
	err_code err = 0;
	if ((err = decode_uleb128((uint8_t **)stream, stream_end, output))) {
		debug_code(err);
		return err;
	}

	return 0;
}

// Decode signed LEB128 to int8_t
err_code leb128_to_s8(void **stream, uintptr_t stream_end, int8_t *output)
{
	int64_t temp = 0;

	err_code err = 0;
	if ((err = decode_sleb128((uint8_t **)stream, stream_end, &temp,
							  sizeof(int8_t) * 8))) {
		debug_code(err);
		return err;
	}

	*output = (int8_t)temp;

	return 0;
}

// Decode signed LEB128 to int16_t
err_code leb128_to_s16(void **stream, uintptr_t stream_end, int16_t *output)
{
	int64_t temp = 0;

	err_code err = 0;
	if ((err = decode_sleb128((uint8_t **)stream, stream_end, &temp,
							  sizeof(int16_t) * 8))) {
		debug_code(err);
		return err;
	}

	*output = (int16_t)temp;

	return 0;
}

// Decode signed LEB128 to int32_t
err_code leb128_to_s32(void **stream, uintptr_t stream_end, int32_t *output)
{
	int64_t temp = 0;

	err_code err = 0;
	if ((err = decode_sleb128((uint8_t **)stream, stream_end, &temp,
							  sizeof(int32_t) * 8))) {
		debug_code(err);
		return err;
	}

	*output = (int32_t)temp;

	return 0;
}

// Decode signed LEB128 to int64_t
err_code leb128_to_s64(void **stream, uintptr_t stream_end, int64_t *output)
{
	err_code err = 0;
	if ((err = decode_sleb128((uint8_t **)stream, stream_end, output,
							  sizeof(int64_t) * 8))) {
		debug_code(err);
		return err;
	}

	return 0;
}
