#include <isotp/send.h>

#define PCI_NIBBLE_INDEX 0
#define PAYLOAD_LENGTH_NIBBLE_INDEX 1
#define PAYLOAD_BYTE_INDEX 1

void isotp_complete_send(IsoTpShims* shims, IsoTpMessage* message,
        bool status, IsoTpMessageSentHandler callback) {
    callback(message, status);
}

IsoTpHandle isotp_send_single_frame(IsoTpShims* shims, IsoTpMessage* message,
        IsoTpMessageSentHandler callback) {
    IsoTpHandle handle = {
        success: false,
        completed: true,
        type: ISOTP_HANDLE_SENDING
    };

    uint8_t can_data[CAN_MESSAGE_BYTE_SIZE] = {0};
    if(!set_nibble(PCI_NIBBLE_INDEX, PCI_SINGLE, can_data, sizeof(can_data))) {
        shims->log("Unable to set PCI in CAN data");
        return handle;
    }

    if(!set_nibble(PAYLOAD_LENGTH_NIBBLE_INDEX, message->size, can_data,
                sizeof(can_data))) {
        shims->log("Unable to set payload length in CAN data");
        return handle;
    }

    if(message->size > 0) {
        memcpy(&can_data[1], message->payload, message->size);
    }

    shims->send_can_message(message->arbitration_id, can_data,
            1 + message->size);
    handle.success = true;
    isotp_complete_send(shims, message, true, callback);
    return handle;
}

IsoTpHandle isotp_send_multi_frame(IsoTpShims* shims, IsoTpMessage* message,
        IsoTpMessageSentHandler callback) {
    // TODO make sure to copy payload into a local buffer
    shims->log("Only single frame messages are supported");
    IsoTpHandle handle = {
        success: false,
        completed: true,
        type: ISOTP_HANDLE_SENDING
    };
    return handle;
}

IsoTpHandle isotp_send(IsoTpShims* shims, const uint16_t arbitration_id,
        const uint8_t* payload, uint16_t size,
        IsoTpMessageSentHandler callback) {
    IsoTpMessage message = {
        arbitration_id: arbitration_id,
        payload: payload,
        size: size
    };

    if(size < 8) {
        return isotp_send_single_frame(shims, &message, callback);
    } else {
        return isotp_send_multi_frame(shims, &message, callback);
    }
}
