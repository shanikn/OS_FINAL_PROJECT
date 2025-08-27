#include <check.h>
#include <string.h>
#include "plugins/plugin_common.h"

#include "plugins/flipper.c"
/* pull in the implementation so we reach the static helper (expand) */

START_TEST(flipper_even_and_odd)
{
    char a[] = "ABCD";
    char b[] = "ABCDE";
    expand(a); expand(b);
    ck_assert_str_eq(a, "DCBA");
    ck_assert_str_eq(b, "EDCBA");
}
END_TEST

        Suite* flipper_suite(void)
{
    Suite *s  = suite_create("flipper");
    TCase *tc = tcase_create("core");
    tcase_add_test(tc, flipper_even_and_odd);
    suite_add_tcase(s, tc);
    return s;
}

int main(void)
{
    SRunner *sr = srunner_create(flipper_suite());
    srunner_run_all(sr, CK_ENV);
    int nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return nf ? 1 : 0;
}
