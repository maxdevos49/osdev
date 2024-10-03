#include <limine.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "color.h"
#include "cpuid.h"
#include "debug.h"
#include "devices/tty.h"
#include "dwarf.h"
#include "elf.h"
#include "fonts/font.h"
#include "gdt.h"
#include "graphics/graphics.h"
#include "interrupts/idt.h"
#include "macro.h"
#include "memory/memory.h"
#include "memory/stack.h"
#include "panic.h"
#include "serial.h"
#include "string/utility.h"

ATTR_REQUEST static volatile LIMINE_BASE_REVISION(2);

ATTR_REQUEST volatile struct limine_kernel_file_request kernel_file_request = {
	.id = LIMINE_KERNEL_FILE_REQUEST, .revision = 0};

void kmain(void)
{
	err_code err = 0;

	// Ensure the bootloader actually understands our base revision (see spec).
	if (LIMINE_BASE_REVISION_SUPPORTED == false) {
		halt();
	}

	init_serial();

	// Attempt to load debug symbols.
	do {
		if (kernel_file_request.response == NULL)
			break;

		Elf64_Ehdr *elf_header = NULL;
		if ((err = elf64_header(
				 kernel_file_request.response->kernel_file->address,
				 &elf_header))) {
			debug_code(err);
			break;
		}

		dwarf_load_sections(elf_header);
	} while (0);

	struct FONT font;
	PSF2_load_font(&font);
	GRAPHICS_init(&font);

	GRAPHICS_CONTEXT *ctx =
		GRAPHICS_get_ctx(SINGLE, 0, 0, get_screen_width(), get_screen_height());

	uint32_t color = 0;
	uint32_t color_mask = 0x0f000000;
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < 16; j++) {
			color = (color_mask >> (j * 2));
			set_fill(ctx, color);
			fill_rect(ctx,
					  ((get_screen_width() / 32) * j) + get_screen_width() / 2 +
						  150,
					  (get_screen_height() / 10) * i, 50, 200);
		}
	}

	TTY_init();

	printf(KINFO "========== m4xdevOS ========== \n");

	init_memory();
	init_gdt();
	init_idt();

	strace(10, NULL, NULL);

	printf(KINFO "Done. Halting\n");
	halt();
}
