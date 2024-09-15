#include <limine.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "color.h"
#include "cpuid.h"
#include "devices/tty.h"
#include "error.h"
#include "fonts/font.h"
#include "graphics/graphics.h"
#include "macro.h"
#include "memory/memory.h"
#include "serial.h"
#include "gdt.h"
#include "interrupts/idt.h"
#include "string/utility.h"

// TODO list
// Setup own stack
// Then claim all bootloader reclaimable memory

ATTR_REQUEST static volatile LIMINE_BASE_REVISION(2);

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
	printf("[ABORT] %s\n", error);

	hcf();
}

void kmain(void)
{
	init_serial();

	// Ensure the bootloader actually understands our base revision (see spec).
	if (LIMINE_BASE_REVISION_SUPPORTED == false)
	{
		hcf();
	}

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

	printf(KINFO "========== m4xdevOS ========== \n");

	init_memory();
	init_gdt();
	init_idt();

	printf(KINFO "Done. Halting\n");
	hcf();
}
