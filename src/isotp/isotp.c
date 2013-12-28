#include <isotp/isotp.h>
#include <isotp/receive.h>
#include <bitfield/bitfield.h>

const uint16_t MAX_ISO_TP_MESSAGE_SIZE = 4096;
const uint16_t MAX_CAN_FRAME_SIZE = 8;
const uint8_t ISO_TP_DEFAULT_RESPONSE_TIMEOUT = 100;
const bool ISO_TP_DEFAULT_FRAME_PADDING_STATUS = true;

void isotp_receive_can_frame(IsoTpHandler* handler,
        const uint16_t arbitration_id, const uint8_t* data, const uint8_t length) {
    if(arbitration_id != handler->arbitration_id){
        return;
    }

    // TODO use CanMessage struct from canutil library - allocate payload buffer
    // on stack, 8 bytes
    // TODO  this function should receive uint64_t...
    IsoTpProtocolControlInformation pci = (IsoTpProtocolControlInformation)
            getBitField((uint64_t)data, 0, 2, false);

    switch(pci) {
        case PCI_SINGLE:
            isotp_handle_single_frame(handler, arbitration_id, data, length);
            break;
        default:
            handler->shims->log("Only single frame messages are supported");
            break;
    }
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

// TODO this would be better as a "isotp_message_to_string"
void log_isotp_message(const uint16_t arbitration_id,
        const uint8_t* payload, const uint16_t size) {
    debug("ID: 0x%02x, Payload:", arbitration_id);
    for(int i = 0; i < size; i++) {
        debug("0x%x", payload[i]);
    }
}
