#include "physical.h"
#include "../macro.h"
#include "../string/utility.h"
#include "debug.h"
#include "memory.h"
#include "panic.h"
#include "type.h"
#include <stdbool.h>
#include <stddef.h>

#define PAGES_PER_BYTE (8ULL)
#define PAGES_PER_BITMAP_INDEX (64ULL)

typedef struct {
	size_t bitmap_size;
	uint64_t *bitmap;
	uint64_t used_pages;
	uint64_t total_pages;
} Phys_Ctx;

static Phys_Ctx _ctx = {0};

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
static inline bool is_page_used(Phys_Ctx *memory, uint64_t page_index)
{
	return memory->bitmap[page_index / PAGES_PER_BITMAP_INDEX] &
		   (1ULL << (page_index % PAGES_PER_BITMAP_INDEX));
}

// Gets the page index into the paging bitmap. Returns an
// `ERROR_ADDRESS_ALIGNMENT` error code if the address is not page aligned or
// `ERROR_OUT_OF_BOUNDS` error code if the address exceeds the bounds of
// physical memory mapped by the memory bitmap.
static inline err_code addr_to_page_index(Phys_Ctx *memory,
										  phys_addr_t physical_address,
										  uint64_t *output_page_index)
{
	if (physical_address % PAGE_BYTE_SIZE != 0) {
		// Address is not page aligned.
		debug_code(ERROR_ADDRESS_ALIGNMENT);
		return ERROR_ADDRESS_ALIGNMENT;
	}

	uint64_t page_index = physical_address / PAGE_BYTE_SIZE;

	if (page_index >= memory->total_pages) {
		// Address exceeds bounds of physical memory mapped by the memory
		// bitmap.
		debug_code(ERROR_OUT_OF_BOUNDS);
		return ERROR_OUT_OF_BOUNDS;
	}

	*output_page_index = page_index;
	return 0;
}

// Sets a single page frame as used.
static inline void reserve_page(Phys_Ctx *memory, uintptr_t page_index)
{
	memory->bitmap[page_index / PAGES_PER_BITMAP_INDEX] |=
		(1ULL << (page_index % PAGES_PER_BITMAP_INDEX));
	memory->used_pages++;
}

// Sets a single page frame as available
static inline void release_page(Phys_Ctx *memory, uintptr_t page_index)
{
	memory->bitmap[page_index / PAGES_PER_BITMAP_INDEX] &=
		~(1ULL << (page_index % PAGES_PER_BITMAP_INDEX));
	memory->used_pages--;
}

// Opens up a region of page frames to be able to be allocated for general
// purpose use. Returns the error code `ERROR_ADDRESS_ALIGNMENT` if the physical
// page address is not page aligned or `ERROR_OUT_OF_BOUNDS` if the physical
// address is outside the bounds of the memory bitmap.
err_code release_memory(const phys_addr_t physical_address,
						const size_t size_in_bytes)
{
	err_code err = 0;
	uint64_t page_index = 0;

	if ((err = addr_to_page_index(&_ctx, physical_address, &page_index))) {
		debug_code(err);
		return err;
	}

	size_t page_count = size_to_num_of_pages(size_in_bytes);

	uint64_t page_index_end = page_index + page_count;
	if (page_index_end >= _ctx.total_pages) {
		debug_code(ERROR_OUT_OF_BOUNDS);
		return ERROR_OUT_OF_BOUNDS;
	}

	for (uint64_t i = page_index; i < page_index_end; i++) {
		release_page(&_ctx, i);
	}

	return 0;
}

// Reserves a region of pages to not be used. If the physical address is invalid
// the error codes `ERROR_ADDRESS_ALIGNMENT` or `ERROR_OUT_OF_BOUNDS` may be
// returned. If the address is already reserved the error code
// `ERROR_ALREADY_USED` will be returned.
err_code reserve_memory(phys_addr_t physical_address, size_t size_in_bytes)
{
	err_code err = 0;
	uint64_t page_index = 0;
	size_t page_count = size_to_num_of_pages(size_in_bytes);

	if ((err = addr_to_page_index(&_ctx, physical_address, &page_index))) {
		debug_code(err);
		return err;
	}

	uint64_t page_index_end = page_index + page_count;
	if (page_index_end >= _ctx.total_pages) {
		debug_code(ERROR_OUT_OF_BOUNDS);
		return ERROR_OUT_OF_BOUNDS;
	}

	for (uint64_t i = page_index; i < page_index_end; i++) {
		if (is_page_used(&_ctx, i) == true) {
			debug_code(ERROR_ALREADY_USED);
			return ERROR_ALREADY_USED;
		}

		reserve_page(&_ctx, i);
	}

	return 0;
}

// Determines if a set of pages can be allocated at a given index sequentially.
static bool can_store_pages(size_t pages_needed, uint64_t start_page,
							uint64_t end_page)
{
	if (start_page + pages_needed >= end_page) {
		return false;
	}

	for (uint64_t i = start_page; i < start_page + pages_needed; i++) {
		if (is_page_used(&_ctx, i)) {
			return false;
		}
	}

	return true;
}

// Finds a set of sequential pages for a range of page index's. If no
// consecutive free pages exist then a `ERROR_NOT_FOUND` error code is returned.
static err_code find_pages(size_t pages_needed, uint64_t start_page_index,
						   uint64_t end_page_index, uint64_t *output_index)
{
	for (uintptr_t i = start_page_index; i < end_page_index; i++) {
		if (can_store_pages(pages_needed, i, end_page_index)) {
			*output_index = i;
			return 0;
		}
	}

	debug_code(ERROR_NOT_FOUND);
	return ERROR_NOT_FOUND;
}

// Allocates a sequential set of pages for the given amount of memory. Returns
// the error code `ERROR_NOT_FOUND` if no sequential sets of pages are
// available.
err_code allocate_memory(const size_t size_in_bytes,
						 phys_addr_t *output_physical_address)
{
	err_code err = 0;

	size_t pages_needed = size_to_num_of_pages(size_in_bytes);
	uint64_t page_index = 0;

	if ((err = find_pages(pages_needed, 1, _ctx.total_pages, &page_index))) {
		debug_code(err);
		return err;
	}

	phys_addr_t address = page_index * PAGE_BYTE_SIZE;
	if ((err = reserve_memory(address, size_in_bytes))) {
		debug_code(err);
		return err;
	}

	*output_physical_address = address;
	return 0;
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
		panicf("Failed to find a memory region for the paging bitmap.\n");
	}

	_ctx.total_pages = total_system_memory_in_bytes / PAGE_BYTE_SIZE;
	_ctx.used_pages = total_system_memory_in_bytes / PAGE_BYTE_SIZE;
	_ctx.bitmap_size = bitmap_size_in_bytes;
	_ctx.bitmap = (uint64_t *)(suitable_bitmap_entry->base +
							   hhdm_request.response->offset);

	printf(KINFO "Setting up memory bitmap...\n");
	printf("\tBitmap address: %p\n", _ctx.bitmap);
	printf("\tBitmap size: %'ld bytes\n", _ctx.bitmap_size);

	// Mark everything unavailable by default.
	memset(_ctx.bitmap, 0xff, _ctx.bitmap_size);

	printf(KINFO "Releasing usable memory regions...\n");

	// Now mark only usable memory regions as available
	for (uint64_t i = 0; i < memmap_request.response->entry_count; i++) {
		struct limine_memmap_entry *entry = memmap_request.response->entries[i];

		if (entry->type != LIMINE_MEMMAP_USABLE) {
			continue;
		}

		if (release_memory(entry->base, entry->length)) {
			panicf("Failed to release memory region %#018lx - %#018lx\n",
				   entry->base, entry->base + entry->length - 1);
		}
	}

	if (reserve_memory(suitable_bitmap_entry->base, bitmap_size_in_bytes)) {
		panicf("Failed to reserve memory region %#018lx - %#018lx\n",
			   suitable_bitmap_entry->base,
			   suitable_bitmap_entry->base + bitmap_size_in_bytes - 1);
	}

	printf("\tUsable free memory: %'llu bytes\n",
		   (_ctx.total_pages - _ctx.used_pages) * PAGE_BYTE_SIZE);

	struct MEMORY_BITMAP bitmap = {0};

	bitmap.address = _ctx.bitmap;
	bitmap.size = _ctx.bitmap_size;

	printf(KOK "Physical memory management ready\n");

	return bitmap;
}
