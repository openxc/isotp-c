#include <isotp/isotp.h>
#include <check.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

extern IsoTpShims SHIMS;
extern IsoTpHandler ISOTP_HANDLER;

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

START_TEST (test_send_single_frame)
{
    fail_if(true);
}
END_TEST

START_TEST (test_send_multi_frame)
{
    fail_if(true);
}
END_TEST

Suite* testSuite(void) {
    Suite* s = suite_create("iso15765");
    TCase *tc_core = tcase_create("send");
    tcase_add_checked_fixture(tc_core, setup, NULL);
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
