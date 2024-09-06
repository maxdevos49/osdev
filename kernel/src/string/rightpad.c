#include <stddef.h>

#include "utility.h"

/**
 * Pads the right side of a string.
 *
 * @param str_buffer A buffer containing a null terminated string.
 * @param buffer_size The length of the buffer containing the string. This is
 * not the length of the string.
 * @param desired_length The new total length of the string not including the
 * null byte.
 * @param character The character to pad the string with.
 *
 * @returns The change in length of the string. Zero if no changes were applied.
 */
size_t rightpad(char *str_buffer, size_t buffer_size, size_t desired_length, char character)
{
	size_t current_length = strlen(str_buffer);

	if (current_length >= desired_length || desired_length >= buffer_size)
	{
		return 0;
	}

	size_t delta = desired_length - current_length;
	for (size_t i = 0; i < delta; i++)
	{
		str_buffer[current_length++] = character;
	}

	str_buffer[current_length] = '\0';

	return delta;
}