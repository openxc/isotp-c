#ifndef __ISOTP_TYPES__
#define __ISOTP_TYPES__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define CAN_MESSAGE_BYTE_SIZE 8
#define MAX_ISO_TP_MESSAGE_SIZE 4096
// TODO we want to avoid malloc, and we can't be allocated 4K on the stack for
// each IsoTpMessage, so for now we're setting an artificial max message size
// here - since we only handle single frame messages, 8 bytes is plenty.
#define OUR_MAX_ISO_TP_MESSAGE_SIZE 8

/* Private: The default timeout to use when waiting for a response during a
 * multi-frame send or receive.
 */
#define ISO_TP_DEFAULT_RESPONSE_TIMEOUT 100

/* Private: Determines if by default, padding is added to ISO-TP message frames.
 */
#define ISO_TP_DEFAULT_FRAME_PADDING_STATUS true

#ifdef __cplusplus
extern "C" {
#endif

/* Public: A container for a sent or received ISO-TP message.
 *
 * completed - An IsoTpMessage is the return value from a few functions - this
 *      attribute will be true if the message is actually completely received.
 *      If the function returns but is only partially through receiving the
 *      message, this will be false and you should not consider the other data
 *      to be valid.
 * arbitration_id - The arbitration ID of the message.
 * payload - The optional payload of the message - don't forget to check the
 *      size!
 * size -  The size of the payload. The size will be 0 if there is no payload.
 */
typedef struct {
    const uint16_t arbitration_id;
    uint8_t payload[OUR_MAX_ISO_TP_MESSAGE_SIZE];
    uint16_t size;
    bool completed;
} IsoTpMessage;

/* Public: The type signature for an optional logging function, if the user
 * wishes to provide one. It should print, store or otherwise display the
 * message.
 *
 * message - A format string to log using the given parameters.
 * ... (vargs) - the parameters for the format string.
 */
typedef void (*LogShim)(const char* message, ...);
/* Public: The type signature for a function to send a single CAN message.
 *
 * arbitration_id - The arbitration ID of the message.
 * data - The data payload for the message. NULL is valid if size is also 0.
 * size - The size of the data payload, in bytes.
 *
 * Returns true if the CAN message was sent successfully.
 */
typedef bool (*SendCanMessageShim)(const uint16_t arbitration_id,
        const uint8_t* data, const uint8_t size);

/* Public: The type signature for a... TODO, not used yet.
 */
typedef bool (*SetTimerShim)(uint16_t time_ms, void (*callback));

/* Public: The signature for a function to be called when an ISO-TP message has
 * been completely received.
 *
 * message - The received message.
 */
typedef void (*IsoTpMessageReceivedHandler)(const IsoTpMessage* message);

/* Public: the signature for a function to be called when an ISO-TP message has
 * been completely sent, or had a fatal error during sending.
 *
 * message - The sent message.
 * success - True if the message was sent successfully.
 */
typedef void (*IsoTpMessageSentHandler)(const IsoTpMessage* message,
        const bool success);

/* Public: The signature for a function to be called when a CAN frame has been
 * sent as as part of sending or receive an ISO-TP message.
 *
 * This is really only useful for debugging the library itself.
 *
 * message - The ISO-TP message that generated this CAN frame.
 */
typedef void (*IsoTpCanFrameSentHandler)(const IsoTpMessage* message);

/* Public: A container for the 3 shim functions used by the library to interact
 * with the wider system.
 *
 * Use the isotp_init_shims(...) function to create an instance of this struct.
 */
typedef struct {
    LogShim log;
    SendCanMessageShim send_can_message;
    SetTimerShim set_timer;
} IsoTpShims;

/* Private: PCI types, for identifying each frame of an ISO-TP message.
 */
typedef enum {
    PCI_SINGLE = 0x0,
    PCI_FIRST_FRAME = 0x1,
    PCI_CONSECUTIVE_FRAME = 0x2,
    PCI_FLOW_CONTROL_FRAME = 0x3
} IsoTpProtocolControlInformation;

/* Private: PCI flow control identifiers.
 */
typedef enum {
    PCI_FLOW_STATUS_CONTINUE = 0x0,
    PCI_FLOW_STATUS_WAIT = 0x1,
    PCI_FLOW_STATUS_OVERFLOW = 0x2
} IsoTpFlowStatus;

#ifdef __cplusplus
}
#endif

#endif // __ISOTP_TYPES__
