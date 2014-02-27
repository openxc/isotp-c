#include <isotp/isotp.h>
#include <bitfield/bitfield.h>
#include <inttypes.h>

/* void isotp_set_timeout(IsoTpHandler* handler, uint16_t timeout_ms) { */
    /* handler->timeout_ms = timeout_ms; */
/* } */

IsoTpShims isotp_init_shims(LogShim log, SendCanMessageShim send_can_message,
        SetTimerShim set_timer) {
    IsoTpShims shims = {
        log: log,
        send_can_message: send_can_message,
        set_timer: set_timer,
        frame_padding: ISO_TP_DEFAULT_FRAME_PADDING_STATUS
    };
    return shims;
}

void isotp_message_to_string(const IsoTpMessage* message, char* destination,
        size_t destination_length) {
    snprintf(destination, destination_length, "ID: 0x%" SCNd32 ", Payload: 0x%02x%02x%02x%02x%02x%02x%02x%02x",
            message->arbitration_id,
            message->payload[0],
            message->payload[1],
            message->payload[2],
            message->payload[3],
            message->payload[4],
            message->payload[5],
            message->payload[6],
            message->payload[7]);
}
