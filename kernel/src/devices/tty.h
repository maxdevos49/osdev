#ifndef __DEVICES_TTY_H
#define __DEVICES_TTY_H 1

#include "graphics/graphics.h"
#include <stdbool.h>

void TTY_init(GRAPHICS_CONTEXT *graphics_ctx, int rows, int cols);
bool TTY_ready(void);
void TTY_putc(char c);
void TTY_puts(const char *str);

#endif
