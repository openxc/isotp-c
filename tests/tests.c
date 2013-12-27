#include <check.h>
#include <stdint.h>
#include <bitfield/bitfield.h>

START_TEST (test_fail)
{
    fail_if(true);
}
END_TEST

Suite* bitfieldSuite(void) {
    Suite* s = suite_create("iso15765");
    TCase *tc_core = tcase_create("core");
    tcase_add_test(tc_core, test_fail);
    suite_add_tcase(s, tc_core);

    return s;
}

int main(void) {
    int numberFailed;
    Suite* s = bitfieldSuite();
    SRunner *sr = srunner_create(s);
    // Don't fork so we can actually use gdb
    srunner_set_fork_status(sr, CK_NOFORK);
    srunner_run_all(sr, CK_NORMAL);
    numberFailed = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (numberFailed == 0) ? 0 : 1;
}
