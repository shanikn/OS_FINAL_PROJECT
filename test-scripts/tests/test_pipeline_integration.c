/* ------------------------------------------------------------------ *
 *  End-to-end: run analyzer in a child process                       *
 * ------------------------------------------------------------------ */
#include <check.h>
#include <stdio.h>
#include <string.h>

START_TEST(e2e_upper_logger)
        {
                FILE *out = fopen("analyzer.log", "r");
        if (!out) ck_abort_msg("missing analyzer.log");

        char  line[512];
        int   found = 0;
        while (fgets(line, sizeof line, out)) {
            /* strip ANSI escapes so that raw “HELLO” can be spotted      */
            char *p = line, clean[sizeof line]; size_t w = 0;
            while (*p && w+1 < sizeof clean) {
                if (*p == '\x1b') {          /* skip “\x1b[ … m” sequence */
                    while (*p && *p != 'm') ++p;
                    if (*p) ++p;              /* skip final 'm' */
                } else {
                    clean[w++] = *p++;
                }
            }
            clean[w] = '\0';
            if (strstr(clean, "HELLO")) { found = 1; break; }
        }
        fclose(out);

        ck_assert_msg(found, "expected transformed text in analyzer.log");
        }
END_TEST

        Suite *pipeline_suite(void)
{
    Suite *s = suite_create("pipeline");
    TCase *tc = tcase_create("core");
    tcase_add_test(tc, e2e_upper_logger);
    suite_add_tcase(s, tc);
    return s;
}

int main(void) {
    SRunner *sr = srunner_create(pipeline_suite());
    srunner_run_all(sr, CK_ENV);
    int nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return nf ? 1 : 0;
}
