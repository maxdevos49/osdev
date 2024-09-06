#include <limine.h>
#include <stddef.h>

#include "../error.h"
#include "../string/utility.h"
#include "utility.h"

#define PAGE_SIZE (4096ULL)
#define FRAMES_PER_BYTE (8ULL)
#define NUM_OF_BITS (64ULL)

struct MEMORY_MAP_STATS
{
	size_t total_memory;
	size_t total_usable_memory;
	size_t total_reclaimable_memory;
	size_t total_reserved_memory;
	size_t total_bad_memory;
};

struct MEMORY_BITMAP
{
	size_t size;
	uint64_t *bitmap;
	uint64_t used_pages;
	uint64_t total_pages;
};

__attribute__((used, section(".requests"))) static volatile struct limine_memmap_request memmap_request = {
	.id = LIMINE_MEMMAP_REQUEST,
	.revision = 0};

__attribute__((used, section(".requests"))) static volatile struct limine_hhdm_request hhdm_request = {
	.id = LIMINE_HHDM_REQUEST,
	.revision = 0};

__attribute__((used, section(".requests"))) static volatile struct limine_kernel_address_request kernel_address_request = {
	.id = LIMINE_KERNEL_ADDRESS_REQUEST,
	.revision = 0};

static struct MEMORY_BITMAP _memory_bitmap = {0};

static char MEMORY_MAP_REGION_NAME[8][24] = {
	"Usable",
	"Reserved",
	"ACPI Reclaimable",
	"ACPI NVS",
	"Bad Memory",
	"Bootloader reclaimable",
	"Kernel and modules",
	"Framebuffer"};

// Prints the memory map provided by Limine.
static void pmm_memory_map_print(void)
{
	uint64_t i;
	for (i = 0; i < memmap_request.response->entry_count; i++)
	{
		struct limine_memmap_entry *entry = memmap_request.response->entries[i];
		printf("Base: %p Length: %'10d bytes Type: %s\n", (void*)entry->base, entry->length, MEMORY_MAP_REGION_NAME[entry->type]);
	}
}

// Aligns a size with a page size. Rounds up.
static inline size_t pmm_page_align_size(size_t size_in_bytes)
{
	if (size_in_bytes % PAGE_SIZE)
	{
		return size_in_bytes + (PAGE_SIZE - (size_in_bytes % PAGE_SIZE));
	}

	return size_in_bytes;
}

// Gets the memory map stats from the Limine.
static struct MEMORY_MAP_STATS pmm_memory_map_stats(void)
{
	struct MEMORY_MAP_STATS stats = {
		.total_memory = 0,
		.total_bad_memory = 0,
		.total_reclaimable_memory = 0,
		.total_reserved_memory = 0,
		.total_usable_memory = 0};

	uint64_t i;
	for (i = 0; i < memmap_request.response->entry_count; i++)
	{
		struct limine_memmap_entry *entry = memmap_request.response->entries[i];

		stats.total_memory += entry->length;

		switch (entry->type)
		{
		case LIMINE_MEMMAP_USABLE:
		case LIMINE_MEMMAP_KERNEL_AND_MODULES:
			stats.total_usable_memory += entry->length;
			break;
		case LIMINE_MEMMAP_RESERVED:
		case LIMINE_MEMMAP_ACPI_NVS:
		case LIMINE_MEMMAP_FRAMEBUFFER:
			stats.total_reserved_memory += entry->length;
			break;
		case LIMINE_MEMMAP_ACPI_RECLAIMABLE:
		case LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE:
			stats.total_reclaimable_memory += entry->length;
			break;
		case LIMINE_MEMMAP_BAD_MEMORY:
			stats.total_bad_memory += entry->length;
			break;
		}
	}

	return stats;
}

// Calculates the required physical memory manager bitmap size.
static size_t pmm_bitmap_size(size_t total_system_memory_in_bytes)
{
	size_t total_pages = total_system_memory_in_bytes / PAGE_SIZE;
	return pmm_page_align_size(total_pages / FRAMES_PER_BYTE);
}

// Determines if the page is used.
static inline bool is_page_used(uintptr_t page_index)
{
	return _memory_bitmap.bitmap[page_index / NUM_OF_BITS] & (1ULL << (page_index % NUM_OF_BITS));
}

// Gets the index into the paging bitmap.
static inline uintptr_t pmm_addr_to_page_index(phys_addr_t addr)
{
	if (addr % PAGE_SIZE != 0)
	{
		// Address is not aligned.
		return INVALID_PHYS;
	}

	uintptr_t page_index = addr / PAGE_SIZE;

	if (page_index >= _memory_bitmap.total_pages)
	{
		return INVALID_PHYS;
	}

	return page_index;
}

// Gets the number of pages pages on the size in bytes.
static inline size_t pmm_size_to_num_of_pages(size_t size_in_bytes)
{
	return (size_in_bytes / PAGE_SIZE) + (size_in_bytes % PAGE_SIZE ? 1 : 0);
}

static inline void mark_page_used(uintptr_t page_index)
{
	_memory_bitmap.bitmap[page_index / NUM_OF_BITS] |= (1ULL << (page_index % NUM_OF_BITS));
	_memory_bitmap.used_pages++;
}

static inline void mark_page_available(uintptr_t page_index)
{
	_memory_bitmap.bitmap[page_index / NUM_OF_BITS] &= ~(1ULL << (page_index % NUM_OF_BITS));
	_memory_bitmap.used_pages--;
}

// Opens up a region of page frames to be able to be allocated for general
// purpose use.
uintptr_t pmm_mark_addr_available(const phys_addr_t addr, const size_t size)
{
	uintptr_t page_index = pmm_addr_to_page_index(addr);

	if (page_index == INVALID_PHYS)
	{
		return INVALID_PHYS;
	}

	size_t page_count = pmm_size_to_num_of_pages(size);

	for (uintptr_t i = page_index; i < page_index + page_count; i++)
	{
		if (is_page_used(i) == false)
		{
			return INVALID_PHYS;
		}

		mark_page_available(i);
	}

	return page_index;
}

// Closes a region of page frames to not be used.
uintptr_t pmm_mark_addr_used(const phys_addr_t addr, const size_t size)
{
	uintptr_t page_index = pmm_addr_to_page_index(addr);

	if (page_index == INVALID_PHYS)
	{
		return INVALID_PHYS;
	}

	size_t page_count = pmm_size_to_num_of_pages(size);

	for (uintptr_t i = page_index; i < page_index + page_count; i++)
	{
		if (is_page_used(i) == true)
		{
			return INVALID_PHYS;
		}

		mark_page_used(i);
	}

	return page_index;
}

// Init the physical and virtual memory manager
void init_memory(void)
{
	if (memmap_request.response == NULL)
	{
		abort("Limine response to 'limine_memmap_request' was null\n");
		return;
	}

	if (memmap_request.response->entry_count == 0 || memmap_request.response->entries == NULL)
	{
		abort("Limine memory map is empty\n");
		return;
	}

	if (hhdm_request.response == NULL)
	{
		abort("Limine response to 'hhdm_request' was null\n");
		return;
	}

	if (kernel_address_request.response == NULL)
	{
		abort("Limine response to 'kernel_address_request' was null\n");
		return;
	}

	pmm_memory_map_print();

	struct MEMORY_MAP_STATS stats = pmm_memory_map_stats();

	size_t bitmap_size = pmm_bitmap_size(stats.total_memory);
	struct limine_memmap_entry *suitable_bitmap_entry = NULL;

	// Find the smallest usable region to place the bitmap.
	for (uint64_t i = 0; i < memmap_request.response->entry_count; i++)
	{
		struct limine_memmap_entry *entry = memmap_request.response->entries[i];

		if (entry->type != LIMINE_MEMMAP_USABLE || entry->length < bitmap_size)
			continue;

		if (suitable_bitmap_entry == NULL || entry->length < suitable_bitmap_entry->length)
			suitable_bitmap_entry = entry;
	}

	if (suitable_bitmap_entry == NULL)
	{
		abort("Failed to find a memory region for the paging bitmap.\n");
		return;
	}

	_memory_bitmap.total_pages = stats.total_memory / PAGE_SIZE;
	_memory_bitmap.used_pages = stats.total_memory / PAGE_SIZE;
	_memory_bitmap.size = bitmap_size;
	_memory_bitmap.bitmap = (uint64_t *)(suitable_bitmap_entry->base + hhdm_request.response->offset);

	printf("Storing page frame bitmap at %p %'d bytes\n", _memory_bitmap.bitmap, _memory_bitmap.size);

	// Mark everything unavailable by default.
	memset(_memory_bitmap.bitmap, 0xff, _memory_bitmap.size);

	// Now mark only usable memory regions as available
	for (uint64_t i = 0; i < memmap_request.response->entry_count; i++)
	{
		struct limine_memmap_entry *entry = memmap_request.response->entries[i];

		if (entry->type != LIMINE_MEMMAP_USABLE)
			continue;

		if (pmm_mark_addr_available(entry->base, entry->length) == INVALID_PHYS)
		{
			printf("Failed to mark region %p %p available for paging.\n", entry->base, entry->base + entry->length);
			abort("^^^^^^"); // Lol lets make a panic, panicp, and panicpf
		}
	}

	if (pmm_mark_addr_used(suitable_bitmap_entry->base, bitmap_size) == INVALID_PHYS)
	{
		printf("Failed to mark region %p %p as closed for paging.\n", suitable_bitmap_entry->base, suitable_bitmap_entry->base + bitmap_size);
		abort("^^^^^^"); // Lol lets make a panic, panicp, and panicpf
	}

	printf("Initializing virtual memory\n");
}
