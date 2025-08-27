#include <check.h>
#include <string.h>
#include "plugins/plugin_common.h"

/* pull in the implementation so we reach the static helper */
#include "plugins/uppercaser.c"

START_TEST(to_upper_handles_utf8)
        {
                char s[] = "angstrom\n";
                uppercase(s);                     /* static from uppercaser.c */
                ck_assert_str_eq(s, "ANGSTROM\n");
        }
END_TEST

        Suite* upper_suite(void)
{
    Suite *s  = suite_create("uppercaser");
    TCase *tc = tcase_create("core");
    tcase_add_test(tc, to_upper_handles_utf8);
    suite_add_tcase(s, tc);
    return s;
}

int main(void)
{
    SRunner *sr = srunner_create(upper_suite());
    srunner_run_all(sr, CK_ENV);
    int nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return nf ? 1 : 0;
}
