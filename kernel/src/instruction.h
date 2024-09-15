#ifndef __INSTRUCTION_H
#define __INSTRUCTION_H

#include <stdint.h>

// Get the contents of the CR3 Register
static inline uint64_t read_CR3(void)
{
	uint64_t val;
	asm volatile("mov %%cr3, %0" : "=r"(val));
	return val;
}

// Set the contents of the CR3 Register.
static inline void write_CR3(uint64_t value)
{
	asm volatile("movq %[cr3_val], %%cr3" ::[cr3_val] "r"(value));
}

// Invalidates the given page entry.
static inline void flush_tlb(uintptr_t virtual_address)
{
	asm volatile("invlpg (%0)" ::"r"(virtual_address) : "memory");
}

static inline void enable_interrupts(void)
{
	asm volatile("sti");
}

static inline void disable_interrupts(void)
{
	asm volatile("cli");
}

#endif
