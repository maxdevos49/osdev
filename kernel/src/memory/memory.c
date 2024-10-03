#include "../error.h"
#include "../macro.h"
#include "../string/utility.h"
#include "heap.h"
#include "physical.h"
#include "virtual.h"
#include <limine.h>
#include <stddef.h>

#define HEAP_INITIAL_SIZE (0x1000 * 5)

extern char kernel_end; // Last address in the kernel

ATTR_REQUEST volatile struct limine_memmap_request memmap_request = {
	.id = LIMINE_MEMMAP_REQUEST, .revision = 0};
ATTR_REQUEST volatile struct limine_hhdm_request hhdm_request = {
	.id = LIMINE_HHDM_REQUEST, .revision = 0};
ATTR_REQUEST volatile struct limine_kernel_address_request
	kernel_address_request = {.id = LIMINE_KERNEL_ADDRESS_REQUEST,
							  .revision = 0};

static char MEMORY_MAP_REGION_NAME[8][24] = {"Usable",
											 "Reserved",
											 "ACPI Reclaimable",
											 "ACPI NVS",
											 "Bad Memory",
											 "Bootloader reclaimable",
											 "Kernel and modules",
											 "Framebuffer"};

// Prints the bootloader memory map.
void print_memory_layout(void)
{
	size_t total = 0;

	printf("Memory Layout:\n");

	for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
		struct limine_memmap_entry *entry = memmap_request.response->entries[i];

		total += entry->length;

		printf("\t%-23s %#018lx-%#018lx Length: %'14lu bytes\n",
			   MEMORY_MAP_REGION_NAME[entry->type], entry->base,
			   entry->base + entry->length - 1, entry->length);
	}

	printf("Total system memory: %'lu bytes\n", total);
}

// Init the physical and virtual memory manager
void init_memory(void)
{
	if (memmap_request.response == NULL) {
		abort("Limine response to 'limine_memmap_request' was null\n");
		return;
	}

	if (memmap_request.response->entry_count == 0 ||
		memmap_request.response->entries == NULL) {
		abort("Limine memory map is empty\n");
		return;
	}

	if (hhdm_request.response == NULL) {
		abort("Limine response to 'hhdm_request' was null\n");
		return;
	}

	if (kernel_address_request.response == NULL) {
		abort("Limine response to 'kernel_address_request' was null\n");
		return;
	}

	print_memory_layout();

	struct MEMORY_BITMAP bitmap = init_physical_memory();

	init_virtual_memory(bitmap);

	// Get the page aligned address 1 page after the end of the kernel. 1 page
	// will ensure if malloc misbehaves and we overwrite we will get a page
	// fault.
	uintptr_t heap_virtual_address = (uintptr_t)&kernel_end;
	if (heap_virtual_address % PAGE_BYTE_SIZE) {
		heap_virtual_address +=
			(PAGE_BYTE_SIZE - (heap_virtual_address % PAGE_BYTE_SIZE));
	}

	init_heap((void *)heap_virtual_address, HEAP_INITIAL_SIZE);

	// TODO relocate stack

	// TODO release bootloader regions
}
