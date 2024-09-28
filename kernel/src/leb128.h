#ifndef __UTILITY_H
#define __UTILITY_H 1

#include <stddef.h>
#include <stdint.h>

err_code leb128_to_u8(void **stream, uintptr_t stream_end, uint8_t *output);
err_code leb128_to_u16(void **stream, uintptr_t stream_end, uint16_t *output);
err_code leb128_to_u32(void **stream, uintptr_t stream_end, uint32_t *output);
err_code leb128_to_u64(void **stream, uintptr_t stream_end, uint64_t *output);

err_code leb128_to_s8(void **stream, uintptr_t stream_end, int8_t *output);
err_code leb128_to_s16(void **stream, uintptr_t stream_end, int16_t *output);
err_code leb128_to_s32(void **stream, uintptr_t stream_end, int32_t *output);
err_code leb128_to_s64(void **stream, uintptr_t stream_end, int64_t *output);

#endif
