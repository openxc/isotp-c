#include <isotp/isotp.h>

const uint16_t MAX_ISO_TP_MESSAGE_SIZE = 4096;
const uint16_t MAX_CAN_FRAME_SIZE = 8;
const uint8_t ISO_TP_DEFAULT_RESPONSE_TIMEOUT = 100;
const bool ISO_TP_DEFAULT_FRAME_PADDING_STATUS = true;

void isotp_receive_can_frame(const uint16_t arbitration_id, const uint8_t* data,
        const uint8_t length) {
    //match with any request we made
    //handle flow control if necessary
    //call callback if message completed
}

bool isotp_send(const uint8_t* payload, uint16_t payload_size) {
     // we determine if it's single/multi frame and start the send
}

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
        message_received_callback: message_received_callback,
        message_sent_callback: message_sent_callback,
        can_frame_sent_callback: can_frame_sent_callback,
        timeout_ms: ISO_TP_DEFAULT_RESPONSE_TIMEOUT,
        frame_padding: ISO_TP_DEFAULT_FRAME_PADDING_STATUS,
        sending: false
    };
    return handler;
}

// TODO this would be better as a "isotp_message_to_string"
void log_isotp_message(const uint16_t arbitration_id,
        const uint8_t* payload, const uint16_t size) {
    debug("ID: 0x%02x, Payload:", arbitration_id);
    for(int i = 0; i < size; i++) {
        debug("0x%x", payload[i]);
    }
}
