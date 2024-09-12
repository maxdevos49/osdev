#ifndef __MEMORY_HEAP_H
#define __MEMORY_HEAP_H 1

#include <stddef.h>

void init_heap(void *heap_address, size_t size);
void print_heap(void);

void *kmalloc(size_t size);
void kfree(void *ptr);

#endif
