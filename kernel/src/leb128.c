#include <stddef.h>
#include <stdint.h>

// TODO this should take a limit to prevent over read
uint64_t decode_uleb128(uint8_t **stream)
{
	uint64_t output = 0;

	int shift = 0;
	while (1) {
		uint8_t byte = **stream;
		*stream += sizeof(uint8_t);

		output |= ((byte & 0x7f) << shift);

		if ((byte & 0x80) == 0)
			break;

		shift += 7;
	}

	return output;
}

// TODO this should take a limit to prevent over read
int64_t decode_sleb128(uint8_t **stream, size_t bit_size)
{
	int64_t output = 0;

	size_t shift = 0;
	while (1) {
		uint8_t byte = **stream;
		*stream += sizeof(uint8_t);

		output |= ((byte & 0x7f) << shift);

		shift += 7;
		/* sign bit of byte is second high order bit (0x40) */
		if ((byte & 0x80) == 0) {
			if ((shift < bit_size) && (byte & 0x40))
				/* sign extend. */
				output |= (~0UL << shift); // TODO test

			break;
		}
	}

	return output;
}
