#include "physical.h"
#include "../error.h"
#include "../macro.h"
#include "../string/utility.h"
#include "memory.h"
#include <stdbool.h>
#include <stddef.h>

#define PAGES_PER_BYTE (8ULL)
#define PAGES_PER_BITMAP_INDEX (64ULL)

struct PHYSICAL_MEMORY_CONTEXT {
	size_t bitmap_size;
	uint64_t *bitmap;
	uint64_t used_pages;
	uint64_t total_pages;
};

static struct PHYSICAL_MEMORY_CONTEXT _pm_context = {0};

// Rounds up a size if needed to match page boundaries
static inline size_t page_align_size(size_t size_in_bytes)
{
	if (size_in_bytes % PAGE_BYTE_SIZE) {
		return size_in_bytes +
			   (PAGE_BYTE_SIZE - (size_in_bytes % PAGE_BYTE_SIZE));
	}

	return size_in_bytes;
}

// Gets the total system memory in bytes.
size_t total_system_memory(void)
{
	size_t total = 0;

	for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
		total += memmap_request.response->entries[i]->length;
	}

	return total;
}

// Calculates the required physical memory manager bitmap size.
static size_t bitmap_required_size(size_t total_system_memory_in_bytes)
{
	size_t total_pages = total_system_memory_in_bytes / PAGE_BYTE_SIZE;
	return page_align_size(total_pages / PAGES_PER_BYTE);
}

// Gets the number of pages pages on the size in bytes.
static inline size_t size_to_num_of_pages(size_t size_in_bytes)
{
	return (size_in_bytes / PAGE_BYTE_SIZE) +
		   (size_in_bytes % PAGE_BYTE_SIZE ? 1 : 0);
}

// Determines if the page is used.
static inline bool is_page_used(struct PHYSICAL_MEMORY_CONTEXT *memory,
								uintptr_t page_index)
{
	return memory->bitmap[page_index / PAGES_PER_BITMAP_INDEX] &
		   (1ULL << (page_index % PAGES_PER_BITMAP_INDEX));
}

// Gets the index into the paging bitmap.
static inline uint64_t
addr_to_page_index(struct PHYSICAL_MEMORY_CONTEXT *memory, phys_addr_t addr)
{
	if (addr % PAGE_BYTE_SIZE != 0) {
		// Address is not aligned.
		printf(
			KERROR
			"Cannot convert address to page index. Address is not aligned.\n");
		return INVALID_PHYS;
	}

	int page_index = addr / PAGE_BYTE_SIZE;

	if (page_index >= (int)memory->total_pages) {
		return INVALID_PHYS;
	}

	return page_index;
}

// Sets a single page frame as used.
static inline void reserve_page(struct PHYSICAL_MEMORY_CONTEXT *memory,
								uintptr_t page_index)
{
	memory->bitmap[page_index / PAGES_PER_BITMAP_INDEX] |=
		(1ULL << (page_index % PAGES_PER_BITMAP_INDEX));
	memory->used_pages++;
}

// Sets a single page frame as available
static inline void release_page(struct PHYSICAL_MEMORY_CONTEXT *memory,
								uintptr_t page_index)
{
	memory->bitmap[page_index / PAGES_PER_BITMAP_INDEX] &=
		~(1ULL << (page_index % PAGES_PER_BITMAP_INDEX));
	memory->used_pages--;
}

// Opens up a region of page frames to be able to be allocated for general
// purpose use.
bool release_memory(const phys_addr_t physical_address,
					const size_t size_in_bytes)
{
	uintptr_t page_index = addr_to_page_index(&_pm_context, physical_address);
	if (page_index == INVALID_PHYS) {
		return false;
	}

	size_t page_count = size_to_num_of_pages(size_in_bytes);

	for (uintptr_t i = page_index; i < page_index + page_count; i++) {
		// if (is_page_used(&_pm_context, i) == false)
		// {
		// 	printf(KERROR "Cannot mark a page 'available' which is already
		// marked 'available'. Page Index: %ld\n", i); 	return false;
		// }

		release_page(&_pm_context, i);
	}

	return true;
}

// Closes a region of page frames to not be used.
bool reserve_memory(const phys_addr_t physical_address,
					const size_t size_in_bytes)
{
	uintptr_t page_index = addr_to_page_index(&_pm_context, physical_address);
	if (page_index == INVALID_PHYS) {
		return false;
	}

	size_t page_count = size_to_num_of_pages(size_in_bytes);

	for (uintptr_t i = page_index; i < page_index + page_count; i++) {
		if (is_page_used(&_pm_context, i) == true) {
			printf(KERROR "Cannot mark a page 'used' which is already marked "
						  "'used'. Page Index: %ld\n",
				   page_index);
			return false;
		}

		reserve_page(&_pm_context, i);
	}

	return true;
}

// Determines if a set of pages can be allocated at a given index sequentially.
static bool can_store_pages(uintptr_t page_index, size_t size_in_pages,
							size_t total_pages)
{
	for (uintptr_t i = page_index; i < page_index + size_in_pages; i++) {
		if (i >= total_pages ||
			is_page_used(&_pm_context, page_index)) // TODO pass in
		{
			return false;
		}
	}

	return true;
}

// Finds a set of pages
static uintptr_t find_pages(size_t pages_needed, uintptr_t start_page,
							uintptr_t end_page)
{
	for (uintptr_t i = start_page; i < end_page; i++) {
		if (can_store_pages(i, pages_needed, end_page)) {
			return i;
		}
	}

	printf(KERROR "Out of memory! Failed to find %lu unallocated pages in "
				  "sequencial order.\n",
		   pages_needed);
	return INVALID_PHYS;
}

// Allocates the needed physical pages for the given size in bytes.
phys_addr_t allocate_memory(const size_t size_in_bytes)
{
	size_t pages_needed = size_to_num_of_pages(size_in_bytes);
	uintptr_t page_index = find_pages(pages_needed, 1, _pm_context.total_pages);

	if (page_index == INVALID_PHYS) {
		return INVALID_PHYS;
	}

	phys_addr_t address = page_index * PAGE_BYTE_SIZE;
	if (reserve_memory(address, size_in_bytes) == false) {
		return INVALID_PHYS;
	}

	return address;
}

// Deallocates the physical pages given.
bool deallocate_memory(const phys_addr_t physical_address,
					   const size_t size_in_bytes)
{
	return release_memory(physical_address, size_in_bytes);
}

struct MEMORY_BITMAP init_physical_memory(void)
{
	printf(KINFO "Initiating physical memory management...\n");

	size_t total_system_memory_in_bytes = total_system_memory();
	size_t bitmap_size_in_bytes =
		bitmap_required_size(total_system_memory_in_bytes);

	printf("\tTotal system memory: %'ld bytes\n", total_system_memory_in_bytes);
	printf("\tTotal pages: %'lld\n",
		   total_system_memory_in_bytes / PAGE_BYTE_SIZE);

	struct limine_memmap_entry *suitable_bitmap_entry = NULL;

	// Find the smallest usable region to place the bitmap.
	for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
		struct limine_memmap_entry *entry = memmap_request.response->entries[i];

		if (entry->type != LIMINE_MEMMAP_USABLE ||
			entry->length < bitmap_size_in_bytes)
			continue;

		if (suitable_bitmap_entry == NULL ||
			entry->length < suitable_bitmap_entry->length)
			suitable_bitmap_entry = entry;
	}

	if (suitable_bitmap_entry == NULL) {
		abort("Failed to find a memory region for the paging bitmap.\n");
	}

	_pm_context.total_pages = total_system_memory_in_bytes / PAGE_BYTE_SIZE;
	_pm_context.used_pages = total_system_memory_in_bytes / PAGE_BYTE_SIZE;
	_pm_context.bitmap_size = bitmap_size_in_bytes;
	_pm_context.bitmap = (uint64_t *)(suitable_bitmap_entry->base +
									  hhdm_request.response->offset);

	printf(KINFO "Setting up memory bitmap...\n");
	printf("\tBitmap address: %p\n", _pm_context.bitmap);
	printf("\tBitmap size: %'ld bytes\n", _pm_context.bitmap_size);

	// Mark everything unavailable by default.
	memset(_pm_context.bitmap, 0xff, _pm_context.bitmap_size);

	printf(KINFO "Releasing usable memory regions...\n");

	// Now mark only usable memory regions as available
	for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
		struct limine_memmap_entry *entry = memmap_request.response->entries[i];

		if (entry->type != LIMINE_MEMMAP_USABLE) {
			continue;
		}

		if (release_memory(entry->base, entry->length) == false) {
			printf(KERROR "Failed to release memory region %#018lx - %#018lx\n",
				   entry->base, entry->base + entry->length - 1);
			abort("^^^^^^"); // Lol lets make a panic, panicp, and panicpf
		}
	}

	if (reserve_memory(suitable_bitmap_entry->base, bitmap_size_in_bytes) ==
		false) {
		printf(KERROR "Failed to reserve memory region %#018lx - %#018lx\n",
			   suitable_bitmap_entry->base,
			   suitable_bitmap_entry->base + bitmap_size_in_bytes - 1);
		abort("^^^^^^"); // Lol lets make a panic, panicp, and panicpf
	}

	printf("\tUsable free memory: %'llu bytes\n",
		   (_pm_context.total_pages - _pm_context.used_pages) * PAGE_BYTE_SIZE);

	struct MEMORY_BITMAP bitmap = {0};

	bitmap.address = _pm_context.bitmap;
	bitmap.size = _pm_context.bitmap_size;

	printf(KOK "Physical memory management ready\n");

	return bitmap;
}
