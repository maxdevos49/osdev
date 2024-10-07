#ifndef __INSTRUCTION_H
#define __INSTRUCTION_H

#include <stdint.h>

// Get the contents of the CR0 Register
static inline uint64_t read_CR0(void)
{
	uint64_t val;
	asm volatile("mov %%cr0, %0" : "=r"(val));
	return val;
}

// Set the contents of the CR0 Register.
static inline void write_CR0(uint64_t value)
{
	asm volatile("movq %[cr0_val], %%cr0" ::[cr0_val] "r"(value));
}

// Get the contents of the CR2 Register
static inline uint64_t read_CR2(void)
{
	uint64_t val;
	asm volatile("mov %%cr2, %0" : "=r"(val));
	return val;
}

// Set the contents of the CR2 Register.
static inline void write_CR2(uint64_t value)
{
	asm volatile("movq %[cr2_val], %%cr2" ::[cr2_val] "r"(value));
}

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

// Get the contents of the CR4 Register
static inline uint64_t read_CR4(void)
{
	uint64_t val;
	asm volatile("mov %%cr4, %0" : "=r"(val));
	return val;
}

// Set the contents of the CR4 Register.
static inline void write_CR4(uint64_t value)
{
	asm volatile("movq %[cr4_val], %%cr4" ::[cr4_val] "r"(value));
}

// Invalidates the given page entry.
static inline void flush_tlb(void *virtual_address)
{
	asm volatile("invlpg (%0)" ::"r"((uintptr_t)virtual_address) : "memory");
}

static inline void enable_interrupts(void) { asm volatile("sti"); }

static inline void disable_interrupts(void) { asm volatile("cli"); }

static inline void enable_sse2()
{
	uint64_t cr0 = read_CR0();
	cr0 &= ~(1 << 2); // Clear CR0.EM bit
	cr0 |= (1 << 1);  // Set CR0.MP bit
	write_CR0(cr0);

	uint64_t cr4 = read_CR4();
	cr4 |= (1 << 9);  // Set CR4.OSFXSR bit
	cr4 |= (1 << 10); // Set CR4.OSXMMEXCPT bit
	write_CR4(cr4);
}

#endif
