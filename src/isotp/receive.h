#ifndef __ISOTP_RECEIVE_H__
#define __ISOTP_RECEIVE_H__

#include <isotp/isotp.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Public: A handle for beginning and continuing receiving a single ISO-TP
 * message - both single and multi-frame.
 *
 * Since an ISO-TP message may contain multiple frames, we need to keep a handle
 * around while waiting for subsequent CAN messages to complete the message.
 * This struct encapsulates the local state required.
 *
 * completed - True if the received message request is completely finished.
 * success - True if the message request was successful. The value if this field
 *      isn't valid if 'completed' isn't true.
 */
typedef struct {
    bool completed;
    bool success;

    // Private
    uint32_t arbitration_id;
    IsoTpMessageReceivedHandler message_received_callback;
    uint16_t timeout_ms;
    // timeout_ms: ISO_TP_DEFAULT_RESPONSE_TIMEOUT,
    uint8_t* receive_buffer;
    uint16_t received_buffer_size;
    uint16_t incoming_message_size;
    // TODO timer callback for multi frame
} IsoTpReceiveHandle;

/* Public: Initiate receiving a single ISO-TP message on a particular
 * arbitration ID.
 *
 * Note that no actual CAN data has been received at this point - this just sets
 * up a handle to be used when new CAN messages to arrive, so they can be parsed
 * as potential single or multi-frame ISO-TP messages.
 *
 * shims -  Low-level shims required to send and receive CAN messages, etc.
 * arbitration_id - The arbitration ID to receive the message on.
 * callback - an optional function to be called when the message is completely
 *      received (use NULL if no callback required).
 *
 * Returns a handle to be used with isotp_continue_receive when a new CAN frame
 * arrives. The 'completed' field in the returned IsoTpReceiveHandle will be true
 * when the message is completely sent.
 */
IsoTpReceiveHandle isotp_receive(IsoTpShims* shims,
        const uint32_t arbitration_id, IsoTpMessageReceivedHandler callback);

/* Public: Continue to receive a an ISO-TP message, based on a freshly
 * received CAN message.
 *
 * For a multi-frame ISO-TP message, this function must be called
 * repeatedly whenever a new CAN message is received in order to complete
 * receipt.
 *
 * TODO does this API work for if we wanted to receive an ISO-TP message and
 * send our own flow control messages back?
 *
 * shims -  Low-level shims required to send and receive CAN messages, etc.
 * handle - An IsoTpReceiveHandle previously returned by isotp_receive(...).
 * arbitration_id - The arbitration_id of the received CAN message.
 * data - The data of the received CAN message.
 * size - The size of the data in the received CAN message.
 *
 * Returns an IsoTpMessage with the 'completed' field set to true if a message
 * was completely received. If 'completed' is false, more CAN frames are
 * required to complete the messages, or the arbitration ID didn't match this
 * handle. Keep passing the same handle to this function when CAN frames arrive.
 */
IsoTpMessage isotp_continue_receive(IsoTpShims* shims,
        IsoTpReceiveHandle* handle, const uint32_t arbitration_id,
        const uint8_t data[], const uint8_t size);

#ifdef __cplusplus
}
#endif

#endif // __ISOTP_RECEIVE_H__
