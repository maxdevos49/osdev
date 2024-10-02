#ifndef __DEVICES_TTY_H
#define __DEVICES_TTY_H 1

#include <stdbool.h>

void TTY_init(void);
bool TTY_ready(void);
void TTY_putc(char c);
void TTY_puts(const char *str);

#endif
