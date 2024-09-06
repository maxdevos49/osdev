/**
 * Graphics Library for madOS
 * Designed for graphics modes with 32 bpp.
 * Supports both Double and Triple Buffering.
 * */
#ifndef __LIB_GRAPHICS_H
#define __LIB_GRAPHICS_H 1

#include <stdint.h>
#include <stddef.h>

enum GRAPHICS_BUFFER_COUNT
{
    SINGLE,
    // DOUBLE,
    // TRIPLE
};

struct GRAPHICS_FRAMEBUFFER
{
    void *address;
    struct FONT *font;
    int width;
    int height;
    int pitch;
    int valid;
    uint16_t bpp;
};

typedef struct __graphics_context GRAPHICS_CONTEXT;

struct FONT
{
    uint8_t width;
    uint8_t height;
    void (*putc)(GRAPHICS_CONTEXT *, int, int, char, uint32_t);
};

int GRAPHICS_init(struct FONT *font);
int GRAPHICS_uninit();

GRAPHICS_CONTEXT *GRAPHICS_get_ctx(enum GRAPHICS_BUFFER_COUNT buffer_count, int x, int y, int width, int height);
int GRAPHICS_destroy_ctx(GRAPHICS_CONTEXT *ctx);

// int print_error(GRAPHICS_CONTEXT *ctx, char *str, int error_code);
// void swap_buffer(GRAPHICS_CONTEXT *ctx);
void set_origin(GRAPHICS_CONTEXT *ctx, int x, int y);
void fill(GRAPHICS_CONTEXT *ctx);
void set_fill(GRAPHICS_CONTEXT *ctx, uint32_t color);
void stroke(GRAPHICS_CONTEXT *ctx);
void set_stroke(GRAPHICS_CONTEXT *ctx, uint32_t color);
void set_line_width(GRAPHICS_CONTEXT *ctx, uint32_t thickness);
void pixel(GRAPHICS_CONTEXT *ctx, int x, int y, uint32_t color);
void draw_char(GRAPHICS_CONTEXT *ctx, int x, int y, char c);
void draw_text(GRAPHICS_CONTEXT *ctx, int x, int y, char *txt);
void move_to(GRAPHICS_CONTEXT *ctx, int x, int y);
void line_to(GRAPHICS_CONTEXT *ctx, int x, int y);
void rect(GRAPHICS_CONTEXT *ctx, int x, int y, int w, int h);
void stroke_rect(GRAPHICS_CONTEXT *ctx, int x, int y, int w, int h);
void fill_rect(GRAPHICS_CONTEXT *ctx, int x, int y, int w, int h);
void clear_rect(GRAPHICS_CONTEXT *ctx, int x, int y, int w, int h);
void scroll(GRAPHICS_CONTEXT *ctx, uint32_t pixels);

uint8_t get_font_width();
uint8_t get_font_height();

uint32_t get_screen_width();
uint32_t get_screen_height();

uint32_t get_ctx_width(GRAPHICS_CONTEXT *ctx);
uint32_t get_ctx_height(GRAPHICS_CONTEXT *ctx);
uint32_t get_ctx_pitch(GRAPHICS_CONTEXT *ctx);

GRAPHICS_CONTEXT *GRAPHICS_get_global_context(); // TODO temp

// TODO Add Ellipse drawing

static inline uint64_t color32_to_color64(uint32_t clr)
{
    return (((uint64_t)clr) << 32) | (uint64_t)clr;
}

#endif