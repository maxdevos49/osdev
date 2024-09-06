#include "utility.h"

char *strcpy(char *restrict dst_ptr, const char *restrict src_ptr)
{
    return memcpy(dst_ptr, src_ptr, strlen(src_ptr) + 1);
}