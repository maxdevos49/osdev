#include <stdint.h>
#include <stddef.h>

#include "utility.h"

size_t strlen(const char *restrict str)
{
    size_t len = 0;
    while (str[len])
        len++;

    return len;
}