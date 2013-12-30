#include <isotp/isotp.h>
#include <isotp/receive.h>
#include <bitfield/bitfield.h>

const uint16_t MAX_ISO_TP_MESSAGE_SIZE = 4096;
const uint16_t MAX_CAN_FRAME_SIZE = 8;
const uint8_t ISO_TP_DEFAULT_RESPONSE_TIMEOUT = 100;
const bool ISO_TP_DEFAULT_FRAME_PADDING_STATUS = true;


void isotp_set_timeout(IsoTpHandler* handler, uint16_t timeout_ms) {
    handler->timeout_ms = timeout_ms;
}

IsoTpShims isotp_init_shims(LogShim log, SendCanMessageShim send_can_message,
        SetTimerShim set_timer) {
    IsoTpShims shims = {
        log: log,
        send_can_message: send_can_message,
        set_timer: set_timer
    };
    return shims;
}

IsoTpHandler isotp_init(IsoTpShims* shims, uint16_t arbitration_id,
        IsoTpMessageReceivedHandler message_received_callback,
        IsoTpMessageSentHandler message_sent_callback,
        IsoTpCanFrameSentHandler can_frame_sent_callback) {
    IsoTpHandler handler = {
        shims: shims,
        arbitration_id: arbitration_id,
        message_received_callback: message_received_callback,
        message_sent_callback: message_sent_callback,
        can_frame_sent_callback: can_frame_sent_callback,
        timeout_ms: ISO_TP_DEFAULT_RESPONSE_TIMEOUT,
        frame_padding: ISO_TP_DEFAULT_FRAME_PADDING_STATUS,
        sending: false
    };
    return handler;
}

void isotp_message_to_string(const IsoTpMessage* message, char* destination,
        size_t destination_length) {
    snprintf(destination, destination_length,"ID: 0x%02x, Payload: 0x%llx",
            // TODO the payload may be backwards here
            message->arbitration_id, message->payload);
}
