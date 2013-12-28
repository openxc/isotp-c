#include <isotp/isotp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

IsoTpShims SHIMS;
IsoTpHandler ISOTP_HANDLER;

uint16_t last_can_frame_sent_arb_id;
uint8_t last_can_payload_sent[8];
uint8_t last_can_payload_size;
bool can_frame_was_sent;

bool message_was_received;
uint16_t last_message_received_arb_id;
uint8_t* last_message_received_payload;
uint8_t last_message_received_payload_size;

uint16_t last_message_sent_arb_id;
bool last_message_sent_status;
uint8_t* last_message_sent_payload;
uint8_t last_message_sent_payload_size;

void debug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    printf("\r\n");
    va_end(args);
}

void mock_send_can(const uint16_t arbitration_id, const uint8_t* data,
        const uint8_t size) {
    can_frame_was_sent = true;
    last_can_frame_sent_arb_id = arbitration_id;
    last_can_payload_size = size;
    if(size > 0) {
        memcpy(last_can_payload_sent, data, size);
    }
}

void mock_set_timer(uint16_t time_ms, void (*callback)) {
}

void message_received(const uint16_t arbitration_id, const uint8_t* payload,
        const uint16_t size) {
    debug("Received ISO-TP message:");
    message_was_received = true;
    log_isotp_message(arbitration_id, payload, size);
    last_message_received_arb_id = arbitration_id;
    last_message_received_payload_size = size;
    if(size > 0) {
        memcpy(last_message_received_payload, payload, size);
    }
}

void message_sent(const uint16_t arbitration_id, const uint8_t* payload,
        const uint16_t size, const bool success) {
    if(success) {
        debug("Sent ISO-TP message:");
    } else {
        debug("Unable to send ISO-TP message:");
    }
    log_isotp_message(arbitration_id, payload, size);

    last_message_sent_arb_id = arbitration_id;
    last_message_sent_payload_size = size;
    last_message_sent_status = success;
    if(size > 0) {
        memcpy(last_message_sent_payload, payload, size);
    }
}

void can_frame_sent(const uint16_t arbitration_id,
        const uint8_t* payload, const uint8_t size) {
    debug("Sent CAN Frame:");
    log_isotp_message(arbitration_id, payload, size);
    for(int i = 0; i < size; i++) {
        debug("0x%x", payload[i]);
    }
}

void setup() {
    SHIMS = isotp_init_shims(debug, mock_send_can, mock_set_timer);
    ISOTP_HANDLER = isotp_init(&SHIMS, 0x2a, message_received, message_sent,
            can_frame_sent);
    last_message_sent_payload = malloc(MAX_ISO_TP_MESSAGE_SIZE);
    last_message_received_payload = malloc(MAX_ISO_TP_MESSAGE_SIZE);
    memset(last_can_payload_sent, 0, sizeof(last_can_payload_sent));
    last_message_sent_status = false;
    message_was_received = false;
    can_frame_was_sent = false;
}

