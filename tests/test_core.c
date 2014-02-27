#include <isotp/receive.h>
#include <check.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

extern IsoTpShims SHIMS;

extern void message_sent(const IsoTpMessage* message, const bool success);

extern uint16_t last_can_frame_sent_arb_id;
extern uint8_t last_can_payload_sent[8];
extern uint8_t last_can_payload_size;
extern bool can_frame_was_sent;

extern bool message_was_received;
extern uint16_t last_message_received_arb_id;
extern uint8_t last_message_received_payload[];
extern uint8_t last_message_received_payload_size;

extern uint16_t last_message_sent_arb_id;
extern bool last_message_sent_status;
extern uint8_t last_message_sent_payload[];
extern uint8_t last_message_sent_payload_size;

extern void setup();

START_TEST (test_default_frame_padding_on)
{
    ck_assert(SHIMS.frame_padding);
    const uint8_t payload[] = {0x12, 0x34};
    uint32_t arbitration_id = 0x2a;
    isotp_send(&SHIMS, arbitration_id, payload, sizeof(payload), message_sent);
    ck_assert_int_eq(last_message_sent_arb_id, arbitration_id);
    fail_unless(last_message_sent_status);
    ck_assert_int_eq(last_message_sent_payload_size, 2);
    ck_assert_int_eq(last_can_payload_size, 8);

}
END_TEST

START_TEST (test_disabled_frame_padding)
{
    SHIMS.frame_padding = false;
    const uint8_t payload[] = {0x12, 0x34};
    uint32_t arbitration_id = 0x2a;
    isotp_send(&SHIMS, arbitration_id, payload, sizeof(payload), message_sent);
    ck_assert_int_eq(last_message_sent_arb_id, arbitration_id);
    fail_unless(last_message_sent_status);
    ck_assert_int_eq(last_message_sent_payload_size, 2);
    ck_assert_int_eq(last_can_payload_size, 3);

}
END_TEST

Suite* testSuite(void) {
    Suite* s = suite_create("iso15765");
    TCase *tc_core = tcase_create("core");
    tcase_add_checked_fixture(tc_core, setup, NULL);
    tcase_add_test(tc_core, test_default_frame_padding_on);
    tcase_add_test(tc_core, test_disabled_frame_padding);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void) {
    int numberFailed;
    Suite* s = testSuite();
    SRunner *sr = srunner_create(s);
    // Don't fork so we can actually use gdb
    srunner_set_fork_status(sr, CK_NOFORK);
    srunner_run_all(sr, CK_NORMAL);
    numberFailed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (numberFailed == 0) ? 0 : 1;
}
