#ifndef __STRING_UTILITY_H
#define __STRING_UTILITY_H 1

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdbool.h>

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
size_t strlen(const char *restrict str);
char *strcpy(char *restrict dst_ptr, const char *restrict src_ptr);
void strrev(char *str);
int strcmp(const char *s1, const char *s2);
size_t leftpad(char *str_buffer, size_t buffer_size, size_t desired_str_length, char character);
size_t rightpad(char *str_buffer, size_t buffer_size, size_t desired_str_length, char character);
__attribute__ ((format (printf, 3, 4))) size_t snprintf(char *restrict buffer, size_t buffer_size, const char *restrict format, ...);
size_t vsnprintf(char *restrict buffer, size_t buffer_size, const char *restrict format, va_list args);

__attribute__ ((format (printf, 1, 2))) int printf(const char *restrict format, ...);

#endif
