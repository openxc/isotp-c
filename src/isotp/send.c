#include <isotp/send.h>

#define PCI_NIBBLE_INDEX 0
#define PAYLOAD_LENGTH_NIBBLE_INDEX 1
#define PAYLOAD_BYTE_INDEX 1

void isotp_complete_send(IsoTpHandler* handler, IsoTpMessage* message,
        bool status) {
    handler->message_sent_callback(message, status);
}

bool isotp_send_single_frame(IsoTpHandler* handler, IsoTpMessage* message) {
    uint8_t can_data[CAN_MESSAGE_BYTE_SIZE] = {0};
    if(!set_nibble(PCI_NIBBLE_INDEX, PCI_SINGLE, can_data, sizeof(can_data))) {
        handler->shims->log("Unable to set PCI in CAN data");
        return false;
    }

    if(!set_nibble(PAYLOAD_LENGTH_NIBBLE_INDEX, message->size, can_data,
                sizeof(can_data))) {
        handler->shims->log("Unable to set payload length in CAN data");
        return false;
    }

    if(message->size > 0) {
        memcpy(&can_data[1], message->payload, message->size);
    }

    handler->shims->send_can_message(message->arbitration_id, can_data,
            1 + message->size);
    isotp_complete_send(handler, message, true);
    return true;
}

bool isotp_send_multi_frame(IsoTpHandler* handler, IsoTpMessage* message) {
    // TODO make sure to copy payload into a local buffer
    handler->shims->log("Only single frame messages are supported");
    return false;
}

bool isotp_send(IsoTpHandler* handler, const uint8_t* payload,
        uint16_t size) {
    // we determine if it's single/multi frame and start the send
    IsoTpMessage message = {
        arbitration_id: handler->arbitration_id,
        payload: payload,
        size: size
    };

    if(size < 8) {
        return isotp_send_single_frame(handler, &message);
    } else {
        return isotp_send_multi_frame(handler, &message);
    }
}
