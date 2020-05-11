#include <isotp/receive.h>
#include <isotp/send.h>
#include <isotp/allocate.h>
#include <bitfield/bitfield.h>
#include <string.h>
#include <stdlib.h>

#define ARBITRATION_ID_OFFSET 0x8

#define MULTIFRAME  1

static void isotp_complete_receive(IsoTpReceiveHandle* handle, IsoTpMessage* message) {
    if(handle->message_received_callback != NULL) {
        handle->message_received_callback(message);
    }
}

bool isotp_handle_single_frame(IsoTpReceiveHandle* handle, IsoTpMessage* message) {
    isotp_complete_receive(handle, message);
    return true;
}

bool isotp_handle_multi_frame(IsoTpReceiveHandle* handle, IsoTpMessage* message) {
    // call this once all consecutive frames have been received
    isotp_complete_receive(handle, message);
    return true;
}

bool isotp_send_flow_control_frame(IsoTpShims* shims, IsoTpMessage* message) {
    uint8_t can_data[CAN_MESSAGE_BYTE_SIZE] = {0};

    if(!set_nibble(PCI_NIBBLE_INDEX, PCI_FLOW_CONTROL_FRAME, can_data, sizeof(can_data))) {
        shims->log("Unable to set PCI in CAN data");
        return false;
    }

    shims->send_can_message(message->arbitration_id - ARBITRATION_ID_OFFSET, can_data,
            shims->frame_padding ? 8 : 1 + message->size);
    return true;
}


IsoTpReceiveHandle isotp_receive(IsoTpShims* shims,
        const uint32_t arbitration_id, IsoTpMessageReceivedHandler callback) {
    IsoTpReceiveHandle handle = {
        success: false,
        completed: false,
        arbitration_id: arbitration_id,
        message_received_callback: callback
    };

    return handle;
}

IsoTpMessage isotp_continue_receive(IsoTpShims* shims,
        IsoTpReceiveHandle* handle, const uint32_t arbitration_id,
        const uint8_t data[], const uint8_t size) {
    IsoTpMessage message = {
        arbitration_id: arbitration_id,
        completed: false,
        multi_frame: false,
        payload: {0},
        size: 0
    };
    int headersize = 3;

    if(size < 1) {
        return message;
    }

    if(handle->arbitration_id != arbitration_id) {
        if(shims->log != NULL)  {
            // You may turn this on for debugging, but in normal operation it's
            // very noisy if you are passing all received CAN messages to this
            // handler.
            /* shims->log("The arb ID 0x%x doesn't match the expected rx ID 0x%x", */
                    /* arbitration_id, handle->arbitration_id); */
        }
        return message;
    }

    IsoTpProtocolControlInformation pci = (IsoTpProtocolControlInformation)
            get_nibble(data, size, 0);

    // TODO this is set up to handle rx a response with a payload, but not to
    // handle flow control responses for multi frame messages that we're in the
    // process of sending

    switch(pci) {
        case PCI_SINGLE: {
            uint8_t payload_length = get_nibble(data, size, 1);
            
            if(payload_length > 0) {
                memcpy(message.payload, &data[1], payload_length);
            }
            
            message.size = payload_length;
            message.completed = true;
            handle->success = true;
            handle->completed = true;
            isotp_handle_single_frame(handle, &message);
            break;
        }
        //If multi-frame, then the payload length is contained in the 12
        //bits following the first nibble of Byte 0. 
        case PCI_FIRST_FRAME: {
            uint16_t payload_length = (get_nibble(data, size, 1) << 8) + get_byte(data, size, 1);

            if(payload_length > OUR_MAX_ISO_TP_MESSAGE_SIZE) {
                shims->log("Multi-frame response too large for receive buffer.");
                break;
            }

            //combined_payload - pre 2020 was large enough to hold the
            // full diagnostic response.  When multiframe is fully
            // implemented this will only need to be large enough to
            // hold a 3 byte header. (gja)

            //Need to allocate memory for the combination of multi-frame
            //messages. That way we don't have to allocate 4k of memory 
            //for each multi-frame response.
            uint8_t* combined_payload = NULL;
#if (MULTIFRAME==1)
            // Only need space for the header for stitched multiframe
            combined_payload = allocate(headersize);
#else
            combined_payload = allocate(payload_length);
#endif

            if(combined_payload == NULL) {
                shims->log("Unable to allocate memory for multi-frame response.");
                break;
            }

#if (MULTIFRAME==1)
            // Only need the header for stitched multiframe
            // Since we are no longer collecting the parts and passing the total response at the
            // end, we do not need to store it which is what combined_payload was for.  We just
            // need to keep the header for the data which is only 3 bytes
            // TODO: Fix tests/common.c - (OUR_MAX_ISO...) unit test
            memcpy(combined_payload, &data[2], headersize);
#else
            memcpy(combined_payload, &data[2], CAN_MESSAGE_BYTE_SIZE - 2);
#endif
            handle->receive_buffer = combined_payload;
            handle->received_buffer_size = CAN_MESSAGE_BYTE_SIZE - 2;
            handle->incoming_message_size = payload_length;

            message.multi_frame = true;
            handle->success = false;
            handle->completed = false;
            isotp_send_flow_control_frame(shims, &message);

            // 4/27/2020 Return Partial (First frame has important 3 bytes of
            // preamble code which contains mode and pid needed by UDS layer
            memcpy(message.payload, &data[2], CAN_MESSAGE_BYTE_SIZE - 2);
            message.size = CAN_MESSAGE_BYTE_SIZE - 2;
            break;
        }
        case PCI_CONSECUTIVE_FRAME: {
            uint8_t start_index = handle->received_buffer_size;
            uint8_t remaining_bytes = handle->incoming_message_size - start_index;
            message.multi_frame = true;

            if(remaining_bytes > 7) {   // If > 7 then there will be another frame
#if (MULTIFRAME!=1)
                memcpy(&handle->receive_buffer[start_index], &data[1], CAN_MESSAGE_BYTE_SIZE - 1);
#endif
                handle->received_buffer_size = start_index + 7;

                // 4/27/2020 - Return Partial Frame into message
                memcpy(message.payload, &data[1],CAN_MESSAGE_BYTE_SIZE - 1);
                message.size = CAN_MESSAGE_BYTE_SIZE - 1;

            } else {
#if (MULTIFRAME!=1)
                memcpy(&handle->receive_buffer[start_index], &data[1], remaining_bytes);
#endif
                handle->received_buffer_size = start_index + remaining_bytes;

                if(handle->received_buffer_size != handle->incoming_message_size){
                    free_allocated(handle->receive_buffer);
                    handle->success = false;
                    shims->log("Error capturing all bytes of multi-frame. Freeing memory.");
                } else {
#if (MULTIFRAME==1)
                    // Copy the 3 bytes of header and Only copy the last partial
                    // into message.payload
                    memcpy(message.payload,&handle->receive_buffer[0], headersize);
                    memcpy(&message.payload[headersize],&data[1], remaining_bytes);
                    free_allocated(handle->receive_buffer);
                    message.size = remaining_bytes + headersize;
#else
                    // Copy the full assembled meessage with it many parts into the output buffer "message"
                    memcpy(message.payload,&handle->receive_buffer[0],handle->incoming_message_size);
                    free_allocated(handle->receive_buffer);
                    message.size = handle->incoming_message_size;
#endif
                    message.completed = true;
                    shims->log("Successfully captured all of multi-frame. Freeing memory.");

                    handle->success = true;
                    handle->completed = true;
                    isotp_handle_multi_frame(handle, &message);
                }
            }
            break;
        }
        default:
            break;
    }
    return message;
}
