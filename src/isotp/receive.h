#ifndef __ISOTP_RECEIVE_H__
#define __ISOTP_RECEIVE_H__

#include <isotp/isotp.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void isotp_complete_receive(IsoTpReceiveHandle* handle, IsoTpMessage* message);

bool isotp_handle_single_frame(IsoTpReceiveHandle* handle, IsoTpMessage* message);

IsoTpReceiveHandle isotp_receive(IsoTpShims* shims,
        const uint16_t arbitration_id, IsoTpMessageReceivedHandler callback);

IsoTpMessage isotp_continue_receive(IsoTpShims* shims,
        IsoTpReceiveHandle* handle, const uint16_t arbitration_id,
        const uint8_t data[], const uint8_t size);

#ifdef __cplusplus
}
#endif

#endif // __ISOTP_RECEIVE_H__
