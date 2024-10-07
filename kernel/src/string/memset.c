#include <stddef.h>
#include <stdint.h>

void *memset(void *s, int c, size_t n)
{
	asm volatile("cld\n"
				 "rep stosb" ::"D"(s),
				 "c"(n), "a"(c));

	return s;
}
