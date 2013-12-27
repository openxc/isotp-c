#ifndef __ISOTP_H__
#define __ISOTP_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

const uint16_t MAX_ISO_TP_MESSAGE_SIZE;
const uint16_t MAX_CAN_FRAME_SIZE;
const uint8_t ISO_TP_DEFAULT_RESPONSE_TIMEOUT;
const bool ISO_TP_DEFAULT_FRAME_PADDING_STATUS;

typedef void (*LogShim)(const char* message);
typedef bool (*SendCanMessageShim)(const uint16_t arbitration_id, const uint8_t* data,
            const uint8_t size);
typedef bool (*SetTimerShim)(uint16_t time_ms, void (*callback));

typedef void (*IsoTpMessageReceivedHandler)(const uint16_t arbitration_id,
        const uint8_t* payload, const uint16_t size);
typedef void (*IsoTpMessageSentHandler)(const bool success,
        const uint16_t arbitration_id, const uint8_t* payload,
        const uint16_t size);
typedef void (*IsoTpCanFrameSentHandler)(const uint16_t arbitration_id,
        const uint8_t* payload, const uint8_t size);

// TODO this is really necessary and it would leak this library into the user's
// code
// typedef struct {
    // const uint16_t arbitration_id;
    // const uint8_t* payload;
    // const uint16_t size;
// } IsoTpMessage;

typedef struct {
    LogShim log;
    SendCanMessageShim send_can_message;
    SetTimerShim set_timer;
} IsoTpShims;

typedef struct {
    IsoTpShims* shims;
    uint16_t arb_id;
    IsoTpMessageReceivedHandler message_received_callback;
    IsoTpMessageSentHandler message_sent_callback;
    IsoTpCanFrameSentHandler can_frame_sent_callback;

    // Private
    uint16_t timeout_ms;
    bool frame_padding;
    uint8_t* receive_buffer;
    uint16_t received_buffer_size;
    uint16_t incoming_message_size;
    bool sending;
    // TODO timer callback
} IsoTpHandler;

typedef enum {
    PCI_SINGLE,
    PCI_FIRST_FRAME,
    PCI_CONSECUTIVE_FRAME,
    PCI_FLOW_CONTROL_FRAME
} IsoTpProtocolControlInformation;

typedef enum {
    PCI_FLOW_STATUS_CONTINUE,
    PCI_FLOW_STATUS_WAIT,
    PCI_FLOW_STATUS_OVERFLOW
} IsoTpFlowStatus;

IsoTpShims isotp_init_shims(LogShim log,
        SendCanMessageShim send_can_message,
        SetTimerShim set_timer);

IsoTpHandler isotp_init(IsoTpShims* shims,
        uint16_t arbitration_id,
        IsoTpMessageReceivedHandler message_received_callback,
        IsoTpMessageSentHandler message_sent_callback,
        IsoTpCanFrameSentHandler can_frame_sent_callback);

/* Public: Change the timeout for waiting on an ISO-TP response frame.
 *
 * If this function is not used, the conventional 100ms is used by default.
 *
 * handler - the ISO-TP handler to modify.
 * timeout - the new timeout in milliseconds.
 */
void isotp_set_timeout(IsoTpHandler* handler, uint16_t timeout_ms);

// TODO we have to make sure to copy the payload internall if it's more than 1
// frame, the soure could go out of scope
bool isotp_send(const uint8_t* payload, uint16_t payload_size);

void isotp_receive_can_frame(const uint16_t arbitration_id, const uint8_t* data,
        const uint8_t length);

void isotp_destroy(IsoTpHandler* handler);

void log_isotp_message(const uint16_t arbitration_id, const uint8_t* payload,
        const uint16_t size);


#ifdef __cplusplus
}
#endif

#endif // __ISOTP_H__
