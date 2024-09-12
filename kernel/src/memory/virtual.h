#ifndef __MEMORY_VIRTUAL_H
#define __MEMORY_VIRTUAL_H 1

#include <stddef.h>
#include <stdint.h>
#include "memory.h"

enum PAGE_FLAGS
{
	PAGE_MAP_WRITEABLE = 1,
	PAGE_MAP_USER = 1 << 1,
};

void init_virtual_memory(struct MEMORY_BITMAP bitmap);
void print_memory_mapping(void);

bool map_memory(phys_addr_t physical_address, virt_addr_t virtual_address, size_t size_in_bytes, uint32_t flags);
// bool unmap_memory(virt_addr_t virtual_address, size_t size_in_bytes); // TODO implement when needed

#endif
