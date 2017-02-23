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
	IsoTpMessage* message_ptr;

    // Private
    uint16_t sending_arbitration_id;
    uint16_t receiving_arbitration_id;
    IsoTpMessageSentHandler message_sent_callback;
    IsoTpCanFrameSentHandler can_frame_sent_callback;
    // TODO going to need some state here for multi frame messages
} IsoTpSendHandle;

/* Public: Initiate sending a single ISO-TP message.
 *
 * Create the IsoTpMessage object, copying over the payload data.
 * 
 * If the message fits in a single ISO-TP frame (i.e. the payload isn't more
 * than 7 bytes), call isotp_send_single_frame to immediately send. 
 * 
 * For messages with payload greater than 7 bytes, call isotp_send_multi_frame to
 * to initiate the multi-frame write sequence.  
 *
 * shims -  Low-level shims required to send CAN messages, etc.
 * arbitration_id - The arbitration ID to send the message on.
 * payload - The message payload; if none payload, set to NULL.
 * size - The size of the payload, or 0 if no payload.
 * callback - an optional function to be called when the message is completely
 *      sent (use NULL if no callback required).
 *
 * Returns a handle from appropriate send function to be used by UDS in printing
 * request status and matching to module diagnostic response. 
 */
IsoTpSendHandle isotp_send(IsoTpShims* shims, const uint16_t arbitration_id,
        const uint8_t payload[], uint16_t size,
        IsoTpMessageSentHandler callback);
		
/* Public: Complete sending of single frame ISO-TP message.
 *
 * Create send handle and CAN frame array. Populate frame with mode/service, 
 * DID and payload. Byte 0 denotes single frame message and length. 
 * 
 *
 * If configuration + copying payload successful, send the message and denote
 * send handle success. Handle will always be marked as complete.
 *
 * shims -  Low-level shims required to send CAN messages, etc.
 * message - Contains message arb id, payload and payload length. 
 * callback - an optional function to be called when the message is completely
 *      sent (use NULL if no callback required).
 *
 * Returns a send handle to isotp_send.
 
 IsoTpSendHandle isotp_send_single_frame(IsoTpShims* shims, IsoTpMessage* message,
        IsoTpMessageSentHandler callback);
		*/
		
/* Public: Send first frame of multi frame ISO-TP message.
 *
 * Create send handle and CAN frame array. Populate frame with mode/service, 
 * DID and payload. Nibble 0 denotes first frame message, 1-3 denote length. 
 * 
 *
 * If configuration + copying payload successful, send the message and denote
 * send handle success. Handle will always be marked as incomplete.
 *
 * shims -  Low-level shims required to send CAN messages, etc.
 * message - Contains message arb id, payload and payload length. 
 * callback - an optional function to be called when the message is completely
 *      sent (use NULL if no callback required).
 *
 * Returns a send handle to isotp_send, and eventually isotp_continue_send when
 * completing the multi-frame message sequence.
 
 IsoTpSendHandle isotp_send_multi_frame_frame(IsoTpShims* shims, IsoTpMessage* message,
        IsoTpMessageSentHandler callback);
		*/
		
/* Public: Send Second Frames of ISO-TP message that is greater than 7 bytes.
 * 
 * This function is to be called iteratively by isotp_continue_send to send the
 * second frames of a multi-frame write sequence. 
 *
 * shims -  Low-level shims required to send CAN messages, etc.
 * frame_count - keeps track of which frame in sequence.
 * num_frames - total number of frames needed to send message.
 * arbitration_id - The arbitration ID to send the message on.
 * payload - The message payload; if none payload, set to NULL.
 * size - The size of the payload, or 0 if no payload.
 * callback - an optional function to be called when the message is completely
 *      sent (use NULL if no callback required).
 *
 * Returns true if the message was completely sent.
 */
	bool isotp_send_second_frame(IsoTpShims* shims, uint16_t frame_count, uint8_t num_frames,
        const uint16_t arbitration_id, const uint8_t payload[],
        const uint8_t size);

/* Public: Complete sending multi-frame CAN message. 
 *
 * This function is called by UDS to iterate through the remaining payload of 
 * a multi-frame CAN message. Process is intiated by UDS processing a response
 * to an incomplete send handle. If UDS processes the receive buffer has being a
 * Flowcontrol Acknowledgement, i.e. nibble = 0 == 0x03, isotp_continue_send
 * iteratively calls isotp_send_second_frame until the payload is exhausted.
 *
 * Send handle is now marked as complete.
 *
 * shims -  Low-level shims required to send CAN messages, etc.
 * handle - An IsoTpSendHandle previously returned by isotp_send(...).
 * arbitration_id - The arbitration_id of the received CAN message.
 * payload - The message payload; if none payload, set to NULL..
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
