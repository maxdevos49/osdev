#ifndef __GDT_H
#define __GDT_H 1

#include <stdint.h>

enum SEGMENT_PRIVILEGE
{
	PRIVILEGE_LVL_0 = 0,
	PRIVILEGE_LVL_1 = 1,
	PRIVILEGE_LVL_2 = 2,
	PRIVILEGE_LVL_3 = 3,
};

void init_gdt(void);
void write_code_descriptor(uint16_t index, enum SEGMENT_PRIVILEGE privilege, bool conforming);
void write_data_descriptor(uint16_t index, enum SEGMENT_PRIVILEGE privilege);

#endif
