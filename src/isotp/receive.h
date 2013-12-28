#ifndef __ISOTP_RECEIVE_H__
#define __ISOTP_RECEIVE_H__

#include <isotp/isotp.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void isotp_handle_single_frame(IsoTpHandler* handler, IsoTpMessage* message);

void isotp_complete_receive(IsoTpHandler* handler, IsoTpMessage* message);

void isotp_receive_can_frame(IsoTpHandler* handler,
        const uint16_t arbitration_id, const uint64_t data, const uint8_t size);

#ifdef __cplusplus
}
#endif

#endif // __ISOTP_RECEIVE_H__
