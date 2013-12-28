#include <isotp/receive.h>

void isotp_handle_single_frame(IsoTpHandler* handler, IsoTpMessage* message) {
    isotp_complete_receive(handler, message);
}

void isotp_complete_receive(IsoTpHandler* handler, IsoTpMessage* message) {
    handler->message_received_callback(message);
}

void isotp_receive_can_frame(IsoTpHandler* handler,
        const uint16_t arbitration_id, const uint64_t data,
        const uint8_t length) {
    if(arbitration_id != handler->arbitration_id){
        return;
    }

    // TODO use CanMessage struct from canutil library - allocate payload buffer
    // on stack, 8 bytes
    IsoTpProtocolControlInformation pci = (IsoTpProtocolControlInformation)
            getBitField(data, 0, 4, false);

    IsoTpProtocolControlInformation pci = (IsoTpProtocolControlInformation)
            getNibble(0, data, 64, LITTE_ENDIAN);

    // TODO this is messed up! need a better API for grabbing bytes
    uint8_t payload_length = getBitField(data, 4, 4, false);
    uint8_t payload[payload_length];
    uint64_t flipped_data = __builtin_bswap64(data);
    if(payload_length > 0) {
        memcpy(payload, &(((uint8_t*)&flipped_data)[1]), payload_length);
    }

    IsoTpMessage message = {
        arbitration_id: arbitration_id,
        payload: payload,
        size: payload_length
    };

    switch(pci) {
        case PCI_SINGLE:
            isotp_handle_single_frame(handler, &message);
            break;
        default:
            handler->shims->log("Only single frame messages are supported");
            break;
    }
}

