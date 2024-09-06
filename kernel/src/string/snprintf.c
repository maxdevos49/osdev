#include <stdint.h>
#include <stddef.h>

#include "utility.h"
#include "../graphics/graphics.h"
#include "../devices/tty.h"

size_t snprintf(char *restrict buffer, size_t buffer_size, const char *restrict format, ...)
{
    va_list args;
    va_start(args, format);

    size_t written = vsnprintf(buffer, buffer_size, format, args);

    va_end(args);

    return written;
}