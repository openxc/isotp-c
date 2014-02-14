#include <isotp/send.h>
#include <bitfield/bitfield.h>
#include <string.h>

#define PCI_NIBBLE_INDEX 0
#define PAYLOAD_LENGTH_NIBBLE_INDEX 1
#define PAYLOAD_BYTE_INDEX 1

void isotp_complete_send(IsoTpShims* shims, IsoTpMessage* message,
        bool status, IsoTpMessageSentHandler callback) {
    if(callback != NULL) {
        callback(message, status);
    }
}

IsoTpSendHandle isotp_send_single_frame(IsoTpShims* shims, IsoTpMessage* message,
        IsoTpMessageSentHandler callback) {
    IsoTpSendHandle handle = {
        success: false,
        completed: true
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
            shims->frame_padding ? 8 : 1 + message->size);
    handle.success = true;
    isotp_complete_send(shims, message, true, callback);
    return handle;
}

IsoTpSendHandle isotp_send_multi_frame(IsoTpShims* shims, IsoTpMessage* message,
        IsoTpMessageSentHandler callback) {
    // TODO make sure to copy message into a local buffer
    shims->log("Only single frame messages are supported");
    IsoTpSendHandle handle = {
        success: false,
        completed: true
    };
    // TODO need to set sending and receiving arbitration IDs separately if we
    // can't always just add 0x8 (and I think we can't)
    return handle;
}

IsoTpSendHandle isotp_send(IsoTpShims* shims, const uint16_t arbitration_id,
        const uint8_t payload[], uint16_t size,
        IsoTpMessageSentHandler callback) {
    IsoTpMessage message = {
        arbitration_id: arbitration_id,
        size: size
    };

    memcpy(message.payload, payload, size);
    if(size < 8) {
        return isotp_send_single_frame(shims, &message, callback);
    } else {
        return isotp_send_multi_frame(shims, &message, callback);
    }
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
