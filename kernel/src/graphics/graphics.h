/**
 * Graphics Library for madOS
 * Designed for graphics modes with 32 bpp.
 * Supports both Double and Triple Buffering.
 * */
#ifndef __LIB_GRAPHICS_H
#define __LIB_GRAPHICS_H 1

#include "memory/heap.h"
#include "type.h"
#include <stddef.h>
#include <stdint.h>

enum GRAPHICS_BUFFER_COUNT { SINGLE, DOUBLE, TRIPLE };

struct GRAPHICS_FRAMEBUFFER {
	void *address;
	uint64_t width;
	uint64_t height;
	uint64_t pitch;
	uint16_t bpp;
	uint8_t red_mask_shift;
	uint8_t red_mask_size;
	uint8_t green_mask_shift;
	uint8_t green_mask_size;
	uint8_t blue_mask_shift;
	uint8_t blue_mask_size;

	struct FONT *font;

	int valid;
};

typedef struct __graphics_context GRAPHICS_CONTEXT;

struct FONT {
	uint8_t width;
	uint8_t height;
	void (*putc)(GRAPHICS_CONTEXT *, int, int, char, uint32_t);
};

err_code graphics_init(struct FONT *font);

GRAPHICS_CONTEXT *graphics_get_ctx(enum GRAPHICS_BUFFER_COUNT buffer_count,
								   int x, int y, int width, int height);
int graphics_destroy_ctx(GRAPHICS_CONTEXT *ctx);

void swap_buffer(GRAPHICS_CONTEXT *ctx);
void pixel(GRAPHICS_CONTEXT *ctx, int x, int y, uint32_t color);
void draw_char(GRAPHICS_CONTEXT *ctx, int x, int y, char c);
void scroll(GRAPHICS_CONTEXT *ctx, uint32_t pixels);

void set_origin(GRAPHICS_CONTEXT *ctx, int x, int y);
void fill(GRAPHICS_CONTEXT *ctx);
void set_fill(GRAPHICS_CONTEXT *ctx, uint32_t color);
void stroke(GRAPHICS_CONTEXT *ctx);
void set_stroke(GRAPHICS_CONTEXT *ctx, uint32_t color);
void set_line_width(GRAPHICS_CONTEXT *ctx, uint32_t thickness);
void draw_text(GRAPHICS_CONTEXT *ctx, int x, int y, char *txt);
void move_to(GRAPHICS_CONTEXT *ctx, int x, int y);
void line_to(GRAPHICS_CONTEXT *ctx, int x, int y);
void rect(GRAPHICS_CONTEXT *ctx, int x, int y, int w, int h);
void stroke_rect(GRAPHICS_CONTEXT *ctx, int x, int y, int w, int h);
void fill_rect(GRAPHICS_CONTEXT *ctx, int x, int y, int w, int h);
void clear_rect(GRAPHICS_CONTEXT *ctx, int x, int y, int w, int h);

uint8_t get_font_width();
uint8_t get_font_height();

uint32_t get_screen_width();
uint32_t get_screen_height();

uint32_t get_ctx_width(GRAPHICS_CONTEXT *ctx);
uint32_t get_ctx_height(GRAPHICS_CONTEXT *ctx);
uint32_t get_ctx_pitch(GRAPHICS_CONTEXT *ctx);

#endif
