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
    snprintf(destination, destination_length, "ID: 0x%02x, Payload: 0x%02x%02x%02x%02x%02x%02x%02x%02x",
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

IsoTpMessage isotp_continue_receive(IsoTpShims* shims,
        IsoTpReceiveHandle* handle, const uint16_t arbitration_id,
        const uint8_t data[], const uint8_t size) {
    IsoTpMessage message = {
        arbitration_id: arbitration_id,
        completed: false,
        payload: {0},
        size: 0
    };

    if(size < 1) {
        return message;
    }

    if(handle->arbitration_id != arbitration_id) {
        if(shims->log != NULL)  {
            shims->log("The arb ID 0x%x doesn't match the expected rx ID 0x%x",
                    arbitration_id, handle->arbitration_id);
        }
        return message;
    }

    IsoTpProtocolControlInformation pci = (IsoTpProtocolControlInformation)
            get_nibble(data, size, 0);

    uint8_t payload_length = get_nibble(data, size, 1);
    uint8_t payload[payload_length];
    if(payload_length > 0 && size > 0) {
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

bool isotp_continue_send(IsoTpShims* shims, IsoTpSendHandle* handle,
        const uint16_t arbitration_id, const uint8_t data[],
        const uint8_t size) {
    // TODO this will need to be tested when we add multi-frame support,
    // which is when it'll be necessary to pass in CAN messages to SENDING
    // handles.
    if(handle->receiving_arbitration_id != arbitration_id) {
        if(shims->log != NULL) {
            shims->log("The arb ID 0x%x doesn't match the expected tx continuation ID 0x%x",
                    arbitration_id, handle->receiving_arbitration_id);
        }
        return false;
    }
    return false;
}

