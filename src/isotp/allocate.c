#include <isotp/allocate.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>

uint8_t* allocate (size_t size)
{
    return (uint8_t*) malloc((sizeof(uint8_t))* size);
}

void free_allocated (uint8_t* data)
{
    free(data);
}