#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#include "fonts/font.h"
#include "string/utility.h"
#include "memory/utility.h"
#include "graphics/graphics.h"
#include "devices/tty.h"
#include "error.h"

// TODO list
// Setup own stack
// Setup own GDT
// Setup own Paging
// Setup own IDT
// Then claim all bootloader reclaimable memory

__attribute__((used, section(".requests"))) static volatile LIMINE_BASE_REVISION(2);

// __attribute__((used, section(".requests"))) static volatile struct limine_stack_size_request stack_size_request = {
// 	.id = LIMINE_STACK_SIZE_REQUEST,
// 	.revision = 0};

// __attribute__((used, section(".requests_start_marker"))) static volatile LIMINE_REQUESTS_START_MARKER;
// __attribute__((used, section(".requests_end_marker"))) static volatile LIMINE_REQUESTS_END_MARKER;

static void hcf()
{
	for (;;)
	{
		asm("hlt");
	}
}

void abort(const char *error)
{
	set_stroke(GRAPHICS_get_global_context(), 0xff0000);
	printf("ABORT: %s\n", error);

	hcf();
}

void kmain(void)
{
	// Ensure the bootloader actually understands our base revision (see spec).
	if (LIMINE_BASE_REVISION_SUPPORTED == false)
	{
		hcf();
	}

	// TODO setup serial printing

	struct FONT font;
	PSF2_load_font(&font);
	GRAPHICS_init(&font);

	GRAPHICS_CONTEXT *ctx = GRAPHICS_get_ctx(SINGLE, 0, 0, get_screen_width(), get_screen_height());

	uint32_t color = 0;
	uint32_t color_mask = 0x0f000000;
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 16; j++)
		{
			color = (color_mask >> (j * 2));
			set_fill(ctx, color);
			fill_rect(ctx, ((get_screen_width() / 32) * j) + get_screen_width() / 2 + 150, (get_screen_height() / 10) * i, 50, 200);
		}
	}

	TTY_init();

	init_memory();

	// memory_map_print();

	// struct MEMORY_MAP_STATS memory_stats = memory_map_stats();
	// size_t frame_allocator_size = frame_allocator_required_size(memory_stats.total_memory);
	// void *frame_allocator_address = memory_map_first_usable_frame(frame_allocator_size);
	// if (frame_allocator_address == NULL)
	// {
	// 	abort("Failed to calculate a valid address for the Frame Allocator.");
	// }

	// initialize_frame_allocator(frame_allocator_address, frame_allocator_size);

	// void *frame_address = allocate_frames(1);
	// printf("Frame address: %p\n", frame_address);
	// free_frames((uint64_t)frame_address, 1);
	// printf("Frame address: %p\n", allocate_frames(1));
	// printf("Frame address: %p\n", allocate_frames(1));

	// PAGING_init();

	// uint64_t heap_data[1028];

	// initialize_heap(&heap_data, 1028 * sizeof(uint64_t));

	printf("Done. Halting\n");
	hcf();
}
