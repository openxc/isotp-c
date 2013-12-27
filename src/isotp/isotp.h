#ifndef __ISOTP_H__
#define __ISOTP_H__

#ifdef __cplusplus
extern "C" {
#endif

struct {
    uint16_t arb_id;
    void* message_received_callback;
    void* message_sent_callback;
    void* can_frame_sent_callback;

    // Private
    uint16_t timeout_ms;
    bool framePadding;
    uint8_t* receive_buffer;
    uint16_t received_buffer_size;
    uint16_t incoming_message_size;
    bool sending;
    // TODO timer
} IsoTpHandler;

enum {
    PCI_SINGLE,
    PCI_FIRST_FRAME,
    PCI_CONSECUTIVE_FRAME,
    PCI_FLOW_CONTROL_FRAME
} IsoTpProtocolControlInformation;

enum {
    PCI_FLOW_STATUS_CONTINUE,
    PCI_FLOW_STATUS_WAIT,
    PCI_FLOW_STATUS_OVERFLOW
} IsoTpFlowStatus;

const uint16_t MAX_ISO_TP_MESSAGE_SIZE = 4096;
const uint16_t MAX_CAN_FRAME_SIZE = 8;
const uint8_t ISO_TP_DEFAULT_RESPONSE_TIMEOUT = 100;
const bool ISO_TP_DEFAULT_FRAME_PADDING_STATUS = true;

IsoTpHandler isotp_init(uint16_t arbitration_id,
        IsoTpMessageReceivedHandler* message_received_callback,
        IsoTpMessageSentHandler* message_sent_callback,
        IsoTpCanFrameSentHandler* can_frame_sent_callback);

void isotp_set_timeout(uint16_t timeout);

// TODO we have to make sure to copy the payload internall if it's more than 1
// frame, the soure could go out of scope
bool isotp_send(const uint8_t* payload, uint16_t payload_size);

void isotp_receive_can_frame(const uint16_t arbitration_id, const uint8_t* data,
        const uint8_t length);

void isotp_destroy(IsoTpHandler* handler);

#ifdef __cplusplus
}
#endif

#endif // __ISOTP_H__
