#include <isotp/receive.h>

void isotp_handle_single_frame(IsoTpHandler* handler,
        const uint16_t arbitration_id, const uint8_t* data,
        const uint8_t length) {
    handler->message_received_callback(arbitration_id, data, length);
}
