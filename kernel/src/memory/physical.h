#ifndef __MEMORY_PHYSICAL_H
#define __MEMORY_PHYSICAL_H 1

#include <stddef.h>
#include <stdbool.h>
#include "memory.h"

struct MEMORY_BITMAP init_physical_memory(void);

bool reserve_memory(const phys_addr_t physical_address, const size_t size_in_bytes);
bool release_memory(const phys_addr_t physical_address, const size_t size_in_bytes);
phys_addr_t allocate_memory(const size_t size_in_bytes);
bool deallocate_memory(const phys_addr_t physical_address, const size_t size_in_bytes);

#endif
