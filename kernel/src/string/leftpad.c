#include <stddef.h>

#include "utility.h"

/**
 * Pads the left side of a string.
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
size_t leftpad(char *str_buffer, size_t buffer_size, size_t desired_length, char character)
{
	size_t current_length = strlen(str_buffer);

	// Bail if already long enough or the buffer is to small to complete the padding.
	if (current_length >= desired_length || desired_length >= buffer_size)
	{
		return 0;
	}

	size_t delta = desired_length - current_length;

	// Shift the existing string including the null byte by the change in length
	// in reverse order. We include the null byte by not subtracting 1 from the
	// desired_length.
	for (int i = current_length; i >= 0; i--)
	{
		str_buffer[i + delta] = str_buffer[i];
	}

	// Apply new padding characters
	for (size_t i = 0; i < delta; i++)
	{
		str_buffer[i] = character;
	}

	return delta;
}