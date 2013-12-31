#include <isotp/isotp.h>
#include <check.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

extern IsoTpShims SHIMS;
extern IsoTpHandle HANDLE;

extern uint16_t last_can_frame_sent_arb_id;
extern uint8_t last_can_payload_sent;
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

START_TEST (test_receive_wrong_id)
{
    const uint8_t data[CAN_MESSAGE_BYTE_SIZE] = {0};
    fail_if(isotp_receive_can_frame(&SHIMS, &HANDLE, 0x100, data, 1));
    fail_if(message_was_received);
}
END_TEST

START_TEST (test_receive_bad_pci)
{
    // 4 is a reserved number for the PCI field - only 0-3 are allowed
    const uint8_t data[CAN_MESSAGE_BYTE_SIZE] = {0x40};
    fail_if(isotp_receive_can_frame(&SHIMS, &HANDLE, 0x2a, data, 1));
    fail_if(message_was_received);
}
END_TEST

START_TEST (test_receive_single_frame_empty_payload)
{
    const uint8_t data[CAN_MESSAGE_BYTE_SIZE] = {0x00, 0x12, 0x34};
    fail_unless(isotp_receive_can_frame(&SHIMS, &HANDLE, 0x2a, data, 3));
    fail_unless(message_was_received);
    ck_assert_int_eq(last_message_received_arb_id, 0x2a);
    ck_assert_int_eq(last_message_received_payload_size, 0);
}
END_TEST

START_TEST (test_receive_single_frame)
{
    const uint8_t data[CAN_MESSAGE_BYTE_SIZE] = {0x02, 0x12, 0x34};
    fail_unless(isotp_receive_can_frame(&SHIMS, &HANDLE, 0x2a, data, 3));
    fail_unless(message_was_received);
    ck_assert_int_eq(last_message_received_arb_id, 0x2a);
    ck_assert_int_eq(last_message_received_payload_size, 2);
    ck_assert_int_eq(last_message_received_payload[0], 0x12);
    ck_assert_int_eq(last_message_received_payload[1], 0x34);
}
END_TEST

Suite* testSuite(void) {
    Suite* s = suite_create("iso15765");
    TCase *tc_core = tcase_create("receive");
    tcase_add_checked_fixture(tc_core, setup, NULL);
    tcase_add_test(tc_core, test_receive_wrong_id);
    tcase_add_test(tc_core, test_receive_bad_pci);
    tcase_add_test(tc_core, test_receive_single_frame);
    tcase_add_test(tc_core, test_receive_single_frame_empty_payload);
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
