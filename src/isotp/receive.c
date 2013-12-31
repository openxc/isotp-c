#include <isotp/receive.h>

void isotp_handle_single_frame(IsoTpHandle* handle,
        IsoTpMessage* message) {
    isotp_complete_receive(handle, message);
}

void isotp_complete_receive(IsoTpHandle* handle, IsoTpMessage* message) {
    handle->receive_handle.message_received_callback(message);
}

IsoTpHandle isotp_receive(IsoTpShims* shims,
        const uint16_t arbitration_id, IsoTpMessageReceivedHandler callback) {
    IsoTpReceiveHandle receive_handle = {
        arbitration_id: arbitration_id,
        message_received_callback: callback
    };

    IsoTpHandle handle = {
        success: false,
        completed: true,
        receive_handle: receive_handle,
        type: ISOTP_HANDLE_RECEIVING
    };
    return handle;
}

