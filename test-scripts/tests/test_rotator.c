#include <check.h>
#include <string.h>
#include "plugins/plugin_common.h"

/* pull in the implementation so we reach the static helper */
#include "plugins/rotator.c"

START_TEST(rotator_single_char_noop)
{
    char s[] = "X";
    expand(s);
    ck_assert_str_eq(s, "X");
}
END_TEST

        Suite* rotator_suite(void)
{
    Suite *s  = suite_create("rotator");
    TCase *tc = tcase_create("core");
    tcase_add_test(tc, rotator_single_char_noop);
    suite_add_tcase(s, tc);
    return s;
}

int main(void)
{
    SRunner *sr = srunner_create(rotator_suite());
    srunner_run_all(sr, CK_ENV);
    int nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return nf ? 1 : 0;
}