#include <isotp/receive.h>

void isotp_handle_single_frame(IsoTpHandler* handler, IsoTpMessage* message) {
    isotp_complete_receive(handler, message);
}

void isotp_complete_receive(IsoTpHandler* handler, IsoTpMessage* message) {
    handler->message_received_callback(message);
}

void isotp_receive_can_frame(IsoTpHandler* handler,
        const uint16_t arbitration_id, const uint8_t data[],
        const uint8_t data_length) {
    if(arbitration_id != handler->arbitration_id || data_length < 1) {
        return;
    }

    IsoTpProtocolControlInformation pci = (IsoTpProtocolControlInformation)
            get_nibble(data, data_length, 0);

    uint8_t payload_length = get_nibble(data, data_length, 1);
    uint8_t payload[payload_length];
    if(payload_length > 0 && data_length > 0) {
        memcpy(payload, &data[1], payload_length);
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

