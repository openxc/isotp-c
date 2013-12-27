
void isotp_receive_can_frame(const uint16_t arbitration_id, const uint8_t* data,
        const uint8_t length) {
    //match with any request we made
    //handle flow control if necessary
    //call callback if message completed
}

bool isotp_send(const uint8_t* payload, uint16_t payload_size) {
     // we determine if it's single/multi frame and start the send
}

void isotp_set_timeout(uint16_t timeout) {
}
