#include <isotp/receive.h>
#include <bitfield/bitfield.h>
#include <string.h>

static void isotp_complete_receive(IsoTpReceiveHandle* handle, IsoTpMessage* message) {
    if(handle->message_received_callback != NULL) {
        handle->message_received_callback(message);
    }
}

bool isotp_handle_single_frame(IsoTpReceiveHandle* handle, IsoTpMessage* message) {
    isotp_complete_receive(handle, message);
    return true;
}

IsoTpReceiveHandle isotp_receive(IsoTpShims* shims,
        const uint16_t arbitration_id, IsoTpMessageReceivedHandler callback) {
    IsoTpReceiveHandle handle = {
        success: false,
        completed: false,
        arbitration_id: arbitration_id,
        message_received_callback: callback
    };

    return handle;
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
