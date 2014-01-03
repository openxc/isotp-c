#ifndef __ISOTP_H__
#define __ISOTP_H__

#include <isotp/isotp_types.h>
#include <isotp/send.h>
#include <isotp/receive.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

IsoTpShims isotp_init_shims(LogShim log,
        SendCanMessageShim send_can_message,
        SetTimerShim set_timer);

void isotp_message_to_string(const IsoTpMessage* message, char* destination,
        size_t destination_length);

/* Public: Change the timeout for waiting on an ISO-TP response frame.
 *
 * If this function is not used, the conventional 100ms is used by default.
 *
 * handler - the ISO-TP handler to modify.
 * timeout - the new timeout in milliseconds.
 */
// void isotp_set_timeout(IsoTpHandler* handler, uint16_t timeout_ms);


#ifdef __cplusplus
}
#endif

#endif // __ISOTP_H__
