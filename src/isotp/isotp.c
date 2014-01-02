#include <isotp/isotp.h>
#include <isotp/receive.h>
#include <bitfield/bitfield.h>

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
    // TODO why is this still not printing the entire payload?
    snprintf(destination, destination_length, "ID: 0x%02x, Payload: 0x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
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

IsoTpMessage isotp_receive_can_frame(IsoTpShims* shims, IsoTpHandle* handle,
        const uint16_t arbitration_id, const uint8_t data[],
        const uint8_t data_length) {
    IsoTpMessage message = {
        arbitration_id: arbitration_id,
        completed: false,
        payload: {0},
        size: 0
    };

    if(data_length < 1) {
        return message;
    }

    if(handle->type == ISOTP_HANDLE_RECEIVING) {
        if(handle->receive_handle.arbitration_id != arbitration_id) {
            if(shims->log != NULL)  {
                shims->log("The arb ID 0x%x doesn't match the expected rx ID 0x%x",
                        arbitration_id, handle->receive_handle.arbitration_id);
            }
            return message;
        }
    } else if(handle->type == ISOTP_HANDLE_SENDING) {
        if(handle->send_handle.receiving_arbitration_id != arbitration_id) {
            if(shims->log != NULL) {
                shims->log("The arb ID 0x%x doesn't match the expected tx continuation ID 0x%x",
                        arbitration_id, handle->send_handle.receiving_arbitration_id);
            }
            return message;
        }
    } else {
        shims->log("The ISO-TP handle is corrupt");
        return message;
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
            if(payload_length > 0) {
                memcpy(message.payload, payload, payload_length);
            }
            message.size = payload_length;
            message.completed = true;
            handle->success = true;
            handle->completed = true;
            isotp_handle_single_frame(handle, &message);
            break;
         }
        default:
            shims->log("Only single frame messages are supported");
            break;
    }
    return message;
}
