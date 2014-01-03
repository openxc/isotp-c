#ifndef __ISOTP_SEND_H__
#define __ISOTP_SEND_H__

#include <isotp/isotp.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool success;
    bool completed;
    uint16_t sending_arbitration_id;
    uint16_t receiving_arbitration_id;
    IsoTpMessageSentHandler message_sent_callback;
    IsoTpCanFrameSentHandler can_frame_sent_callback;
    // TODO going to need some state here for multi frame messages
} IsoTpSendHandle;

bool isotp_continue_send(IsoTpShims* shims, IsoTpSendHandle* handle,
        const uint16_t arbitration_id, const uint8_t data[],
        const uint8_t size);

IsoTpSendHandle isotp_send(IsoTpShims* shims, const uint16_t arbitration_id,
        const uint8_t payload[], uint16_t size,
        IsoTpMessageSentHandler callback);

#ifdef __cplusplus
}
#endif

#endif // __ISOTP_SEND_H__
