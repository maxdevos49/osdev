#ifndef __TYPE_H
#define __TYPE_H 1

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// https://flak.tedunangst.com/post/to-errno-or-to-error

typedef int err_code; /* Zero means all is well. Anything else means something
						 went wrong! */

// Standard kernel error codes.
enum ERROR_CODE {
	NO_ERROR = 0x00,
	/* Array or pointer index out of bounds */
	ERROR_OUT_OF_BOUNDS = 0x01,
};

#endif
