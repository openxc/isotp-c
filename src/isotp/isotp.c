#include <isotp/isotp.h>
#include <isotp/receive.h>
#include <bitfield/bitfield.h>

const uint16_t MAX_ISO_TP_MESSAGE_SIZE = 4096;
const uint16_t MAX_CAN_FRAME_SIZE = 8;
const uint8_t ISO_TP_DEFAULT_RESPONSE_TIMEOUT = 100;
const bool ISO_TP_DEFAULT_FRAME_PADDING_STATUS = true;

/* void isotp_set_timeout(IsoTpHandler* handler, uint16_t timeout_ms) { */
    /* handler->timeout_ms = timeout_ms; */
/* } */

IsoTpShims isotp_init_shims(LogShim log, SendCanMessageShim send_can_message,
        SetTimerShim set_timer) {
    IsoTpShims shims = {
        log: log,
        send_can_message: send_can_message,
        set_timer: set_timer
    };
    return shims;
}

void isotp_message_to_string(const IsoTpMessage* message, char* destination,
        size_t destination_length) {
    char payload_string[message->size * 2 + 1];
    for(int i = 0; i < message->size; i++) {
        // TODO, bah this isn't working because snprintf hits the NULL char that
        // it wrote the last time and stops cold
        snprintf(&payload_string[i * 2], 2, "%02x", message->payload[i]);
    }
    snprintf(destination, destination_length, "ID: 0x%02x, Payload: 0x%s",
            message->arbitration_id, payload_string);
}

bool isotp_receive_can_frame(IsoTpShims* shims, IsoTpHandle* handle,
        const uint16_t arbitration_id, const uint8_t data[],
        const uint8_t data_length) {
    bool message_completed = false;

    if(data_length < 1) {
        return message_completed;
    }

    if(handle->type == ISOTP_HANDLE_RECEIVING) {
        if(handle->receive_handle.arbitration_id != arbitration_id) {
            return message_completed;
        }
    } else if(handle->type == ISOTP_HANDLE_SENDING) {
        if(handle->send_handle.receiving_arbitration_id != arbitration_id) {
            return message_completed;
        }
    } else {
        shims->log("The ISO-TP handle is corrupt");
        return message_completed;
    }

    IsoTpProtocolControlInformation pci = (IsoTpProtocolControlInformation)
            get_nibble(data, data_length, 0);

    uint8_t payload_length = get_nibble(data, data_length, 1);
    uint8_t payload[payload_length];
    if(payload_length > 0 && data_length > 0) {
        memcpy(payload, &data[1], payload_length);
    }

    // TODO this is set up to handle rx a response with a payload, but not to
    // handle flow control responses for multi frame messages that we're in the
    // process of sending

    switch(pci) {
        case PCI_SINGLE: {
            IsoTpMessage message = {
                arbitration_id: arbitration_id,
                payload: payload,
                size: payload_length
            };

            message_completed = isotp_handle_single_frame(handle, &message);
            break;
         }
        default:
            shims->log("Only single frame messages are supported");
            break;
    }
    return message_completed;
}
