#include <isotp/receive.h>

bool isotp_handle_single_frame(IsoTpHandle* handle, IsoTpMessage* message) {
    isotp_complete_receive(handle, message);
    return true;
}

void isotp_complete_receive(IsoTpHandle* handle, IsoTpMessage* message) {
    if(handle->receive_handle.message_received_callback != NULL) {
        handle->receive_handle.message_received_callback(message);
    }
}

IsoTpHandle isotp_receive(IsoTpShims* shims,
        const uint16_t arbitration_id, IsoTpMessageReceivedHandler callback) {
    IsoTpReceiveHandle receive_handle = {
        arbitration_id: arbitration_id,
        message_received_callback: callback
    };

    IsoTpHandle handle = {
        success: false,
        completed: false,
        receive_handle: receive_handle,
        type: ISOTP_HANDLE_RECEIVING
    };
    return handle;
}
