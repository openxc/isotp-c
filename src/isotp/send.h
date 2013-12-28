#ifndef __ISOTP_SEND_H__
#define __ISOTP_SEND_H__

#include <isotp/isotp.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool isotp_send(IsoTpHandler* handler, const uint8_t* payload,
        uint16_t payload_size);

#ifdef __cplusplus
}
#endif

#endif // __ISOTP_SEND_H__
