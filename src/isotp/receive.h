#ifndef __ISOTP_RECEIVE_H__
#define __ISOTP_RECEIVE_H__

#include <isotp/isotp.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void isotp_complete_receive(IsoTpHandle* handle, IsoTpMessage* message);

void isotp_handle_single_frame(IsoTpHandle* handle,
        IsoTpMessage* message);

IsoTpHandle isotp_receive(IsoTpShims* shims, const uint16_t arbitration_id,
        IsoTpMessageReceivedHandler callback);

#ifdef __cplusplus
}
#endif

#endif // __ISOTP_RECEIVE_H__
