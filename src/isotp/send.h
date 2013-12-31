#ifndef __ISOTP_SEND_H__
#define __ISOTP_SEND_H__

#include <isotp/isotp.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

IsoTpHandle isotp_send(IsoTpShims* shims, const uint16_t arbitration_id,
        const uint8_t* payload, uint16_t size,
        IsoTpMessageSentHandler callback);

#ifdef __cplusplus
}
#endif

#endif // __ISOTP_SEND_H__
