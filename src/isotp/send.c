#include <isotp/send.h>
#include <bitfield/bitfield.h>
#include <string.h>

#define PCI_NIBBLE_INDEX 0
#define PAYLOAD_LENGTH_NIBBLE_INDEX 1
#define PAYLOAD_BYTE_INDEX 1
#define MULTI_PAYLOAD_BYTE_INDEX 2


void isotp_complete_send(IsoTpShims* shims, IsoTpMessage* message,
        bool status, IsoTpMessageSentHandler callback) {
    if(callback != NULL) {
        callback(message, status);
    }
}

// Depending on CAN message length, calls appropriate send frame function
IsoTpSendHandle isotp_send(IsoTpShims* shims, const uint16_t arbitration_id,
        const uint8_t payload[], uint16_t size,
        IsoTpMessageSentHandler callback) {
    IsoTpMessage message = {
        arbitration_id: arbitration_id,
        size: size
    };

    memcpy(message.payload, payload, size);
    if(size < 8) {
        return isotp_send_single_frame(shims, &message, callback);
    } else {
        return isotp_send_multi_frame(shims, &message, callback);
    }
}

// Function to send single 8 byte CAN frame
IsoTpSendHandle isotp_send_single_frame(IsoTpShims* shims, IsoTpMessage* message,
        IsoTpMessageSentHandler callback) {
    IsoTpSendHandle handle = {
        success: false,
        completed: true
    };    
	
	uint8_t can_data[CAN_MESSAGE_BYTE_SIZE] = {0};
	
	// define the message header as single CAN frame (PCI_SINGLE) in first nibble
    if(!set_nibble(PCI_NIBBLE_INDEX, PCI_SINGLE, can_data, sizeof(can_data))) {
        shims->log("Unable to set PCI in CAN data");
        return handle;
    }

	// define the total message size in the second nibble
    if(!set_nibble(PAYLOAD_LENGTH_NIBBLE_INDEX, message->size, can_data,
                sizeof(can_data))) {
        shims->log("Unable to set payload length in CAN data");
        return handle;
    }

	// if size is valid, copy the payload
    if(message->size > 0) {
        memcpy(&can_data[PAYLOAD_BYTE_INDEX], message->payload, message->size);
    }

	// send the CAN message and update handler to be true
    shims->send_can_message(message->arbitration_id, can_data,
            shims->frame_padding ? 8 : 1 + message->size);
    handle.success = true;
    isotp_complete_send(shims, message, true, callback);
    return handle;
}

// Function to send First Frame of multi-frame CAN message
IsoTpSendHandle isotp_send_multi_frame(IsoTpShims* shims, IsoTpMessage* message,
        IsoTpMessageSentHandler callback) {
	// note that we intentionally mark handle.completed as false to ensure UDS knows to continue sending
    IsoTpSendHandle handle = {
        success: false,
        completed: false
    };
			
	// create the buffer
	uint8_t can_data[CAN_MESSAGE_BYTE_SIZE] = {0};
	
	// define the message header as first CAN frame (PCI_FIRST_FRAME) in first nibble
    if(!set_nibble(PCI_NIBBLE_INDEX, PCI_FIRST_FRAME, can_data, sizeof(can_data))) {
        shims->log("Unable to set PCI in first frame CAN data");
        return handle;
    }

	// define the total message size (note, now we have 3 nibbles instead of 1)
	if(!set_bitfield(message->size,4,12,can_data,sizeof(can_data))){
        shims->log("Unable to set payload length in CAN data");
        return handle;
    }

	// begin payload after byte 2, but only copy first 6
    if(message->size > 0) {
        memcpy(&can_data[MULTI_PAYLOAD_BYTE_INDEX], message->payload, 6);
    }
	
	// send the CAN message and update handler to be true
    shims->send_can_message(message->arbitration_id, can_data,
            shims->frame_padding ? 8 : 1 + message->size);
	handle.message = *message;
    handle.success = true;
    isotp_complete_send(shims, message, true, callback);
    return handle;
}

bool isotp_send_second_frame(IsoTpShims* shims, uint16_t frame_count, uint16_t num_frames,IsoTpMessage* message,
        IsoTpMessageSentHandler callback) {
	// create the buffer
	uint8_t can_data[CAN_MESSAGE_BYTE_SIZE] = {0};
	
	// define the message header as second CAN frame (PCI_CONSECUTIVE_FRAME) in first nibble
    if(!set_nibble(PCI_NIBBLE_INDEX, PCI_CONSECUTIVE_FRAME, can_data, sizeof(can_data))) {
        shims->log("Unable to set PCI in second frame CAN data");
        return false;
    }

	// define which consecutive frame this is in the second nibble
    if(!set_nibble(PAYLOAD_LENGTH_NIBBLE_INDEX, message->size, can_data,
                sizeof(can_data))) {
        shims->log("Unable to set second frame number in CAN data");
        return false;
    }

	// copy the payload and send the message
	// note that payload reference is shifted by first 6 bits of first frame, and 7 for each consecutive
	// last frame we only need to take remainder
	uint16_t shift = 6+(count-1)*7;
    if(frame_count < num_frames) {	
		// begin payload after first byte, accounting for first frame has 6 bits and each second takes 7
		memcpy(&can_data[1], message->payload + shift, 7); 
		shims->send_can_message(message->arbitration_id, can_data, 8);
    }
	else{
		// if we're on the final frame, we need to be careful to only take the remainder in the payload
		memcpy(&can_data[1], message->payload + shift, message->size % shift); 		
		shims->send_can_message(message->arbitration_id, can_data, 1 + message->size % shift);
	}
	
	// send the CAN message and update handler to be true
    isotp_complete_send(shims, message, true, callback);
    return true;
}

void isotp_continue_send(IsoTpShims* shims, IsoTpSendHandle* handle,
        const uint16_t arbitration_id, const uint8_t data[],
        const uint8_t size) {
		
	// we need the flowcontrol Ack here
	if(handle->receiving_arbitration_id != arbitration_id + 0x08 || get_nibble(data, sizeof(data), PCI_NIBBLE_INDEX) != 0x3){
        shims->log("Incorrect flowcontrol response");
		handle.success = false;
		return;
	}
	
	// first we compute how many CAN frames are necessary
	uint16_t num_can_frames = handle->message.size/8;
	
	// send all the CAN second frames
	for(uint16_t count = 1; count <= num_can_frames; i++){
		// if one of them fails to send, break for loop and return handle
		if(!isotp_send_second_frame(shims, count, num_can_frames, &handle->message, callback)){
			handle->success = false;
			return;
		}
	}
	
	handle->completed = true;
}