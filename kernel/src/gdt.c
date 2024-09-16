#include <stdint.h>
#include <stdbool.h>
#include "gdt.h"
#include "./string/utility.h"
#include "macro.h"

extern void load_gdt(uint16_t limit, uint64_t base);
extern void reload_segments(uint16_t code_segment, uint16_t data_segment);

struct CODE_SEGMENT_DESCRIPTOR
{
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_mid;
	uint8_t
		accessed : 1,
		readable : 1,
		conforming : 1,
		code : 1,
		segment : 1,
		privilege : 2,
		present : 1;
	uint8_t
		limit_high : 4,
		available_to_software : 1,
		long_mode : 1,
		default_operand_size : 1,
		granularity : 1;
	uint8_t base_address_high;
} ATTR_PACK;

_Static_assert(sizeof(struct CODE_SEGMENT_DESCRIPTOR) == sizeof(uint64_t));

struct DATA_SEGMENT_DESCRIPTOR
{
	uint16_t limit_low;
	uint16_t base_low;
	uint8_t base_mid;
	uint8_t
		accessed : 1,
		writable : 1,
		expand_down : 1,
		code : 1,
		segment : 1,
		privilege : 2,
		present : 1;
	uint8_t
		limit_high : 4,
		available_to_software : 1,
		long_mode : 1,
		default_operand_size : 1,
		granularity : 1;
	uint8_t base_address_high;
} ATTR_PACK;

_Static_assert(sizeof(struct DATA_SEGMENT_DESCRIPTOR) == sizeof(uint64_t));

union SEGMENT_DESCRIPTOR
{
	uint64_t raw;
	struct CODE_SEGMENT_DESCRIPTOR code_descriptor;
	struct DATA_SEGMENT_DESCRIPTOR data_descriptor;
};

_Static_assert(sizeof(union SEGMENT_DESCRIPTOR) == sizeof(uint64_t));

static union SEGMENT_DESCRIPTOR _gdt[8] = {0};

static inline uint16_t index_to_offset(uint16_t index)
{
	return index * sizeof(uint64_t);
}

void init_gdt(void)
{
	printf(KINFO "Setting up GDT entries...\n");
	_gdt[0].raw = 0; // Null Descriptor

	printf("\tNull Descriptor: %#018lx\n", _gdt[0].raw);

	write_code_descriptor(1, PRIVILEGE_LVL_0, true); // Kernel code descriptor
	write_data_descriptor(2, PRIVILEGE_LVL_0);		 // Kernel data descriptor
	write_code_descriptor(3, PRIVILEGE_LVL_3, true); // Userspace code descriptor
	write_data_descriptor(4, PRIVILEGE_LVL_3);		 // Userspace data descriptor

	printf("\tKernel code segment: %#018lx\n", _gdt[1].raw);
	printf("\tKernel data segment: %#018lx\n", _gdt[2].raw);
	printf("\tUser code segment: %#018lx\n", _gdt[3].raw);
	printf("\tUser data segment: %#018lx\n", _gdt[4].raw);

	printf(KINFO "Loading GDT...\n");
	printf("\tNew GDTR limit: %'lu bytes\n", sizeof(_gdt));
	printf("\tNew GDTR base: %p\n", &_gdt);

	load_gdt(sizeof(_gdt), (uintptr_t)&_gdt);
	reload_segments(index_to_offset(1), index_to_offset(2));

	printf(KOK "GDT loaded\n");
}

void write_code_descriptor(uint16_t index, enum SEGMENT_PRIVILEGE privilege, bool conforming)
{
	struct CODE_SEGMENT_DESCRIPTOR *segment = &_gdt[index].code_descriptor;

	segment->limit_low = 0xffff;
	segment->base_low = 0;
	segment->base_mid = 0;

	// Access
	segment->conforming = conforming;
	segment->readable = true;
	segment->code = true;
	segment->segment = true;
	segment->privilege = privilege;
	segment->present = true;

	segment->limit_high = 0xf;

	// Flags
	segment->available_to_software = false;
	segment->long_mode = true;
	segment->default_operand_size = false;
	segment->granularity = true;

	segment->base_address_high = 0;
}

void write_data_descriptor(uint16_t index, enum SEGMENT_PRIVILEGE privilege)
{
	struct DATA_SEGMENT_DESCRIPTOR *segment = &_gdt[index].data_descriptor;

	segment->limit_low = 0xffff;
	segment->base_low = 0;
	segment->base_mid = 0;

	// Access
	segment->accessed = false;
	segment->writable = true;
	segment->expand_down = false;
	segment->code = false;
	segment->segment = true;
	segment->privilege = privilege;
	segment->present = true;

	segment->limit_high = 0xf;

	// Flags
	segment->available_to_software = false;
	segment->long_mode = true;
	segment->default_operand_size = false;
	segment->granularity = true;

	segment->base_address_high = 0;
}
