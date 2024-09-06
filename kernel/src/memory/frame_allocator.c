// /**
//  * Frame Allocator for use with paging.
//  *
//  * @author Maxwell DeVos
//  * @date 9/3/2024
//  */

// #include <stdint.h>
// #include <stddef.h>
// #include <limine.h>

// #include "utility.h"
// #include "../error.h"
// #include "../string/utility.h"

// #define FRAME_SIZE (4096ULL)
// #define FRAMES_PER_BYTE (8ULL)

// static uint64_t *_mmap;
// static uint64_t _max_frames;
// static uint64_t _used_frames;

// static uint64_t _lowest_free_index;
// static uint64_t _highest_free_index;


// static inline void set_frame(const uint64_t frame)
// {
// 	_mmap[frame / 64] |= (1ULL << (frame % 64));
// }

// static inline void unset_frame(const uint64_t frame)
// {
// 	_mmap[frame / 64] &= ~(1ULL << (frame % 64));
// }

// void *allocate_frames(const uint64_t num_frames)
// {
// 	// Do we have enough frames?
// 	if ((_max_frames - _used_frames) < num_frames)
// 	{
// 		return NULL;
// 	}

// 	uint64_t i = 0;
// 	while (_mmap[i] == 0xffffffffffffffff && i < _max_frames / 64)
// 	{
// 		i++;
// 	}

// 	// Were any frames available?
// 	if (_mmap[i] == 0xffffffffffffffff)
// 	{
// 		return NULL;
// 	}

// 	uint64_t ii = 0;
// 	while (((_mmap[i] >> ii) & 0x0000000000000001) != 0 && ii < 64)
// 	{
// 		ii++;
// 	}

// 	uint64_t index = (i * 64ULL) + ii;
// 	// TODO support multiple contiguous frames
// 	set_frame(index);
// 	_used_frames++;

// 	return (void *)(index * FRAME_SIZE);
// }

// // Frees a page frame.
// void free_frames(const uint64_t start_address, uint64_t num_frames)
// {
// 	uint64_t frame_index = start_address / FRAME_SIZE;

// 	if (frame_index < _lowest_free_index)
// 	{
// 		_lowest_free_index = frame_index;
// 	}

// 	if (frame_index > _highest_free_index)
// 	{
// 		_highest_free_index = frame_index;
// 	}

// 	uint64_t i;
// 	for (i = 0; i < num_frames; i++)
// 	{
// 		unset_frame(frame_index++);
// 		_used_frames--;
// 	}
// }

// // Opens up a region of page frames to be able to be allocated for general
// // purpose use.
// void open_frame_region(const uint64_t start_address, const size_t size)
// {
// 	size_t aligned_size = align_size(size);

// 	uint64_t frame_index = start_address / FRAME_SIZE;
// 	uint64_t frame_count = aligned_size / FRAME_SIZE;

// 	printf("Opening page frames with address range: %p to %p\t%'d bytes\n", start_address, start_address + size, size);

// 	for (; frame_count > 0; frame_count--)
// 	{
// 		if (frame_index < _lowest_free_index)
// 		{
// 			_lowest_free_index = frame_index;
// 		}

// 		if (frame_index > _highest_free_index)
// 		{
// 			_highest_free_index = frame_index;
// 		}

// 		unset_frame(frame_index++);
// 		_used_frames--;
// 	}
// }

// // Closes a region of page frames to not be able to be allocated for general
// // purpose use.
// void close_frame_region(const uint64_t start_address, const size_t size)
// {
// 	// Get the page aligned size rounded up.
// 	size_t aligned_size = align_size(size);

// 	uint64_t frame_index = start_address / FRAME_SIZE;
// 	uint64_t frame_count = aligned_size / FRAME_SIZE;

// 	printf("Closing page frames with address range: %p to %p\t%'d bytes\n", start_address, start_address + size, size);

// 	for (; frame_count > 0; frame_count--)
// 	{
// 		set_frame(frame_index++);
// 		_used_frames++;
// 	}
// }

// // Gets the required size in bytes for use with the frame allocator.
// size_t frame_allocator_required_size(const size_t total_system_memory_in_bytes)
// {

// 	// printf("Total: %'d bytes\n, %'d\n", total_system_memory_in_bytes);

// 	size_t required_bytes = (total_system_memory_in_bytes / FRAME_SIZE) / FRAMES_PER_BYTE;

// 	// Make sure the size is a multiple of 8 bytes
// 	return required_bytes + (required_bytes % ((size_t)8));
// }

// // Sets up the frame allocator. Sets all frames to be closed by default.
// void initialize_frame_allocator(const void *start_address, const size_t size)
// {
// 	// Test address alignment
// 	if ((uint64_t)start_address % sizeof(uint64_t) != 0)
// 	{
// 		abort("Frame allocator address is not 8 byte aligned.");
// 	}

// 	// Test size multiples
// 	if (size % sizeof(uint64_t) != 0)
// 	{
// 		printf("Size: %'d bytes\n", size);
// 		abort("Frame allocator size is not a multiple of 8.");
// 	}

// 	_mmap = (uint64_t *)start_address;
// 	_max_frames = size * FRAMES_PER_BYTE;
// 	_used_frames = _max_frames;

// 	_lowest_free_index = _max_frames;
// 	_highest_free_index = _max_frames;

// 	printf("Initializing Page Frame Allocator | Size: %'d bytes\n", size);

// 	memset(_mmap, 0xff, size);

// 	struct MEMORY_MAP *memory_map = memory_map_copy();

// 	// Open frames with usable memory
// 	for (int i = 0; i < memory_map->entry_count; i++)
// 	{
// 		struct MEMORY_MAP_ENTRY entry = memory_map->entries[i];
// 		if (entry.type == LIMINE_MEMMAP_USABLE)
// 		{
// 			open_frame_region(entry.base, entry.size);
// 		}
// 	}

// 	// Close frames used by the frame allocator itself
// 	close_frame_region((uint64_t)start_address - memory_map->hhdm_offset, size);

// 	printf("Page Frame Allocator ready | Total Frames: %'d | Available Frames: %'d\n", _max_frames, _used_frames);
// }
