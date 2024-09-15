#include <stddef.h>
#include "../string/utility.h"

struct STACK_FRAME
{
	struct STACK_FRAME *rbp;
	uint64_t rip;
};

void strace(int max_frames)
{
	struct STACK_FRAME *stack;

	asm volatile("mov %%rbp, %0" : "=r"(stack));

	printf("Call trace:\n");
	// printf("\t[%p] %s (%s at line %s)\n", &strace, __FUNCTION__, __FILE__, __LINE__);

	for (int i = 0; stack != NULL && stack->rip != 0 && i < max_frames; i++)
	{
		printf("\t[%p]\n", stack->rip);
		stack = stack->rbp;
	}
}
