#ifndef __TYPE_H
#define __TYPE_H 1

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// https://flak.tedunangst.com/post/to-errno-or-to-error

// Zero means all is well. Anything else means something went wrong!
typedef int err_code;

// Standard kernel error codes.
enum ERROR_CODE {
	NO_ERROR = 0x00,
	ERROR_OUT_OF_BOUNDS = 0x01,
	ERROR_UNEXPECTED_NULL_POINTER = 0x02,
	ERROR_DEPENDENCY_NOT_LOADED = 0x03,
	ERROR_UNSUPPORTED = 0x04,
	ERROR_NOT_IMPLEMENTED = 0x05,
	ERROR_NOT_FOUND = 0x06,
	ERROR_INSUFFICIENT_SPACE = 0x07,
	ERROR_INVALID_ADDRESS = 0x08,
	ERROR_ADDRESS_ALIGNMENT = 0x09,
	ERROR_ALREADY_USED = 0x0a,
	ERROR_ALREADY_FREE = 0x0b,
};

#endif
