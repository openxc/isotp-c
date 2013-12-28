#include <isotp/isotp.h>
#include <check.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

extern IsoTpShims SHIMS;
extern IsoTpHandler ISOTP_HANDLER;

extern uint16_t last_can_frame_sent_arb_id;
extern uint8_t last_can_payload_sent[8];
extern uint8_t last_can_payload_size;
extern bool can_frame_was_sent;

extern bool message_was_received;
extern uint16_t last_message_received_arb_id;
extern uint8_t* last_message_received_payload;
extern uint8_t last_message_received_payload_size;

extern uint16_t last_message_sent_arb_id;
extern bool last_message_sent_status;
extern uint8_t* last_message_sent_payload;
extern uint8_t last_message_sent_payload_size;

extern void setup();

START_TEST (test_send_empty_payload)
{
    fail_unless(isotp_send(&ISOTP_HANDLER, NULL, 0));
    ck_assert_int_eq(last_message_sent_arb_id, ISOTP_HANDLER.arbitration_id);
    fail_unless(last_message_sent_status);
    ck_assert_int_eq(last_message_sent_payload[0], NULL);
    ck_assert_int_eq(last_message_sent_payload_size, 0);

    ck_assert_int_eq(last_can_frame_sent_arb_id, ISOTP_HANDLER.arbitration_id);
    fail_unless(can_frame_was_sent);
    ck_assert_int_eq(last_can_payload_sent[0], 0x0);
    ck_assert_int_eq(last_can_payload_size, 1);
}
END_TEST

START_TEST (test_send_single_frame)
{
    const uint8_t payload[] = {0x12, 0x34};
    fail_unless(isotp_send(&ISOTP_HANDLER, &payload, sizeof(payload)));
    ck_assert_int_eq(last_message_sent_arb_id, ISOTP_HANDLER.arbitration_id);
    fail_unless(last_message_sent_status);
    ck_assert_int_eq(last_message_sent_payload[0], 0x12);
    ck_assert_int_eq(last_message_sent_payload[1], 0x34);
    ck_assert_int_eq(last_message_sent_payload_size, 2);

    ck_assert_int_eq(last_can_frame_sent_arb_id, ISOTP_HANDLER.arbitration_id);
    fail_unless(can_frame_was_sent);
    ck_assert_int_eq(last_can_payload_sent[0], 0x2);
    ck_assert_int_eq(last_can_payload_sent[1], 0x12);
    ck_assert_int_eq(last_can_payload_sent[2], 0x34);
    ck_assert_int_eq(last_can_payload_size, 3);
}
END_TEST

START_TEST (test_send_multi_frame)
{
    const uint8_t payload[] = {0x12, 0x34, 0x56, 0x78, 0x90, 0x01, 0x23,
            0x45, 0x67, 0x89};
    bool status = isotp_send(&ISOTP_HANDLER, &payload, sizeof(payload));
    fail_if(status);
    fail_if(last_message_sent_status);
    fail_if(can_frame_was_sent);
}
END_TEST

Suite* testSuite(void) {
    Suite* s = suite_create("iso15765");
    TCase *tc_core = tcase_create("send");
    tcase_add_checked_fixture(tc_core, setup, NULL);
    tcase_add_test(tc_core, test_send_empty_payload);
    tcase_add_test(tc_core, test_send_single_frame);
    tcase_add_test(tc_core, test_send_multi_frame);
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
