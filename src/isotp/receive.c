#include <isotp/receive.h>

bool isotp_handle_single_frame(IsoTpReceiveHandle* handle, IsoTpMessage* message) {
    isotp_complete_receive(handle, message);
    return true;
}

void isotp_complete_receive(IsoTpReceiveHandle* handle, IsoTpMessage* message) {
    if(handle->message_received_callback != NULL) {
        handle->message_received_callback(message);
    }
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
