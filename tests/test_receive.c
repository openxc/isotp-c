#include <isotp/isotp.h>
#include <check.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

extern IsoTpShims SHIMS;
extern IsoTpReceiveHandle RECEIVE_HANDLE;

extern void message_sent(const IsoTpMessage* message, const bool success);

extern uint16_t last_can_frame_sent_arb_id;
extern uint8_t last_can_payload_sent;
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

START_TEST (test_receive_empty_can_message)
{
    const uint8_t data[CAN_MESSAGE_BYTE_SIZE] = {0};
    fail_if(RECEIVE_HANDLE.completed);
    IsoTpMessage message = isotp_continue_receive(&SHIMS, &RECEIVE_HANDLE, 0x100, data, 0);
    fail_if(message.completed);
    fail_if(message_was_received);
}
END_TEST

START_TEST (test_receive_wrong_id)
{
    const uint8_t data[CAN_MESSAGE_BYTE_SIZE] = {0};
    fail_if(RECEIVE_HANDLE.completed);
    IsoTpMessage message = isotp_continue_receive(&SHIMS, &RECEIVE_HANDLE, 0x100, data, 1);
    fail_if(message.completed);
    fail_if(message_was_received);
}
END_TEST

START_TEST (test_receive_bad_pci)
{
    // 4 is a reserved number for the PCI field - only 0-3 are allowed
    const uint8_t data[CAN_MESSAGE_BYTE_SIZE] = {0x40};
    IsoTpMessage message = isotp_continue_receive(&SHIMS, &RECEIVE_HANDLE, 0x2a, data, 1);
    fail_if(message.completed);
    fail_if(message_was_received);
}
END_TEST

START_TEST (test_receive_single_frame_empty_payload)
{
    const uint8_t data[CAN_MESSAGE_BYTE_SIZE] = {0x00, 0x12, 0x34};
    fail_if(RECEIVE_HANDLE.completed);
    IsoTpMessage message = isotp_continue_receive(&SHIMS, &RECEIVE_HANDLE, 0x2a, data, 3);
    fail_unless(RECEIVE_HANDLE.completed);
    fail_unless(message.completed);
    fail_unless(message_was_received);
    ck_assert_int_eq(last_message_received_arb_id, 0x2a);
    ck_assert_int_eq(last_message_received_payload_size, 0);
}
END_TEST

START_TEST (test_receive_single_frame)
{
    const uint8_t data[CAN_MESSAGE_BYTE_SIZE] = {0x02, 0x12, 0x34};
    IsoTpMessage message = isotp_continue_receive(&SHIMS, &RECEIVE_HANDLE, 0x2a, data, 3);
    fail_unless(message.completed);
    fail_unless(message_was_received);
    ck_assert_int_eq(last_message_received_arb_id, 0x2a);
    ck_assert_int_eq(last_message_received_payload_size, 2);
    ck_assert_int_eq(last_message_received_payload[0], 0x12);
    ck_assert_int_eq(last_message_received_payload[1], 0x34);
}
END_TEST

START_TEST (test_receive_multi_frame)
{
    const uint8_t data0[CAN_MESSAGE_BYTE_SIZE] = {0x10, 0x14, 0x49, 0x02, 0x01, 0x31, 0x46, 0x4d};
    IsoTpMessage message0 = isotp_continue_receive(&SHIMS, &RECEIVE_HANDLE, 0x2a, data0, 8);
    fail_unless(!RECEIVE_HANDLE.completed);
    fail_unless(!message0.completed);
    fail_unless(!message_was_received);
    fail_unless(message0.multi_frame);
    //make sure flow control message has been sent.
    ck_assert_int_eq(last_can_frame_sent_arb_id, 0x2a - 8);
    ck_assert_int_eq(last_can_payload_sent, 0x30);

    const uint8_t data1[CAN_MESSAGE_BYTE_SIZE] = {0x21, 0x43, 0x55, 0x39, 0x4a, 0x39, 0x34, 0x48};
    IsoTpMessage message1 = isotp_continue_receive(&SHIMS, &RECEIVE_HANDLE, 0x2a, data1, 8);
    fail_unless(!RECEIVE_HANDLE.completed);
    fail_unless(!message1.completed);
    fail_unless(!message_was_received);
    fail_unless(message1.multi_frame);

    const uint8_t data2[CAN_MESSAGE_BYTE_SIZE] = {0x22, 0x55, 0x41, 0x30, 0x34, 0x35, 0x32, 0x34};
    IsoTpMessage message2 = isotp_continue_receive(&SHIMS, &RECEIVE_HANDLE, 0x2a, data2, 8);
    fail_unless(RECEIVE_HANDLE.completed);
    fail_unless(message2.completed);
    fail_unless(message_was_received);
    fail_unless(message2.multi_frame);

    ck_assert_int_eq(last_message_received_arb_id, 0x2a);
    ck_assert_int_eq(last_message_received_payload_size, 0x14);
    ck_assert_int_eq(last_message_received_payload[0], 0x49);
    ck_assert_int_eq(last_message_received_payload[1], 0x02);
    ck_assert_int_eq(last_message_received_payload[2], 0x01);
    ck_assert_int_eq(last_message_received_payload[3], 0x31);
    ck_assert_int_eq(last_message_received_payload[4], 0x46);
    ck_assert_int_eq(last_message_received_payload[5], 0x4d);
    ck_assert_int_eq(last_message_received_payload[6], 0x43);
    ck_assert_int_eq(last_message_received_payload[7], 0x55);
    ck_assert_int_eq(last_message_received_payload[8], 0x39);
    ck_assert_int_eq(last_message_received_payload[9], 0x4a);
    ck_assert_int_eq(last_message_received_payload[10], 0x39);
    ck_assert_int_eq(last_message_received_payload[11], 0x34);
    ck_assert_int_eq(last_message_received_payload[12], 0x48);
    ck_assert_int_eq(last_message_received_payload[13], 0x55);
    ck_assert_int_eq(last_message_received_payload[14], 0x41);
    ck_assert_int_eq(last_message_received_payload[15], 0x30);
    ck_assert_int_eq(last_message_received_payload[16], 0x34);
    ck_assert_int_eq(last_message_received_payload[17], 0x35);
    ck_assert_int_eq(last_message_received_payload[18], 0x32);
    ck_assert_int_eq(last_message_received_payload[19], 0x34);
}
END_TEST

START_TEST (test_receive_large_multi_frame)
{
    const uint8_t data0[CAN_MESSAGE_BYTE_SIZE] = {0x10, 0x80, 0x49, 0x02, 0x01, 0x31, 0x46, 0x4d};
    IsoTpMessage message = isotp_continue_receive(&SHIMS, &RECEIVE_HANDLE, 0x2a, data0, 8);
    //Make sure we don't try to receive messages that are too large and don't send flow control.
    fail_unless(!can_frame_was_sent);
    fail_unless(!RECEIVE_HANDLE.completed);
    fail_unless(!message.completed);
    fail_unless(!message_was_received);
    fail_unless(!message.multi_frame);
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
    tcase_add_test(tc_core, test_receive_empty_can_message);
    tcase_add_test(tc_core, test_receive_multi_frame);
    tcase_add_test(tc_core, test_receive_large_multi_frame);
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
