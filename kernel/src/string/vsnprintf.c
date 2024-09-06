#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

#include "utility.h"
#include "../fonts/font.h"

#define ZERO_FLAG ((uint8_t)1)
#define SPACE_FLAG ((uint8_t)1 << 1)
#define PLUS_FLAG ((uint8_t)1 << 2)
#define MINUS_FLAG ((uint8_t)1 << 3)
#define POUND_FLAG ((uint8_t)1 << 4)
#define APOSTROPHE_FLAG ((uint8_t)1 << 5)

struct FORMAT_SPECIFIER
{
	char specifier;
	uint8_t flags;
	uint16_t width;
	uint16_t precision;
};

static bool is_numeric(char c);
static bool is_flag(char c);
static int parse_byte(char c);
static char get_pad_char(uint8_t flags);

static size_t snprintf_int(char *buffer, size_t buffer_size, int64_t num, struct FORMAT_SPECIFIER *f);
static size_t snprintf_hex(char *buffer, size_t buffer_size, uint64_t num, struct FORMAT_SPECIFIER *f);
// static size_t snprintf_float(char *buffer, size_t buffer_size, double num, struct FORMAT_SPECIFIER *f);

/**
 * Prints a formatted string to a buffer.
 *
 * Formatting Pattern:
 * 		%[flags][width][.precision]specifier
 *
 * Specifiers:
 * 		%c     -->  character
 * 		%s     -->  string (Accompanying string must be null terminated)
 * 		%u     -->  unsigned decimal integer
 * 		%d, %i -->  decimal integer
 * 		%x, %X -->  hexadecimal (Capital 'X' results in uppercase hex value)
 * 		%f     -->  float (Or double)
 * 		%p     -->  pointer (Displayed as hex with a implicit width of 8)
 *
 * Width:
 * 		[int]  -->  Left pads(by default) the format specifier to the given width.
 * 						Use the "-" flag to right pad.
 *
 * Precision:
 * 		.[int]  --> Sets a specific precision when using the float specifier. The
 * 						default precision is 1.
 *
 * Flags:
 * 		'0'  -->  Pads float, integer, and hexadecimal specifiers with zeros. Does
 * 					nothing if a width is not specified. Defaults to left padding.
 * 		' '  -->  Pads any specifier with spaces. Does nothing if a with is not
 * 					specified. Defaults to left padding.
 * 		'+'  -->  Forces the display of positive sign with integer and float specifiers
 * 					when positive.
 * 		'-'  -->  Changes the width to be a right padding instead of the default
 * 					left padding.
 * 		'''  -->  The thousands separator will be shown with int specifiers.
 */
size_t vsnprintf(char *restrict buffer, size_t buffer_size, const char *restrict format, va_list args)
{
	size_t written = 0;

	while (*format != '\0')
	{
		if (*format != '%')
		{
			buffer[written++] = *(format++);
		}
		else
		{
			format++;

			// Output an escaped '%'
			if (*format == '%')
			{
				buffer[written++] = *(format++);
				continue;
			}

			struct FORMAT_SPECIFIER f;
			f.flags = 0;
			f.precision = 1;
			f.width = 0;
			f.specifier = '\0';

			// Check for flags
			while (is_flag(*format))
			{
				switch (*format)
				{
				case '0':
					f.flags |= ZERO_FLAG;
					break;
				case ' ':
					f.flags |= SPACE_FLAG;
					break;
				case '-':
					f.flags |= MINUS_FLAG;
					break;
				case '+':
					f.flags |= PLUS_FLAG;
					break;
				case '#':
					f.flags |= POUND_FLAG;
					break;
				case '\'':
					f.flags |= APOSTROPHE_FLAG;
					break;
				}

				format++;
			}

			// Check for a width modifier
			while (is_numeric(*format))
			{
				f.width *= 10;
				f.width += parse_byte(*format);

				format++;
			}

			// Check for precision modifier
			if (*format == '.')
			{
				format++;

				while (is_numeric(*format))
				{
					f.precision *= 10;
					f.precision += parse_byte(*format);

					format++;
				}
			}

			f.specifier = *(format++);

			switch (f.specifier)
			{
			case 'c':
				buffer[written++] = (char)va_arg(args, int);
				break;
			case 's':
				const char *str = (const char *)va_arg(args, const char *);

				strcpy(buffer + written, str);

				if (f.width)
				{
					if (f.flags & MINUS_FLAG)
					{
						rightpad(buffer + written, buffer_size - written, f.width, ' ');
					}
					else
					{
						leftpad(buffer + written, buffer_size - written, f.width, ' ');
					}
				}

				written += strlen(buffer + written);
				break;
			case 'd':
			case 'i':
				written += snprintf_int(buffer + written, buffer_size - written, (int64_t)va_arg(args, int32_t), &f);
				break;
			case 'u':
				written += snprintf_int(buffer + written, buffer_size - written, (uint64_t)va_arg(args, uint32_t), &f);
				break;
			case 'x':
			case 'X':
				written += snprintf_hex(buffer + written, buffer_size - written, (uint64_t)va_arg(args, uint32_t), &f);
				break;
			case 'p':
				f.specifier = 'x';
				f.width = 18;
				f.flags = POUND_FLAG | ZERO_FLAG;
				written += snprintf_hex(buffer + written, buffer_size - written, (uint64_t)va_arg(args, void *), &f);
				break;
			default:
				strcpy(buffer + written, "[INVALID SPECIFIER]");
				written += strlen(buffer);
				break;
			}
		}
	}

	va_end(args);

	buffer[written] = '\0';

	return written;
}

static bool is_flag(char c)
{
	switch (c)
	{
	case '0':
	case ' ':
	case '+':
	case '-':
	case '#':
	case '\'':
		return true;
	}

	return false;
}

static bool is_numeric(char c)
{
	return c > 47 && c < 58;
}

static int parse_byte(char c)
{
	if (c - 48 < 0 || c - 48 > 9)
		return 0; // TODO How to handle NaN?

	return c - 48;
}

static char get_pad_char(uint8_t flags)
{
	if (flags & ZERO_FLAG)
	{
		return '0';
	}
	else
	{
		return ' ';
	}
}

static size_t snprintf_int(char *buffer, size_t buffer_size, int64_t num, struct FORMAT_SPECIFIER *f)
{
	size_t written = 0;

	// The biggest unsigned 64 bit int is 26 chars, 1 sign, and 1 null byte equaling 28
	// bytes needed for the biggest possible num. We will for now do nothing if the buffer
	// does not have atleast this(It should).
	if (buffer_size < 28)
	{
		return written;
	}

	// Convert num to int string
	for (int i = num; i != 0; i /= 10)
	{
		char digit = (i % 10) + 48;

		if ((f->flags & APOSTROPHE_FLAG) && ((written + 1) % 4) == 0)
		{
			buffer[written++] = ',';
		}

		buffer[written++] = digit;
	}

	// Ensure atleast a zero is displayed at minimum
	if (written == 0)
	{
		buffer[written++] = '0';
	}

	// Add relevant sign as needed.
	if (num < 0)
	{
		buffer[written++] = '-';
	}
	else if (num >= 0 && f->flags & PLUS_FLAG)
	{
		buffer[written++] = '+';
	}

	buffer[written] = '\0';

	// Reverse the string because decoding results in a reversed string
	strrev(buffer);

	if (written < f->width && f->width < buffer_size)
	{
		int prefix_length = (f->flags & PLUS_FLAG && num >= 0) || (num < 0) ? 1 : 0;

		char *buffer_offset = buffer + prefix_length;
		int remaining_buffer_size = buffer_size - prefix_length;

		if (f->flags & MINUS_FLAG)
		{
			written += rightpad(buffer_offset, remaining_buffer_size, f->width - prefix_length, ' ');
		}
		else
		{
			written += leftpad(buffer_offset, remaining_buffer_size, f->width - prefix_length, get_pad_char(f->flags));
		}
	}

	return written;
}

static const char HEX_LOOKUP_LOWER[] = "0123456789abcdef";
static const char HEX_LOOKUP_UPPER[] = "0123456789ABCDEF";

static size_t snprintf_hex(char *buffer, size_t buffer_size, uint64_t num, struct FORMAT_SPECIFIER *f)
{
	size_t written = 0;

	// The biggest unsigned 64 bit int is 16 chars and 1 null byte equaling 17
	// bytes needed for the biggest possible num. We will for now do nothing if
	// the buffer does not have atleast this(It should).
	if (buffer_size < 17)
	{
		return written;
	}

	bool is_uppercase = f->specifier == 'X';

	// Convert num to hex string
	for (int i = 7; i > -1; i--)
	{
		uint8_t *byte = (uint8_t *)&num + i;

		if (*byte == 0 && written == 0)
		{
			continue;
		}
		else
		{
			const char *hex_lookup = is_uppercase ? HEX_LOOKUP_UPPER : HEX_LOOKUP_LOWER;

			buffer[written++] = hex_lookup[(*byte & 0xf0) >> 4];
			buffer[written++] = hex_lookup[*byte & 0x0f];
		}
	}

	// Ensure atleast a zero is displayed at minimum
	if (written == 0)
	{
		buffer[written++] = '0';
	}

	buffer[written] = '\0';

	// Add the "0x" prefix if specified
	if (f->flags & POUND_FLAG)
	{
		written += leftpad(buffer, buffer_size, written + 2, ' ');
		buffer[0] = '0';
		buffer[1] = is_uppercase ? 'X' : 'x';
	}

	if (written < f->width && f->width < buffer_size)
	{
		int prefix_length = (f->flags & POUND_FLAG) ? 2 : 0;

		char *buffer_offset = buffer + prefix_length;
		int remaining_buffer_size = buffer_size - prefix_length;

		if (f->flags & MINUS_FLAG)
		{
			written += rightpad(buffer_offset, remaining_buffer_size, f->width - prefix_length, ' ');
		}
		else
		{
			written += leftpad(buffer_offset, remaining_buffer_size, f->width - prefix_length, get_pad_char(f->flags));
		}
	}

	return written;
}