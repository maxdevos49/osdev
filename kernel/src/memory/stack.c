#include "../string/utility.h"
#include "debug.h"
#include "dwarf.h"
#include "elf.h"
#include "error.h"
#include "limine.h"
#include "macro.h"
#include "type.h"
#include <stddef.h>

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

	struct LINE_INFO line = {0};
	char *symbol_string = NULL;

	printf("Call trace:\n");

	do {
		if ((err = dwarf_query_line((uintptr_t)starting_rip, &line))) {
			debug_code(err);
			break;
		}

		if ((err = dwarf_query_func((uintptr_t)starting_rip, &symbol_string))) {
			debug_code(err);
			break;
		}
	} while (0);

	if (symbol_string == NULL) {
		printf("\t[%#018lx]\n", (uintptr_t)starting_rip);
	} else {
		printf("\t[%#018lx] at %s (%s/%s:%ld:%d)\n", (uintptr_t)starting_rip,
			   symbol_string, line.path, line.file, line.line, line.column);
	}

	for (int i = 0; stack != NULL && stack->rip != 0 && i < max_frames; i++) {

		do {
			if ((err = dwarf_query_line(stack->rip, &line))) {
				debug_code(err);
				break;
			}

			if ((err = dwarf_query_func(stack->rip, &symbol_string))) {
				debug_code(err);
				break;
			}
		} while (0);

		if (symbol_string == NULL) {
			printf("\t[%#018lx]\n", stack->rip);
		} else {
			printf("\t[%#018lx] at %s (%s/%s:%ld:%d)\n", (uintptr_t)stack->rip,
				   symbol_string, line.path, line.file, line.line, line.column);
		}

		stack = stack->rbp;
	}
}
