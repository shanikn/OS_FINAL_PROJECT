/* ------------------------------------------------------------------ *
 *  One line ≈ 1 MiB must not break any plug-in                       *
 * ------------------------------------------------------------------ */
#include <check.h>
#include <stdlib.h>
#include <string.h>
#include "plugins/sync/consumer_producer.h"

#define BIG (1 << 20)

START_TEST(one_meg_line_roundtrip)
        {
                consumer_producer_t q;
        ck_assert_ptr_eq(consumer_producer_init(&q, 8), NULL);

        /* build a 1 MiB line of ‘A’s + “\n” + NUL */
        char *s = malloc(BIG + 2);
        memset(s, 'A', BIG);
        s[BIG]   = '\n';
        s[BIG+1] = '\0';

        ck_assert_ptr_eq(consumer_producer_put(&q, s), NULL);
        char *out = consumer_producer_get(&q);
        ck_assert(out != NULL);
        ck_assert_int_eq(strlen(out), BIG + 1);   /* incl. ‘\n’        */
        ck_assert(out[BIG] == '\n');

        free(out);
        consumer_producer_destroy(&q);
        }
END_TEST

        Suite *long_line_suite(void)
{
    Suite *s  = suite_create("long_line");
    TCase *tc = tcase_create("core");
    tcase_add_test(tc, one_meg_line_roundtrip);
    suite_add_tcase(s, tc);
    return s;
}