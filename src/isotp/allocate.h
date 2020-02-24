#ifndef __ISOTP_ALLOCATE_H__
#define __ISOTP_ALLOCATE_H__

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t* allocate (size_t size);
void free_allocated (uint8_t* data);

#ifdef __cplusplus
}
#endif

#endif // __ISOTP_ALLOCATE_H_