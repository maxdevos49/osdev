#ifndef __MEMORY_PHYSICAL_H
#define __MEMORY_PHYSICAL_H 1

#include "memory.h"
#include "type.h"
#include <stdbool.h>
#include <stddef.h>

struct MEMORY_BITMAP init_physical_memory(void);

err_code reserve_memory(const phys_addr_t physical_address,
						const size_t size_in_bytes);

err_code release_memory(const phys_addr_t physical_address,
						const size_t size_in_bytes);

err_code allocate_memory(const size_t size_in_bytes,
						 phys_addr_t *output_physical_address);

#endif
