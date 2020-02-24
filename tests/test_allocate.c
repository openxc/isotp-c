#include <isotp/allocate.h>
#include <check.h>

extern void setup();

START_TEST (test_memory_allocation)
{
    uint8_t* test_allocation = allocate(4);
    ck_assert(test_allocation != NULL);
    free_allocated(test_allocation);
}
END_TEST

Suite* testSuite(void) {
    Suite* s = suite_create("iso15765");
    TCase *tc_core = tcase_create("core");
    tcase_add_checked_fixture(tc_core, setup, NULL);
    tcase_add_test(tc_core, test_memory_allocation);
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