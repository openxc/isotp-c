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

// send CAN message that can be contained within 1 8-byte frame (7 byte message max)
IsoTpSendHandle isotp_send_single_frame(IsoTpShims* shims, IsoTpMessage* message,
        IsoTpMessageSentHandler callback) {
	// create the send handle
    IsoTpSendHandle handle = {
        success: false,
        completed: true,
		message_ptr: message
    };

	// create the CAN frame array
    uint8_t can_data[CAN_MESSAGE_BYTE_SIZE] = {0};
	
	// set nibble 0 = 0 to define message as single frame
    if(!set_nibble(PCI_NIBBLE_INDEX, PCI_SINGLE, can_data, sizeof(can_data))) {
        shims->log("Unable to set PCI in CAN data");
        return handle;
    }

	// set nibble 1 = can message length
    if(!set_nibble(PAYLOAD_LENGTH_NIBBLE_INDEX, message->size, can_data,
                sizeof(can_data))) {
        shims->log("Unable to set payload length in CAN data");
        return handle;
    }

	// copy the CAN message into the frame array
    if(message->size > 0) {
        memcpy(&can_data[1], message->payload, message->size);
    }

	// write the message to the CAN bus and note send success
    shims->send_can_message(message->arbitration_id, can_data,
            shims->frame_padding ? 8 : 1 + message->size);
    handle.success = true;
    isotp_complete_send(shims, message, true, callback);
    return handle;
}

// send first frame of multi-frame CAN message
IsoTpSendHandle isotp_send_multi_frame(IsoTpShims* shims, IsoTpMessage* message,
        IsoTpMessageSentHandler callback) {	
	// create the send handle - note that completed = false here
    IsoTpSendHandle handle = {
        success: false,
        completed: false,
		message_ptr: message
    };
			
	// create the CAN frame array
	uint8_t can_data[CAN_MESSAGE_BYTE_SIZE] = {0};
	
	// set nibble 0 = 1 to define message as first frame of multi-frame sequence
    if(!set_nibble(PCI_NIBBLE_INDEX, PCI_FIRST_FRAME, can_data, sizeof(can_data))) {
        shims->log("Unable to set PCI in first frame CAN data");
        return handle;
    }

	// define the total message size (note, now we have 3 nibbles instead of 1)
	if(!set_bitfield(message->size,4,12,can_data,sizeof(can_data))){
        shims->log("Unable to set payload length in CAN data");
        return handle;
    }		

	// copy first 6 bytes of the message
    if(message->size > 0) {
        memcpy(&can_data[MULTI_PAYLOAD_BYTE_INDEX], message->payload, 6);
    }	
	
	// write the message to the CAN bus and note send success
    shims->send_can_message(message->arbitration_id, can_data,
            shims->frame_padding ? 8 : 1 + message->size);
    handle.success = true;
    isotp_complete_send(shims, message, true, callback);
	
    return handle;
}

// create the IsoTpMessage and call the appropriate CAN write function
IsoTpSendHandle isotp_send(IsoTpShims* shims, const uint16_t arbitration_id,
        const uint8_t payload[], uint16_t size,
        IsoTpMessageSentHandler callback) {
    IsoTpMessage message = {
        arbitration_id: arbitration_id,
        size: size
    };

	// copy the payload to the message
    memcpy(message.payload, payload, size);
	
	// if we can fit entire payload onto one message, send single frame
    if(size < 8) {
		return isotp_send_single_frame(shims, &message, callback);}
	// otherwise we need to send first frame of multi-frame sequencey
	else {
		return isotp_send_multi_frame(shims, &message, callback);}
}

// send second frames of multi-frame CAN message
bool isotp_send_second_frame(IsoTpShims* shims, uint16_t frame_count, uint8_t num_frames,
        const uint16_t arbitration_id, const uint8_t payload[],const uint8_t size) {
	// create the CAN frame array
	uint8_t can_data[CAN_MESSAGE_BYTE_SIZE] = {0};
	
	// set nibble 0 = 2 to define message as second frame of multi-frame sequence
    if(!set_nibble(PCI_NIBBLE_INDEX, PCI_CONSECUTIVE_FRAME, can_data, sizeof(can_data))) {
        shims->log("Unable to set PCI in second frame CAN data");
        return false;
    }

	// set nibble 1 to be second frame number of the sequence (i.e. second frame 1 out 3)
    if(!set_nibble(PAYLOAD_LENGTH_NIBBLE_INDEX, frame_count, can_data,sizeof(can_data))) {
        shims->log("Unable to set second frame number in CAN data");
        return false;
    }

	// compute payload array index
	int index = 6+(frame_count-1)*7;
	
	// if we haven't reached the final frame yet, populate all 7 bytes of CAN frame array
	if(frame_count+1 < num_frames) {
		memcpy(&can_data[1],&payload[index],7);}
	// if we are on the final frame yet, we only copy over remaining objects in payload array
	else {
		memcpy(&can_data[1], &payload[index], size - (uint8_t)index);}
	
	// send message and return true	
	shims->send_can_message(arbitration_id, can_data, 8);
    return true;
}

// determine number of second frames and then iterate through them
bool isotp_continue_send(IsoTpShims* shims, IsoTpSendHandle* handle,
        const uint16_t arbitration_id, const uint8_t data[],const uint8_t size) {
	// mark as complete now to free up diagnostic request handle
	handle->completed = true;
	
	// now compute the num can frames we need to send, given we send 7 bytes a time
	uint8_t frame_length = 0x7;
	uint8_t num_can_frames = 1+ size / frame_length;
	
	// send all the CAN second frames
	for(uint16_t count = 1; count < num_can_frames; count++){
		// if one of them fails to send, break for loop and return handle
		if(!isotp_send_second_frame(shims, count, num_can_frames, arbitration_id, data, size)){
			handle->success = false;
			break;
		}
	}
	
	return true;
}
