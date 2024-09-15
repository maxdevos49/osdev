#ifndef __IDT_H
#define __IDT_H 1

#include <stdint.h>

void init_idt(void);
void set_interrupt_gate(uint16_t index, uintptr_t target, uint16_t target_selector, uint16_t flags);

#endif
