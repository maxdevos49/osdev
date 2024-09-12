#ifndef __MEMORY_MEMORY_H
#define __MEMORY_MEMORY_H 1

#include <stdint.h>
#include <limine.h>

#define PAGE_BYTE_SIZE (4096ULL)
#define INVALID_PHYS (1ULL << 52)

void init_memory(void);
void print_memory_layout(void);

extern volatile struct limine_memmap_request memmap_request;
extern volatile struct limine_hhdm_request hhdm_request;
extern volatile struct limine_kernel_address_request kernel_address_request;

typedef void *virt_addr_t;
typedef uintptr_t phys_addr_t;

// Converts a physical address to a kernel space virtual address.
static inline virt_addr_t phys_to_virt(const phys_addr_t physical_address)
{
	return (virt_addr_t)(physical_address + hhdm_request.response->offset);
}

// Converts a kernel space virtual address to a physical address.
static inline phys_addr_t virt_to_phys(const virt_addr_t virtual_address)
{
	return (phys_addr_t)(virtual_address - hhdm_request.response->offset);
}

struct MEMORY_BITMAP
{
	virt_addr_t address;
	size_t size;
};

#endif
