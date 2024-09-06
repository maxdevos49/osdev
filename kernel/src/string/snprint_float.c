// #include <stdint.h>
// #include <stddef.h>

// #include "utility.h"

// /**
//  * Prints a float string to a buffer.
//  *
//  * @param buffer The output buffer for writing.
//  * @param buffer_size The size of the output buffer needed to avoid overflowing
//  * the buffer. If the buffer size is too small to contain the stringified value
//  * then the output may be truncated. A truncated output buffer result will still
//  * contain a null byte.
//  * @param value The float to print to the buffer.
//  * @param precision How many decimal places to include in the result.
//  *
//  * @returns The number of bytes written to the buffer not including the null byte.
//  */
// int snprint_float(char *buffer, size_t buffer_size, double value, uint8_t precision)
// {
// 	size_t output_size = sprint_int(buffer, buffer_size, (int)value);

// 	if (value < 0)
// 	{
// 		value *= -1;
// 	}

// 	buffer[output_size] = '.';
// 	output_size++;

// 	double decimal_value = value - (int)value;
// 	for (uint8_t i = 0; i < precision; i++)
// 	{
// 		decimal_value *= 10;
// 		buffer[output_size] = (int)decimal_value + 48;
// 		output_size++;
// 		decimal_value -= (int)decimal_value;
// 	}

// 	buffer[output_size] = '\0';

// 	return output_size;
// }
