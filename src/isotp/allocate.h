#ifndef __ISOTP_ALLOCATE_H__
#define __ISOTP_ALLOCATE_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* The implementation of the two functions is platform-specific and is left to
 * the user. Implementation used for testing is based on stdlib's malloc/free
 * and can be found in tests/common.c */

uint8_t* allocate (size_t size);
void free_allocated (uint8_t* data);

#ifdef __cplusplus
}
#endif

#endif // __ISOTP_ALLOCATE_H_