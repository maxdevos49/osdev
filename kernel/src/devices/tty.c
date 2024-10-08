#include <stdbool.h>
#include <stdint.h>

#include "../color.h"
#include "../graphics/graphics.h"
#include "../string/utility.h"

struct __tty_context {
	int32_t cursor_x;
	int32_t cursor_y;
	int32_t col_size;
	int32_t row_size;

	GRAPHICS_CONTEXT *graphics_ctx;
};

static struct __tty_context _tty;

void TTY_init(GRAPHICS_CONTEXT *graphics_ctx, int rows, int cols)
{
	// Start cursor at top left
	_tty.cursor_x = 0;
	_tty.cursor_y = 0;

	// Get the number of character rows and columns which can fit inside the
	// graphics ctx bounds.
	_tty.row_size = rows;
	_tty.col_size = cols;

	_tty.graphics_ctx = graphics_ctx;
}

bool TTY_ready(void) { return _tty.row_size != 0; }

void TTY_putc(char c)
{
	switch (c) {
	case '\t': // Horizontal Tab
		_tty.cursor_x += 4;
		break;
	case '\b': // Backspace
		// Only apply backspace if we wont back up above the top of the screen.
		if (_tty.cursor_x > 0 || _tty.cursor_y > 0) {
			_tty.cursor_x--;
		}
		break;
	case '\n': // Newline
		_tty.cursor_x = 0;
		_tty.cursor_y++;
		break;
	case '\r': // Return
		_tty.cursor_y++;
		break;
	default:
		draw_char(_tty.graphics_ctx, _tty.cursor_x * get_font_width(),
				  _tty.cursor_y * get_font_height(), c);

		_tty.cursor_x++;

		break;
	}

	// Wrap cursor to previous line if we have moved to the left side of the
	// screen.
	if (_tty.cursor_x < 0) {
		_tty.cursor_x = _tty.col_size - 1;
		_tty.cursor_y--;
	}

	// Wrap cursor to next line if we have moved to the right side of the
	// screen.
	if (_tty.cursor_x > _tty.col_size - 1) {
		_tty.cursor_x = 0;
		_tty.cursor_y++;
	}

	// Check to make sure the cursor has not reached below the bottom of the
	// screen.
	if (_tty.cursor_y > _tty.row_size - 1) {
		scroll(_tty.graphics_ctx, get_font_height());

		_tty.cursor_y--;
	}
}

void TTY_puts(const char *str)
{
	for (int i = 0; str[i] != '\0'; i++) {
		if (str[i] == '\e') {
			if (memcmp(str + i, BLK, 7) == 0) {
				set_stroke(_tty.graphics_ctx, 0x000000);
				i += 6;
			} else if (memcmp(str + i, RED, 7) == 0) {
				set_stroke(_tty.graphics_ctx, 0xff0000);
				i += 6;
			} else if (memcmp(str + i, GRN, 7) == 0) {
				set_stroke(_tty.graphics_ctx, 0x00ff00);
				i += 6;
			} else if (memcmp(str + i, YEL, 7) == 0) {
				set_stroke(_tty.graphics_ctx, 0xffff00);
				i += 6;
			} else if (memcmp(str + i, BLU, 7) == 0) {
				set_stroke(_tty.graphics_ctx, 0x0000ff);
				i += 6;
			} else if (memcmp(str + i, MAG, 7) == 0) {
				set_stroke(_tty.graphics_ctx, 0xff00ff);
				i += 6;
			} else if (memcmp(str + i, CYN, 7) == 0) {
				set_stroke(_tty.graphics_ctx, 0x00ffff);
				i += 6;
			} else if (memcmp(str + i, WHT, 7) == 0) {
				set_stroke(_tty.graphics_ctx, 0xffffff);
				i += 6;
			}

			continue;
		}

		TTY_putc(str[i]);
	}

	set_stroke(_tty.graphics_ctx, 0xffffff);
	swap_buffer(_tty.graphics_ctx);
}
