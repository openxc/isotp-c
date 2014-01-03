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

#ifdef __cplusplus
}
#endif

#endif // __ISOTP_RECEIVE_H__
