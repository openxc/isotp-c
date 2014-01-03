#ifndef __ISOTP_SEND_H__
#define __ISOTP_SEND_H__

#include <isotp/isotp.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Public: A handle for beginning and continuing sending a single ISO-TP
 * message - both single and multi-frame.
 *
 * Since an ISO-TP message may contain multiple frames, we need to keep a handle
 * around while waiting for flow control messages from the receiver.
 * This struct encapsulates the local state required.
 *
 * completed - True if the message was completely sent, or the send was
 *      otherwise cancelled.
 * success - True if the message send request was successful. The value if this
 *      field isn't valid if 'completed' isn't true.
 */
typedef struct {
    bool completed;
    bool success;

    // Private
    uint16_t sending_arbitration_id;
    uint16_t receiving_arbitration_id;
    IsoTpMessageSentHandler message_sent_callback;
    IsoTpCanFrameSentHandler can_frame_sent_callback;
    // TODO going to need some state here for multi frame messages
} IsoTpSendHandle;

/* Public: Initiate sending a single ISO-TP message.
 *
 * If the message fits in a single ISO-TP frame (i.e. the payload isn't more
 * than 7 bytes) it will be sent immediately and the returned IsoTpSendHandle's
 * 'completed' flag will be true.
 *
 * For multi-frame messages, see isotp_continue_send(...).
 *
 * shims -  Low-level shims required to send CAN messages, etc.
 * arbitration_id - The arbitration ID to send the message on.
 * payload - The payload for the message. If no payload, NULL is valid is size
 *      is also 0.
 * size - The size of the payload, or 0 if no payload.
 * callback - an optional function to be called when the message is completely
 *      sent (use NULL if no callback required).
 *
 * Returns a handle to be used with isotp_continue_send to continue sending
 * multi-frame messages. The 'completed' field in the returned IsoTpSendHandle
 * will be true when the message is completely sent.
 */
IsoTpSendHandle isotp_send(IsoTpShims* shims, const uint16_t arbitration_id,
        const uint8_t payload[], uint16_t size,
        IsoTpMessageSentHandler callback);

/* Public: Continue to send a multi-frame ISO-TP message, based on a freshly
 * received CAN message (potentially from the receiver about flow control).
 *
 * For a multi-frame ISO-TP message, this function must be called
 * repeatedly whenever a new CAN message is received in order to complete the
 * send. The sender can't just blast everything onto the bus at once - it must
 * wait for some response from the receiver to know how much to send at once.
 *
 * shims -  Low-level shims required to send CAN messages, etc.
 * handle - An IsoTpSendHandle previously returned by isotp_send(...).
 * arbitration_id - The arbitration_id of the received CAN message.
 * data - The data of the received CAN message.
 * size - The size of the data in the received CAN message.
 *
 * Returns true if the message was completely sent, or the send was
 *      otherwise cancelled. Check the 'success' field of the handle to see if
 *      it was successful.
 */
bool isotp_continue_send(IsoTpShims* shims, IsoTpSendHandle* handle,
        const uint16_t arbitration_id, const uint8_t data[],
        const uint8_t size);

#ifdef __cplusplus
}
#endif

#endif // __ISOTP_SEND_H__
