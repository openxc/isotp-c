#include <isotp/send.h>

#define PCI_START_BIT 0
#define PCI_WIDTH 4
#define PAYLOAD_LENGTH_START_BIT PCI_START_BIT + PCI_WIDTH
#define PAYLOAD_LENGTH_WIDTH 4
#define PAYLOAD_START_BIT PAYLOAD_LENGTH_START_BIT + PAYLOAD_LENGTH_WIDTH

void isotp_complete_send(IsoTpHandler* handler, IsoTpMessage* message,
        bool status) {
    handler->message_sent_callback(message, status);
}

bool isotp_send_single_frame(IsoTpHandler* handler, IsoTpMessage* message) {
    uint64_t data = 0;
    setBitField(&data, PCI_SINGLE, PCI_START_BIT, PCI_WIDTH);
    setBitField(&data, message->size, PAYLOAD_LENGTH_START_BIT, PAYLOAD_LENGTH_WIDTH);
    // TODO need a better bitfield API to support this - use byte array instead
    // of uint64_t and specify desired total width
    for(int i = 0; i < message->size; i++) {
        setBitField(&data, message->payload[i], PAYLOAD_START_BIT + i * 8, 8);
    }

    uint8_t data_array[message->size + 1];
    for(int i = 0; i < sizeof(data_array); i++) {
        // TODO need getByte(x) function
        data_array[i] = getBitField(data, i * 8, 8, false);
    }
    handler->shims->send_can_message(message->arbitration_id, data_array, sizeof(data_array));
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
