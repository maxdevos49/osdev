#include <limine.h>
#include <stdint.h>

#include "debug.h"
#include "graphics.h"
#include "panic.h"
#include "string/utility.h"
#include "type.h"

__attribute__((
	used,
	section(".requests"))) static volatile struct limine_framebuffer_request
	framebuffer_request = {.id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0};

enum __graphics_drawing_mode { NONE, RECT, ELLIPSE, TEXT, LINE };

enum __graphics_active_buffer { FRAMEBUFFER, BUFFER0, BUFFER1 };

struct __graphics_context {
	int x_offset;
	int y_offset;
	int ctx_width;
	int ctx_height;
	uint32_t pitch;

	enum GRAPHICS_BUFFER_COUNT buffer_count;

	size_t buffer_size;
	void *buffer;
	void *buffer0;
	void *buffer1;

	enum __graphics_active_buffer current_back_buffer;

	// draw origin
	int origin_x;
	int origin_y;

	// Draw details
	int x;
	int y;
	int w;
	int h;

	// Line end
	int line_x;
	int line_y;
	int line_width;

	// Line color in different sizes for when needed
	uint64_t stroke_64;
	uint32_t stroke_32;

	// Fill color in different sizes for when needed
	uint64_t fill_64;
	uint32_t fill_32;

	enum __graphics_drawing_mode mode;
};

static inline uint64_t color32_to_color64(uint32_t clr)
{
	return (((uint64_t)clr) << 32) | (uint64_t)clr;
}

static int g_abs(int n)
{
	if (n >= 0)
		return n;

	return n * -1;
}

static struct GRAPHICS_FRAMEBUFFER _framebuffer;

err_code graphics_init(struct FONT *font)
{
	// Ensure we got a framebuffer.
	if (framebuffer_request.response == NULL ||
		framebuffer_request.response->framebuffer_count < 1) {
		debug_code(ERROR_UNEXPECTED_NULL_POINTER);
		return ERROR_UNEXPECTED_NULL_POINTER;
	}

	struct limine_framebuffer *framebuffer =
		framebuffer_request.response->framebuffers[0];

	_framebuffer.address = framebuffer->address;
	_framebuffer.height = framebuffer->height;
	_framebuffer.width = framebuffer->width;
	_framebuffer.pitch = framebuffer->pitch;
	_framebuffer.bpp = framebuffer->bpp;
	_framebuffer.red_mask_shift = framebuffer->red_mask_shift;
	_framebuffer.red_mask_size = framebuffer->red_mask_size;
	_framebuffer.green_mask_shift = framebuffer->green_mask_shift;
	_framebuffer.green_mask_size = framebuffer->green_mask_size;
	_framebuffer.blue_mask_shift = framebuffer->blue_mask_shift;
	_framebuffer.blue_mask_size = framebuffer->blue_mask_size;

	_framebuffer.font = font;

	_framebuffer.valid = 1;

	return 0;
}

GRAPHICS_CONTEXT *graphics_get_ctx(enum GRAPHICS_BUFFER_COUNT buffer_count,
								   int x, int y, int width, int height)
{
	if (_framebuffer.valid == 0) {
		return NULL;
	}

	GRAPHICS_CONTEXT *ctx = kmalloc(sizeof(GRAPHICS_CONTEXT));
	if (ctx == NULL) {
		return NULL;
	}

	ctx->buffer_count = buffer_count;

	ctx->x_offset = x;
	ctx->y_offset = y;
	ctx->ctx_width = width;
	ctx->ctx_height = height;
	ctx->pitch = width * (_framebuffer.bpp / 8);

	// Default buffer state
	ctx->buffer0 = NULL;
	ctx->buffer1 = NULL;
	ctx->current_back_buffer = FRAMEBUFFER;
	ctx->buffer = _framebuffer.address;
	ctx->buffer_size = width * height * (_framebuffer.bpp / 8);

	set_origin(ctx, 0, 0);
	rect(ctx, 0, 0, 0, 0);
	move_to(ctx, 0, 0);
	set_fill(ctx, 0x00000000);	 // black
	set_stroke(ctx, 0x00ffffff); // white
	set_line_width(ctx, 4);		 // 4 px

	//  Buffer count init
	if (buffer_count == DOUBLE) {
		ctx->buffer0 =
			kmalloc(ctx->ctx_width * ctx->ctx_height * (_framebuffer.bpp / 8));

		if (ctx->buffer0 == NULL) {
			return NULL;
		}

		ctx->buffer1 = NULL;
		ctx->current_back_buffer = BUFFER0;
		ctx->buffer = ctx->buffer0;
	} else if (buffer_count == TRIPLE) {
		ctx->buffer0 =
			kmalloc(ctx->ctx_width * ctx->ctx_height * (_framebuffer.bpp / 8));
		if (ctx->buffer0 == NULL) {
			return NULL;
		}

		ctx->buffer1 =
			kmalloc(ctx->ctx_width * ctx->ctx_height * (_framebuffer.bpp / 8));
		if (ctx->buffer0 == NULL) {
			return NULL;
		}

		ctx->current_back_buffer = BUFFER0;
		ctx->buffer = ctx->buffer0;
	} else if (buffer_count != SINGLE) {
		return NULL;
	}

	clear_rect(ctx, 0, 0, ctx->ctx_width, ctx->ctx_height);
	swap_buffer(ctx);

	return ctx;
}

int graphics_destroy_ctx(GRAPHICS_CONTEXT *ctx)
{
	if (ctx == NULL)
		return 1;

	if (ctx->buffer_count == DOUBLE) {
		kfree(ctx->buffer0);
	} else if (ctx->buffer_count == TRIPLE) {
		kfree(ctx->buffer0);
		kfree(ctx->buffer1);
	}

	kfree(ctx);

	return 0;
}

void swap_buffer(GRAPHICS_CONTEXT *ctx)
{
	// If single buffering there is no need to swap anything
	if (ctx->current_back_buffer == FRAMEBUFFER)
		return;

	uint32_t top_row = ctx->y_offset;
	uint32_t bottom_row = ctx->ctx_height + top_row;

	uint32_t row;

	uint32_t *f_offset = _framebuffer.address + (ctx->pitch * top_row);
	uint32_t *b_offset = ctx->buffer;

	for (row = top_row; row < bottom_row; row++) {

		asm volatile("cld\n"
					 "rep movsq" ::"S"(b_offset),
					 "D"(f_offset), "c"(ctx->ctx_width / 2));

		f_offset += _framebuffer.width;
		b_offset += ctx->ctx_width;
	}

	// If double buffering then there is no back buffer swap
	if (ctx->buffer_count == DOUBLE)
		return;

	// Swap target buffers
	if (ctx->current_back_buffer == BUFFER0) {
		ctx->buffer = ctx->buffer1;
		ctx->current_back_buffer = BUFFER1;
	} else if (ctx->current_back_buffer == BUFFER1) {
		ctx->buffer = ctx->buffer0;
		ctx->current_back_buffer = BUFFER0;
	}
}

void set_origin(GRAPHICS_CONTEXT *ctx, int x, int y)
{
	ctx->origin_x = x;
	ctx->origin_y = y;
}

void fill(GRAPHICS_CONTEXT *ctx)
{
	if (ctx->mode == RECT) {
		int left = ctx->x + ctx->origin_x;
		int right = left + ctx->w;
		int top = ctx->y + ctx->origin_y;
		int bottom = top + ctx->h;

		int width = ctx->ctx_width;
		int height = ctx->ctx_height;

		if (left < 0)
			left = 0;
		if (left > width)
			left = width;

		if (right < 0)
			right = 0;
		if (right > width)
			right = width;

		if (top < 0)
			top = 0;
		if (top > height)
			top = height;

		if (bottom < 0)
			bottom = 0;
		if (bottom > height)
			bottom = height;

		for (int i = top; i < bottom; i++) {
			uint32_t *offset =
				(uint32_t *)(i * ctx->pitch + (uint64_t)ctx->buffer);

			for (int j = left; j < right; j++)
				offset[j] = ctx->fill_32;
		}
	}
}

void set_fill(GRAPHICS_CONTEXT *ctx, uint32_t color)
{
	ctx->fill_64 = color32_to_color64(color);
	ctx->fill_32 = color;
}

static void render_line(GRAPHICS_CONTEXT *ctx, int x0, int y0, int x1, int y1)
{

	int dx = g_abs(x1 - x0);
	int sx = x0 < x1 ? 1 : -1;
	int dy = -g_abs(y1 - y0);
	int sy = y0 < y1 ? 1 : -1;
	int err = dx + dy; /* error value e_xy */

	while (1) {
		pixel(ctx, x0, y0, ctx->stroke_32);

		if (x0 == x1 && y0 == y1)
			break;

		int e2 = 2 * err;

		if (e2 >= dy) { /* e_xy+e_x > 0 */
			err += dy;
			x0 += sx;
		}
		if (e2 <= dx) { /* e_xy+e_y < 0 */
			err += dx;
			y0 += sy;
		}
	}
}

void stroke(GRAPHICS_CONTEXT *ctx)
{
	int left = ctx->x;
	int top = ctx->y;
	int right = left + ctx->w;
	int bottom = top + ctx->h;

	if (ctx->mode == RECT) {
		move_to(ctx, left, top);
		line_to(ctx, right, top);
		stroke(ctx);
		line_to(ctx, right, bottom);
		stroke(ctx);
		line_to(ctx, left, bottom);
		stroke(ctx);
		line_to(ctx, left, top);
		stroke(ctx);
		ctx->mode = RECT;
		move_to(ctx, left, top);
	} else if (ctx->mode == LINE) {
		int x0 = ctx->x;
		int y0 = ctx->y;

		int x1 = ctx->line_x;
		int y1 = ctx->line_y;
		for (int i = -ctx->line_width / 2; i < ctx->line_width / 2; i++) {
			render_line(ctx, x0 + i, y0 + i, x1 + i, y1 + i);
		}
	}
}

void set_stroke(GRAPHICS_CONTEXT *ctx, uint32_t color)
{
	ctx->stroke_64 = color32_to_color64(color);
	ctx->stroke_32 = color;
}

void set_line_width(GRAPHICS_CONTEXT *ctx, uint32_t thickness)
{
	ctx->line_width = thickness;
}

void move_to(GRAPHICS_CONTEXT *ctx, int x, int y)
{
	ctx->x = x;
	ctx->y = y;
	ctx->line_x = x;
	ctx->line_y = y;
	ctx->mode = NONE;
}

void line_to(GRAPHICS_CONTEXT *ctx, int x, int y)
{
	ctx->x = ctx->line_x;
	ctx->y = ctx->line_y;
	ctx->line_x = x;
	ctx->line_y = y;
	ctx->mode = LINE;
	stroke(ctx);
}

void rect(GRAPHICS_CONTEXT *ctx, int x, int y, int w, int h)
{
	ctx->x = x;
	ctx->y = y;
	ctx->w = w;
	ctx->h = h;
	ctx->mode = RECT;
}

void stroke_rect(GRAPHICS_CONTEXT *ctx, int x, int y, int w, int h)
{
	rect(ctx, x, y, w, h);
	stroke(ctx);
}

void fill_rect(GRAPHICS_CONTEXT *ctx, int x, int y, int w, int h)
{
	rect(ctx, x, y, w, h);
	fill(ctx);
}

void clear_rect(GRAPHICS_CONTEXT *ctx, int x, int y, int w, int h)
{
	uint32_t color = ctx->fill_32;
	set_fill(ctx, 0x00000000);
	fill_rect(ctx, x - ctx->origin_x, y - ctx->origin_y, w, h);
	set_fill(ctx, color);
}

void draw_char(GRAPHICS_CONTEXT *ctx, int x, int y, char c)
{
	_framebuffer.font->putc(ctx, x, y, c, ctx->stroke_32);
}

void draw_text(GRAPHICS_CONTEXT *ctx, int x, int y, char *txt)
{
	int sx = x + ctx->origin_x;
	int sy = y + ctx->origin_y;

	int i = 0;
	int offset = 0;
	while (txt[i] != '\0') {
		draw_char(ctx, sx + offset, sy, txt[i]);
		i++;
		offset += get_font_width();
	}
}

void pixel(GRAPHICS_CONTEXT *ctx, int x, int y, uint32_t color)
{
	int sx = x + ctx->origin_x;
	int sy = y + ctx->origin_y;

	if (sx < 0 || sx > ctx->ctx_width)
		return;

	if (sy < 0 || sy > ctx->ctx_height)
		return;

	*((uint32_t *)(sy * ctx->pitch + (sx * 4) + (uint64_t)ctx->buffer)) = color;
}

/**
 * Scrolls the graphics buffer by the given amount of pixels. Works best when
 * using double/triple buffering.
 */
void scroll(GRAPHICS_CONTEXT *ctx, uint32_t pixels)
{
	// TODO update for multibuffers...

	if (ctx->buffer_count == SINGLE &&
		get_ctx_width(ctx) != get_screen_width()) {
		panicf("Not yet implemented!");
	} else if (ctx->buffer_count == DOUBLE) {
		uint32_t size = (ctx->ctx_height - pixels) * ctx->pitch;
		void *src_ptr = ctx->buffer + (pixels * ctx->pitch);
		void *dst_ptr = ctx->buffer;

		// Note: Assumes 8 byte alignment and length is a multiple of 8
		// bytes.
		asm volatile("cld\n"
					 "rep movsq" ::"S"(src_ptr),
					 "D"(dst_ptr), "c"(size / 8));

		asm volatile("rep stos" ::"D"(ctx->buffer + size),
					 "c"(pixels * ctx->ctx_width), "a"((uint32_t)0));
	} else {
		panicf("Not yet implemented!");
	}
}

uint8_t get_font_width() { return _framebuffer.font->width; }

uint8_t get_font_height() { return _framebuffer.font->height; }

uint32_t get_screen_width() { return _framebuffer.width; }

uint32_t get_screen_height() { return _framebuffer.height; }

uint32_t get_ctx_width(GRAPHICS_CONTEXT *ctx) { return ctx->ctx_width; }

uint32_t get_ctx_height(GRAPHICS_CONTEXT *ctx) { return ctx->ctx_height; }

uint32_t get_ctx_pitch(GRAPHICS_CONTEXT *ctx) { return ctx->pitch; }
