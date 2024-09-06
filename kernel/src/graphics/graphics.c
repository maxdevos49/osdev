#include <stdint.h>
#include <limine.h>

#include "../string/utility.h"
#include "graphics.h"

enum __graphics_drawing_mode
{
	NONE,
	RECT,
	ELLIPSE,
	TEXT,
	LINE
};

struct __graphics_context
{
	int x_offset;
	int y_offset;
	int ctx_width;
	int ctx_height;
	uint32_t pitch;

	enum GRAPHICS_BUFFER_COUNT buffer_count;

	size_t buffer_size;
	void *buffer;
	// void *buffer0;
	// void *buffer1;

	// enum __graphics_active_buffer current_back_buffer;

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

static int g_abs(int n);

__attribute__((used, section(".requests"))) static volatile struct limine_framebuffer_request framebuffer_request = {
	.id = LIMINE_FRAMEBUFFER_REQUEST,
	.revision = 0};

static struct GRAPHICS_FRAMEBUFFER _framebuffer;

int GRAPHICS_init(struct FONT *font)
{
	// Ensure we got a framebuffer.
	if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1)
	{
		// TODO serial print
		// hcf();
	}

	struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

	_framebuffer.address = framebuffer->address;
	if (_framebuffer.address == NULL)
		return 1;

	_framebuffer.bpp = framebuffer->bpp;
	if (_framebuffer.bpp != 32)
		return 2;

	_framebuffer.pitch = framebuffer->pitch;
	if (_framebuffer.pitch <= 0)
		return 3;

	_framebuffer.height = framebuffer->height;
	if (_framebuffer.height <= 0)
		return 4;

	_framebuffer.width = framebuffer->width;
	if (_framebuffer.width <= 0)
		return 5;

	if ((_framebuffer.font = font) == NULL)
		return 6;

	_framebuffer.valid = 1;

	return 0;
}

int GRAPHICS_uninit()
{
	if (_framebuffer.valid == 0)
		return 1;

	// TODO keep track of active contexts for freeing buffers?

	_framebuffer.valid = 0;
	_framebuffer.address = NULL;
	_framebuffer.font = NULL;
	_framebuffer.bpp = 0;
	_framebuffer.height = 0;
	_framebuffer.width = 0;
	_framebuffer.pitch = 0;

	return 0;
}

static struct __graphics_context _ctx; // TODO Temp

GRAPHICS_CONTEXT *GRAPHICS_get_ctx(enum GRAPHICS_BUFFER_COUNT buffer_count, int x, int y, int width, int height)
{
	// printf("Getting Graphics context\n");
	GRAPHICS_CONTEXT *ctx = &_ctx;

	if (ctx == NULL)
	{
		// print_error(ctx, "CTX malloc error", 1);//TODO
		return NULL;
	}

	if (_framebuffer.valid == 0)
	{
		// print_error(ctx, "Invalid Buffer", 1);//TODO
		return NULL;
	}

	ctx->buffer_count = buffer_count;

	ctx->x_offset = x;
	ctx->y_offset = y;
	ctx->ctx_width = width;
	ctx->ctx_height = height;
	ctx->pitch = width * sizeof(uint32_t);

	// Default buffer state
	// ctx->buffer0 = NULL;
	// ctx->buffer1 = NULL;
	// ctx->current_back__framebuffer = FRAMEBUFFER;
	ctx->buffer = _framebuffer.address;
	ctx->buffer_size = width * height * sizeof(uint32_t);

	set_origin(ctx, 0, 0);
	rect(ctx, 0, 0, 0, 0);
	move_to(ctx, 0, 0);
	set_fill(ctx, 0x00000000);	 // black
	set_stroke(ctx, 0x00ffffff); // white
	set_line_width(ctx, 4);		 // 4 px

	// TODO restore double/triple buffering once malloc is implemented
	//  Buffer count init
	//  if (buffer_count == DOUBLE)
	//  {
	//      ctx->buffer0 = malloc(ctx->ctx_width * ctx->ctx_height * sizeof(uint32_t));

	//     if (ctx->buffer0 == NULL)
	//     {
	//         print_error(ctx, "Malloc Error: buffer0 is null.", 4);
	//         return NULL;
	//     }

	//     ctx->buffer1 = NULL;
	//     ctx->current_back_buffer = BUFFER0;
	//     ctx->buffer = ctx->buffer0;
	// }
	// else if (buffer_count == TRIPLE)
	// {
	//     ctx->buffer0 = malloc(ctx->ctx_width * ctx->ctx_height * sizeof(uint32_t));
	//     if (ctx->buffer0 == NULL)
	//     {
	//         print_error(ctx, "Malloc Error: buffer0 is null.", 4);
	//         return NULL;
	//     }

	//     ctx->buffer1 = malloc(ctx->ctx_width * ctx->ctx_height * sizeof(uint32_t));
	//     if (ctx->buffer0 == NULL)
	//     {
	//         print_error(ctx, "Malloc Error: buffer1 is null.", 4);
	//         return NULL;
	//     }

	//     ctx->current_back_buffer = BUFFER0;
	//     ctx->buffer = ctx->buffer0;
	// }
	// else if (buffer_count != SINGLE)
	// {
	//     print_error(ctx, "Invalid buffer Count", 2);
	//     return NULL;
	// }

	clear_rect(ctx, 0, 0, ctx->ctx_width, ctx->ctx_height);
	// swap_buffer(ctx);//TODO

	return ctx;
}

int GRAPHICS_destroy_ctx(GRAPHICS_CONTEXT *ctx)
{
	if (ctx == NULL)
		return 1;

	// if (ctx->buffer_count == DOUBLE)
	// {
	//     free(ctx->buffer0);
	// }
	// else if (ctx->buffer_count == TRIPLE)
	// {
	//     free(ctx->buffer0);
	//     free(ctx->buffer1);
	// }

	// free(ctx);

	return 0;
}

// int print_error(GRAPHICS_CONTEXT *ctx, char *str, int error_code)
// {
//     // ctx->buffer = _buffer.framebuffer;
//     // ctx->current_back_buffer = FRAMEBUFFER;

//     // set_stroke(ctx, 0x00ffffff);
//     // draw_text(ctx, _buffer.width / 2 - (strlen(str) * CHAR_WIDTH) / 2, _buffer.height / 2 - CHAR_HEIGHT / 2, str);
//     // printf("%s\n", str);

//     return error_code;
// }

// void swap_buffer(GRAPHICS_CONTEXT *ctx)
// {
//     // If single buffering there is no need to swap anything
//     if (ctx->current_back_buffer == FRAMEBUFFER)
//         return;

//     int left = ctx->x_offset;
//     int right = ctx->ctx_width + left;
//     int top = ctx->y_offset;
//     int bottom = ctx->ctx_height + top;

//     int fi, bi, fj, bj;
//     for (fi = top, bi = 0; fi < bottom; fi++, bi++)
//     {
//         uint32_t *f_offset = (uint32_t *)(_buffer.pitch * fi + (uint64_t)_buffer.framebuffer);
//         uint32_t *b_offset = (uint32_t *)(ctx->pitch * bi + (uint64_t)ctx->buffer);

//         for (fj = left, bj = 0; fj < right; fj++, bj++)
//             f_offset[fj] = b_offset[bj];
//     }

//     // If double buffering then there is no back buffer swap
//     if (ctx->buffer_count == DOUBLE)
//         return;

//     // Swap target buffers
//     if (ctx->current_back_buffer == BUFFER0)
//     {
//         ctx->buffer = ctx->buffer1;
//         ctx->current_back_buffer = BUFFER1;
//     }
//     else if (ctx->current_back_buffer == BUFFER1)
//     {
//         ctx->buffer = ctx->buffer0;
//         ctx->current_back_buffer = BUFFER0;
//     }
// }

void set_origin(GRAPHICS_CONTEXT *ctx, int x, int y)
{
	ctx->origin_x = x;
	ctx->origin_y = y;
}

void fill(GRAPHICS_CONTEXT *ctx)
{
	if (ctx->mode == RECT)
	{
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

		for (int i = top; i < bottom; i++)
		{
			uint32_t *offset = (uint32_t *)(i * ctx->pitch + (uint64_t)ctx->buffer);

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

	while (1)
	{
		pixel(ctx, x0, y0, ctx->stroke_32);

		if (x0 == x1 && y0 == y1)
			break;

		int e2 = 2 * err;

		if (e2 >= dy)
		{ /* e_xy+e_x > 0 */
			err += dy;
			x0 += sx;
		}
		if (e2 <= dx)
		{ /* e_xy+e_y < 0 */
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

	if (ctx->mode == RECT)
	{
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
	}
	else if (ctx->mode == LINE)
	{
		int x0 = ctx->x;
		int y0 = ctx->y;

		int x1 = ctx->line_x;
		int y1 = ctx->line_y;
		for (int i = -ctx->line_width / 2; i < ctx->line_width / 2; i++)
		{
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
	while (txt[i] != '\0')
	{
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
	if (ctx->buffer_count == SINGLE && get_ctx_width(ctx) != get_screen_width())
	{
		// TODO Slow line by line copy when ctx is not full screen and is only single buffered
	}
	else
	{
		uint32_t length = (get_ctx_height(ctx) - pixels) * get_ctx_pitch(ctx);
		void *dst_ptr = ctx->buffer;
		void *src_ptr = dst_ptr + (pixels * get_ctx_pitch(ctx));

		memmove(dst_ptr, src_ptr, length);

		// Clear the pixels on the next row
		memset(ctx->buffer + length, 0, pixels * get_ctx_pitch(ctx));
	}
}

uint8_t get_font_width()
{
	return _framebuffer.font->width;
}

uint8_t get_font_height()
{
	return _framebuffer.font->height;
}

uint32_t get_screen_width()
{
	return _framebuffer.width;
}

uint32_t get_screen_height()
{
	return _framebuffer.height;
}

uint32_t get_ctx_width(GRAPHICS_CONTEXT *ctx)
{
	return ctx->ctx_width;
}

uint32_t get_ctx_height(GRAPHICS_CONTEXT *ctx)
{
	return ctx->ctx_height;
}

uint32_t get_ctx_pitch(GRAPHICS_CONTEXT *ctx)
{
	return ctx->pitch;
}

GRAPHICS_CONTEXT *GRAPHICS_get_global_context()
{
	return &_ctx;
}

static int g_abs(int n)
{
	if (n >= 0)
		return n;

	return n * -1;
}

// enum __graphics_active_buffer
// {
//     FRAMEBUFFER,
//     BUFFER0,
//     BUFFER1
// };