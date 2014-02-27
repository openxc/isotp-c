#include <isotp/isotp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

IsoTpShims SHIMS;
IsoTpReceiveHandle RECEIVE_HANDLE;

uint32_t last_can_frame_sent_arb_id;
uint8_t last_can_payload_sent[8];
uint8_t last_can_payload_size;
bool can_frame_was_sent;

bool message_was_received;
uint32_t last_message_received_arb_id;
uint8_t last_message_received_payload[OUR_MAX_ISO_TP_MESSAGE_SIZE];
uint8_t last_message_received_payload_size;

uint32_t last_message_sent_arb_id;
bool last_message_sent_status;
uint8_t last_message_sent_payload[OUR_MAX_ISO_TP_MESSAGE_SIZE];
uint8_t last_message_sent_payload_size;

void debug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    printf("\r\n");
    va_end(args);
}

bool mock_send_can(const uint32_t arbitration_id, const uint8_t* data,
        const uint8_t size) {
    can_frame_was_sent = true;
    last_can_frame_sent_arb_id = arbitration_id;
    last_can_payload_size = size;
    if(size > 0) {
        memcpy(last_can_payload_sent, data, size);
    }
    return true;
}

void message_received(const IsoTpMessage* message) {
    debug("Received ISO-TP message:");
    message_was_received = true;
    char str_message[48] = {0};
    isotp_message_to_string(message, str_message, sizeof(str_message));
    debug("%s", str_message);
    last_message_received_arb_id = message->arbitration_id;
    last_message_received_payload_size = message->size;
    if(message->size > 0) {
        memcpy(last_message_received_payload, message->payload, message->size);
    }
}

void message_sent(const IsoTpMessage* message, const bool success) {
    if(success) {
        debug("Sent ISO-TP message:");
    } else {
        debug("Unable to send ISO-TP message:");
    }
    char str_message[48] = {0};
    isotp_message_to_string(message, str_message, sizeof(str_message));
    debug("%s", str_message);

    last_message_sent_arb_id = message->arbitration_id;
    last_message_sent_payload_size = message->size;
    last_message_sent_status = success;
    if(message->size > 0) {
        memcpy(last_message_sent_payload, message->payload, message->size);
    }
}

void can_frame_sent(const uint32_t arbitration_id, const uint8_t* payload,
        const uint8_t size) {
    debug("Sent CAN Frame with arb ID 0x%x and %d bytes", arbitration_id, size);
}

void setup() {
    SHIMS = isotp_init_shims(debug, mock_send_can, NULL);
    RECEIVE_HANDLE = isotp_receive(&SHIMS, 0x2a, message_received);
    memset(last_message_sent_payload, 0, OUR_MAX_ISO_TP_MESSAGE_SIZE);
    memset(last_message_received_payload, 0, OUR_MAX_ISO_TP_MESSAGE_SIZE);
    memset(last_can_payload_sent, 0, sizeof(last_can_payload_sent));
    last_message_sent_status = false;
    message_was_received = false;
    can_frame_was_sent = false;
}

