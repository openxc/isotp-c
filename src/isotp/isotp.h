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

/* Public: Initialize an IsoTpShims with the given callback functions.
 *
 * If any callbacks are not to be used, set them to NULL. For documentation of
 * the function type signatures, see isotp_types.h. This struct is a handy
 * encapsulation used to pass the shims around to the various isotp_* functions.
 *
 * Returns a struct with the fields initailized to the callbacks.
 */
IsoTpShims isotp_init_shims(LogShim log,
        SendCanMessageShim send_can_message,
        SetTimerShim set_timer);

#ifdef __cplusplus
}
#endif

#endif /* __ISOTP_H__ */
