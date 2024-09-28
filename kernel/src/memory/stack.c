#include "../string/utility.h"
#include "debug.h"
#include "dwarf.h"
#include "elf.h"
#include "error.h"
#include "limine.h"
#include "macro.h"
#include "type.h"
#include <stddef.h>

ATTR_REQUEST volatile struct limine_kernel_file_request kernel_file_request = {
	.id = LIMINE_KERNEL_FILE_REQUEST, .revision = 0};

struct STACK_FRAME {
	struct STACK_FRAME *rbp;
	uint64_t rip;
};

void strace(int max_frames, void *starting_rbp, void *starting_rip)
{
	err_code err = 0;
	struct STACK_FRAME *stack;

	if (starting_rbp == NULL) {
		asm volatile("mov %%rbp, %0" : "=r"(stack));
	} else {
		stack = (struct STACK_FRAME *)starting_rbp;
	}

	if (starting_rip == NULL) {
		starting_rip = strace;
	}

	// Attempt to load debug symbols.
	Elf64_Ehdr *elf_header = NULL;
	DW_Ctx *dwarf_ctx = NULL;
	do {
		if (kernel_file_request.response == NULL)
			break;

		void *kernel_file = kernel_file_request.response->kernel_file->address;

		if ((err = elf64_header(kernel_file, &elf_header))) {
			debug_code(err);
			break;
		}

		dwarf_ctx = dwarf_init_context(elf_header);
	} while (0);

	char *default_symbol = "UNKNOWN";
	char *symbol_string = NULL;

	printf("Call trace:\n");

	do {
		// Print function symbols if the dwarf symbols are present.
		if (dwarf_ctx == NULL)
			break;

		DW_Chdr *compilation_unit = NULL;
		if ((err = dwarf_cu_for_address(dwarf_ctx, (uintptr_t)starting_rip,
										&compilation_unit))) {
			debug_code(err);
			break;
		}

		if (compilation_unit == NULL) {
			break;
		}

		if ((err = dwarf_cu_query_func(dwarf_ctx, compilation_unit,
									   (uintptr_t)starting_rip,
									   &symbol_string))) {
			debug_code(err);
			break;
		}
	} while (0);

	if (symbol_string == NULL) {
		symbol_string = default_symbol;
	}

	printf("\t[%#018lx] %s\n", (uintptr_t)starting_rip, symbol_string);

	for (int i = 0; stack != NULL && stack->rip != 0 && i < max_frames; i++) {

		do {
			// Print function symbols if the dwarf symbols are present.
			if (dwarf_ctx == NULL)
				break;

			DW_Chdr *compilation_unit = NULL;
			if ((err = dwarf_cu_for_address(dwarf_ctx, stack->rip,
											&compilation_unit))) {
				debug_code(err);
				break;
			}

			if (compilation_unit == NULL) {
				break;
			}

			if ((err = dwarf_cu_query_func(dwarf_ctx, compilation_unit,
										   stack->rip, &symbol_string))) {
				debug_code(err);
				break;
			}
		} while (0);

		if (symbol_string == NULL) {
			symbol_string = default_symbol;
		}

		printf("\t[%#018lx] %s\n", stack->rip, symbol_string);

		stack = stack->rbp;
	}
}
