#include <isotp/isotp.h>
#include <isotp/receive.h>
#include <bitfield/bitfield.h>

const uint16_t MAX_ISO_TP_MESSAGE_SIZE = 4096;
const uint16_t MAX_CAN_FRAME_SIZE = 8;
const uint8_t ISO_TP_DEFAULT_RESPONSE_TIMEOUT = 100;
const bool ISO_TP_DEFAULT_FRAME_PADDING_STATUS = true;

const uint8_t PCI_START_BIT = 0;
const uint8_t PCI_WIDTH = 4;
const uint8_t PAYLOAD_LENGTH_START_BIT = 4;
const uint8_t PAYLOAD_LENGTH_WIDTH = 4;
const uint8_t PAYLOAD_START_BIT = 8;

void isotp_receive_can_frame(IsoTpHandler* handler,
        const uint16_t arbitration_id, const uint64_t data,
        const uint8_t length) {
    if(arbitration_id != handler->arbitration_id){
        return;
    }

    // TODO use CanMessage struct from canutil library - allocate payload buffer
    // on stack, 8 bytes
    // TODO  this function should receive uint64_t...
    IsoTpProtocolControlInformation pci = (IsoTpProtocolControlInformation)
            getBitField(data, 0, 4, false);

    // TODO this is messed up! need a better API for grabbing bytes
    uint8_t payload_length = getBitField(data, 4, 4, false);
    uint8_t payload[payload_length];
    uint64_t flipped_data = __builtin_bswap64(data);
    if(payload_length > 0) {
        memcpy(payload, &(((uint8_t*)&flipped_data)[1]), payload_length);
    }

    switch(pci) {
        case PCI_SINGLE:
            isotp_handle_single_frame(handler, arbitration_id, payload,
                    payload_length);
            break;
        default:
            handler->shims->log("Only single frame messages are supported");
            break;
    }
}

void isotp_complete_send(IsoTpHandler* handler, const uint8_t* payload,
        uint8_t size, bool status) {
    handler->message_sent_callback(handler->arbitration_id, payload, size,
            status);
}

bool isotp_send_single_frame(IsoTpHandler* handler, const uint8_t* payload,
        uint8_t size) {
    uint64_t data;
    setBitField(&data, PCI_SINGLE, PCI_START_BIT, PCI_WIDTH);
    setBitField(&data, size, PAYLOAD_LENGTH_START_BIT, PAYLOAD_LENGTH_WIDTH);
    // TODO this is probably wrong
    if(size > 0) {
        setBitField(&data, *payload, PAYLOAD_START_BIT, size * 8);
    }
    handler->shims->send_can_message(handler->arbitration_id, payload, size);
    isotp_complete_send(handler, payload, size, true);
    return true;
}

bool isotp_send_multi_frame(IsoTpHandler* handler, const uint8_t* payload,
        uint16_t size) {
    return false;
}

bool isotp_send(IsoTpHandler* handler, const uint8_t* payload,
        uint16_t size) {
     // we determine if it's single/multi frame and start the send
     if(size < 8) {
         return isotp_send_single_frame(handler, payload, size);
     } else {
         return isotp_send_multi_frame(handler, payload, size);
     }
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
    debug("ID: 0x%02x", arbitration_id);
    if(size > 0) {
        debug("Payload:");
        for(int i = 0; i < size; i++) {
            debug("0x%x", payload[i]);
        }
    }  else {
        debug("(no payload)");
    }
}
