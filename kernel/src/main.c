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
#include "instruction.h"
#include "interrupts/idt.h"
#include "macro.h"
#include "memory/heap.h"
#include "memory/memory.h"
#include "memory/stack.h"
#include "panic.h"
#include "serial.h"
#include "string/utility.h"
#include "type.h"

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

	printf("\n\n");

	enable_sse2();

	init_memory();

	struct FONT font;
	PSF2_load_font(&font);

	if (graphics_init(&font)) {
		panic();
	}

	GRAPHICS_CONTEXT *ctx =
		graphics_get_ctx(DOUBLE, 0, 0, get_screen_width(), get_screen_height());

	TTY_init(ctx, get_ctx_height(ctx) / get_font_height(),
			 get_ctx_width(ctx) / get_font_width());

	printf(KINFO "========== m4xdevOS ========== \n");

	init_gdt();
	init_idt();

	while (1) {
		print_memory_layout();
	}

	printf(KINFO "Done. Halting\n");
	halt();
}
